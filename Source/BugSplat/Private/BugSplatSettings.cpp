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

	if (!fileJson->TryGetStringField("Database", outString))
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
	TSharedPtr<FJsonObject> postBuildStepsJson = MakeShareable(new FJsonObject());
	TArray<TSharedPtr<FJsonValue>> win64PostBuildStepsJson;
	TSharedPtr<FJsonValueString> win64Value = MakeShareable(new FJsonValueString(buildPostBuildStepsConsoleCommand()));
	win64PostBuildStepsJson.Add(win64Value);
	postBuildStepsJson->SetArrayField("Win64", win64PostBuildStepsJson);
	jsonObject->SetObjectField("PostBuildSteps", postBuildStepsJson);
	
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
