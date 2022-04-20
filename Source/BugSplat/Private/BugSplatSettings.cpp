#include "BugSplatSettings.h"

BugSplatSettings::BugSplatSettings(FString configFilePath, FString uProjectFilePath)
{
	_configFilePath = configFilePath;
	_uProjectFilePath = uProjectFilePath;
	loadSettingsFromConfigFile();
}

void BugSplatSettings::setAppName(const FText& appName)
{
	_appName = appName.ToString();
}

void BugSplatSettings::setVersion(const FText& version)
{
	_version = version.ToString();
}

void BugSplatSettings::setDatabase(const FText& database)
{
	_database = database.ToString();
}

void BugSplatSettings::setUsername(const FText& username)
{
	_username = username.ToString();
}

void BugSplatSettings::setPassword(const FText& password)
{
	_password = password.ToString();
}

FString BugSplatSettings::buildEndpointUrl()
{
	FStringFormatOrderedArguments args;

	args.Add(_database);
	args.Add(_appName);
	args.Add(_version);

	return *FString::Format(*BUGSPLAT_ENDPOINT_URL_FORMAT, args);
}

FString BugSplatSettings::buildPostBuildStepsConsoleCommand()
{
	FStringFormatOrderedArguments args;

	args.Add(BUGSPLAT_SENDPDBS_DIR);
	args.Add(_username);
	args.Add(_password);
	args.Add(_appName);
	args.Add(_version);
	args.Add(_database);

	return *FString::Format(*POST_BUILD_STEPS_CONSOLE_COMMAND_FORMAT, args);
}

void BugSplatSettings::loadSettingsFromConfigFile()
{
	FString fileText;
	TSharedPtr<FJsonObject> fileJson = MakeShareable(new FJsonObject());
	FFileHelper::LoadFileToString(fileText, *_uProjectFilePath);
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(fileText);
	FJsonSerializer::Deserialize(Reader, fileJson);

	FString outString;

	if (!fileJson->TryGetStringField(DATABASE_TAG, outString))
	{
		// Is this setting it equal to the value or the ref?
		FString empty = FString("");
		_database = empty;
		_appName = empty;
		_version = empty;
		_username = empty;
		_password = empty;
		return;
	}

	_database = fileJson->GetStringField(DATABASE_TAG);
	_appName = fileJson->GetStringField(APP_NAME_TAG);
	_version = fileJson->GetStringField(VERSION_TAG);
	_username = fileJson->GetStringField(USERNAME_TAG);
	_password = fileJson->GetStringField(PASSWORD_TAG);
}

void BugSplatSettings::addPostBuildSteps(TSharedRef<FJsonObject> jsonObject)
{
	const TArray<TSharedPtr<FJsonValue>> * win64ConsoleCommandsOutput;
	const TSharedPtr<FJsonObject> * postBuildStepsOutput;

	TSharedPtr<FJsonObject> postBuildSteps = MakeShareable(new FJsonObject());
	TArray<TSharedPtr<FJsonValue>> win64ConsoleCommands = TArray<TSharedPtr<FJsonValue>>();

	TSharedPtr<FJsonValue> updatedPostBuildStepCommand = MakeShareable(new FJsonValueString(buildPostBuildStepsConsoleCommand()));
	if (jsonObject->TryGetObjectField(POST_BUILD_STEPS_LABEL, postBuildStepsOutput))
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
	jsonObject->SetObjectField(POST_BUILD_STEPS_LABEL, postBuildSteps);	
}

void BugSplatSettings::saveSettingsToConfigFile()
{
	FConfigCacheIni ini(EConfigCacheType::DiskBacked);

	// TODO this could potentially be set in the global config file for ALL projects
	ini.LoadFile(_configFilePath);

	FString sectionTag = FString(TEXT("CrashReportClient"));

	FString dataRouterUrlConfigTag = FString(TEXT("DataRouterUrl"));
	FString dataRouterUrlValue = buildEndpointUrl();

	FString crashReportClientVersionConfigTag = FString(TEXT("CrashReportClientVersion"));
	FString crashReportClientVersionValue = FString(TEXT("1.0"));

	ini.SetString(*sectionTag, *dataRouterUrlConfigTag, *dataRouterUrlValue, _configFilePath);
	ini.SetString(*sectionTag, *crashReportClientVersionConfigTag, *crashReportClientVersionValue, _configFilePath);
}

void BugSplatSettings::saveSettingsToUProject()
{
	FString fileText;

	TSharedPtr<FJsonObject> fileJson = MakeShareable(new FJsonObject());
	FFileHelper::LoadFileToString(fileText, *_uProjectFilePath);
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(fileText);
	FJsonSerializer::Deserialize(Reader, fileJson);

	FString Text;
	TSharedRef<FJsonObject> PluginJsonObject = fileJson.ToSharedRef();
	TSharedRef< TJsonWriter<> > WriterRef = TJsonWriterFactory<>::Create(&Text);
	TJsonWriter<>& Writer = WriterRef.Get();

	PluginJsonObject->SetStringField(DATABASE_TAG, _database);
	PluginJsonObject->SetStringField(APP_NAME_TAG, _appName);
	PluginJsonObject->SetStringField(VERSION_TAG, _version);
	PluginJsonObject->SetStringField(USERNAME_TAG, _username);	
	PluginJsonObject->SetStringField(PASSWORD_TAG, _password);

	addPostBuildSteps(PluginJsonObject);

	FJsonSerializer::Serialize(PluginJsonObject, Writer);
	FFileHelper::SaveStringToFile(Text, *_uProjectFilePath);
}

void BugSplatSettings::save()
{
	saveSettingsToUProject();
	saveSettingsToConfigFile();
}
