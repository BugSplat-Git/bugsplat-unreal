import { compress } from "https://deno.land/x/zip@v1.2.5/mod.ts";

const supportedVersions = ['5.0', '5.1', '5.2'];

const pluginJson = JSON.parse(await Deno.readTextFile('./BugSplat.uplugin'));

for (const unrealVersion of supportedVersions) {
  const pluginVersion = pluginJson.VersionName;
  pluginJson.EngineVersion = `${unrealVersion}.0`;
  await Deno.writeTextFile('./BugSplat.uplugin', JSON.stringify(pluginJson, null, 4));
  
  const zipName = `bugsplat-unreal-${pluginVersion}-unreal-${unrealVersion}.zip`;
  const zipEntries = [
    'Source',
    'Resources',
    'README.md',
    'Content',
    'Config',
    'BugSplat.uplugin',
  ];
  
  console.log(`About to create release package for Unreal Engine ${unrealVersion}...`);
  
  await compress(zipEntries, zipName);
  
  console.log(`Created ${zipName} successfully!`);
}