import { compress } from "https://raw.githubusercontent.com/BugSplat-Git/deno-zip/d6591c94506fde867edb06578549ded598f345dc/mod.ts";

const supportedVersions = ['5.0', '5.1', '5.2', '5.3', '5.4', '5.5', '5.6'];

const pluginJson = JSON.parse(await Deno.readTextFile('./BugSplat.uplugin'));

// Remove downloaded symbol-upload-* files before packaging
const symUploaderDir = './Source/ThirdParty/SymUploader';
for await (const entry of Deno.readDir(symUploaderDir)) {
  if (entry.isFile && entry.name.startsWith('symbol-upload-')) {
    await Deno.remove(`${symUploaderDir}/${entry.name}`);
    console.log(`Deleted ${entry.name}`);
  }
}

// Update the plugin version for each supported Unreal Engine version
for (const unrealVersion of supportedVersions) {
  const pluginVersion = pluginJson.VersionName;
  pluginJson.EngineVersion = `${unrealVersion}.0`;
  await Deno.writeTextFile('./BugSplat.uplugin', JSON.stringify(pluginJson, null, 4));
  
  const zipName = `bugsplat-unreal-${pluginVersion}-unreal-${unrealVersion}.zip`;
  const zipEntries = [
    'Source',
    'Resources',
    'README.md',
    'LICENSE.md',
    'Content',
    'Config',
    'BugSplat.uplugin',
  ];
  
  console.log(`About to create release package for Unreal Engine ${unrealVersion}...`);
  
  await compress(zipEntries, zipName);
  
  console.log(`Created ${zipName} successfully!`);
}