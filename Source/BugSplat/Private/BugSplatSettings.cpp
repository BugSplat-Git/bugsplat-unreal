#include "BugSplatSettings.h"
#include <Developer/DesktopPlatform/Public/DesktopPlatformModule.h>

FBugSplatSettings::FBugSplatSettings(FString UProjectFilePath)
{
	FBugSplatSettings::UProjectFilePath = UProjectFilePath;
	LoadSettingsFromConfigFile();
}

void FBugSplatSettings::SetAppName(const FText& NewAppName)
{
	AppName = NewAppName.ToString();
}

void FBugSplatSettings::SetVersion(const FText& NewVersion)
{
	Version = NewVersion.ToString();
}

void FBugSplatSettings::SetDatabase(const FText& NewDatabase)
{
	Database = NewDatabase.ToString();
}

void FBugSplatSettings::SetClientID(const FText& NewClientID)
{
	ClientID = NewClientID.ToString();
}

void FBugSplatSettings::SetClientSecret(const FText& NewPassword)
{
	ClientSecret = NewPassword.ToString();
}

FString FBugSplatSettings::CreateBugSplatEndpointUrl()
{
	FStringFormatOrderedArguments args;

	args.Add(Database);
	args.Add(AppName);
	args.Add(Version);

	return *FString::Format(*BUGSPLAT_ENDPOINT_URL_FORMAT, args);
}

FString FBugSplatSettings::CreatePostBuildStepsConsoleCommand()
{
	FString PostBuildStepsConsoleCommandFormat =
		FString(
			"call \"$(ProjectDir){0}\" " //Send PDBS Endpoint
			"/u {1} " // Username
			"/p {2} " // Password
			"/a {3} " // AppName
			"/v {4} " // Version
			"/b {5} " // Database
			"/d \"$(ProjectDir)\\Binaries\\$(TargetPlatform)\"" // Project Directory
		);

	FStringFormatOrderedArguments args;

	args.Add(BUGSPLAT_SENDPDBS_DIR);
	args.Add(ClientID);
	args.Add(ClientSecret);
	args.Add(AppName);
	args.Add(Version);
	args.Add(Database);

	return *FString::Format(*PostBuildStepsConsoleCommandFormat, args);
}

void FBugSplatSettings::LoadSettingsFromConfigFile()
{
	FString fileText;
	TSharedPtr<FJsonObject> fileJson = MakeShareable(new FJsonObject());
	FFileHelper::LoadFileToString(fileText, *UProjectFilePath);
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(fileText);
	FJsonSerializer::Deserialize(Reader, fileJson);

	FString outString;

	if (!fileJson->TryGetStringField(DATABASE_TAG, outString))
	{
		FString empty = FString("");
		Database = empty;
		AppName = empty;
		Version = empty;
		ClientID = empty;
		ClientSecret = empty;
		return;
	}

	Database = fileJson->GetStringField(DATABASE_TAG);
	AppName = fileJson->GetStringField(APP_NAME_TAG);
	Version = fileJson->GetStringField(VERSION_TAG);
	ClientID = fileJson->GetStringField(CLIENT_ID_TAG);
	ClientSecret = fileJson->GetStringField(CLIENT_SECRET_TAG);
}

void FBugSplatSettings::AddPostBuildSteps(TSharedRef<FJsonObject> JsonObject)
{
	const TArray<TSharedPtr<FJsonValue>> * win64ConsoleCommandsOutput;
	const TSharedPtr<FJsonObject> * postBuildStepsOutput;

	TSharedPtr<FJsonObject> postBuildSteps = MakeShareable(new FJsonObject());
	TArray<TSharedPtr<FJsonValue>> win64ConsoleCommands = TArray<TSharedPtr<FJsonValue>>();

	TSharedPtr<FJsonValue> updatedPostBuildStepCommand = MakeShareable(new FJsonValueString(CreatePostBuildStepsConsoleCommand()));
	if (JsonObject->TryGetObjectField(POST_BUILD_STEPS_LABEL, postBuildStepsOutput))
	{
		if (postBuildStepsOutput->Get()->TryGetArrayField(WIN_64_LABEL, win64ConsoleCommandsOutput))
		{
			int bugSplatConsoleCommandIndex = -1;
			for (int i = 0; i < win64ConsoleCommandsOutput->Num(); i++)
			{
				TSharedPtr<FJsonValue> value = (*win64ConsoleCommandsOutput)[i];
				FString stringValue = (*win64ConsoleCommandsOutput)[i]->AsString();
				win64ConsoleCommands.Add(value);
				if (stringValue.Contains(BUGSPLAT_SENDPDBS_DIR))
				{
					bugSplatConsoleCommandIndex = i;
				}
			}

			if (bugSplatConsoleCommandIndex < 0)
			{
				win64ConsoleCommands.Add(updatedPostBuildStepCommand);
			}
			else
			{
				win64ConsoleCommands[bugSplatConsoleCommandIndex] = updatedPostBuildStepCommand;
			}
		}
	}
	else
	{
		win64ConsoleCommands.Add(updatedPostBuildStepCommand);
	}

	postBuildSteps->SetArrayField(WIN_64_LABEL, win64ConsoleCommands);
	JsonObject->SetObjectField(POST_BUILD_STEPS_LABEL, postBuildSteps);	
}

void FBugSplatSettings::UpdateCrashReportClientIni(FString IniFilePath)
{
	FConfigCacheIni ini(EConfigCacheType::DiskBacked);

	ini.LoadFile(IniFilePath);

	FString sectionTag = FString(TEXT("CrashReportClient"));

	FString dataRouterUrlConfigTag = FString(TEXT("DataRouterUrl"));
	FString dataRouterUrlValue = CreateBugSplatEndpointUrl();

	FString crashReportClientVersionConfigTag = FString(TEXT("CrashReportClientVersion"));
	FString crashReportClientVersionValue = FString(TEXT("1.0"));

	ini.SetString(*sectionTag, *dataRouterUrlConfigTag, *dataRouterUrlValue, IniFilePath);
	ini.SetString(*sectionTag, *crashReportClientVersionConfigTag, *crashReportClientVersionValue, IniFilePath);
}

void FBugSplatSettings::SaveSettingsToUProject()
{
	FString fileText;

	TSharedPtr<FJsonObject> fileJson = MakeShareable(new FJsonObject());
	FFileHelper::LoadFileToString(fileText, *UProjectFilePath);
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(fileText);
	FJsonSerializer::Deserialize(Reader, fileJson);

	FString Text;
	TSharedRef<FJsonObject> PluginJsonObject = fileJson.ToSharedRef();
	TSharedRef< TJsonWriter<> > WriterRef = TJsonWriterFactory<>::Create(&Text);
	TJsonWriter<>& Writer = WriterRef.Get();

	PluginJsonObject->SetStringField(DATABASE_TAG, Database);
	PluginJsonObject->SetStringField(APP_NAME_TAG, AppName);
	PluginJsonObject->SetStringField(VERSION_TAG, Version);
	PluginJsonObject->SetStringField(CLIENT_ID_TAG, ClientID);	
	PluginJsonObject->SetStringField(CLIENT_SECRET_TAG, ClientSecret);

	AddPostBuildSteps(PluginJsonObject);

	FJsonSerializer::Serialize(PluginJsonObject, Writer);
	FFileHelper::SaveStringToFile(Text, *UProjectFilePath);
}

void FBugSplatSettings::Save()
{
	SaveSettingsToUProject();
}

void FBugSplatSettings::PackageWithBugSplat()
{
	FString OutFolderName;


	if (!FDesktopPlatformModule::Get()->OpenDirectoryDialog(
		FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr),
		FString("Packaged Directory"),
		FPaths::GetProjectFilePath(),
		OutFolderName))
	{
		return;
	}

	FString ConfigFilePathFormat = *FPaths::Combine( OutFolderName , *PACKAGED_BUILD_CONFIG_PATH);
	
	UpdateCrashReportClientIni(ConfigFilePathFormat);
}	
