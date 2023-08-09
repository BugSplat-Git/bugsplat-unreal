import { compress } from "https://deno.land/x/zip@v1.2.5/mod.ts";

const unrealVersion = Deno.args[0];
const supportedVersions = ['5.0', '5.1', '5.2'];

if (!supportedVersions.includes(unrealVersion)) {
  console.error(`Unsupported version: ${unrealVersion}`);
  Deno.exit(1);
}

const pluginJson = await Deno.readTextFile('./BugSplat.uplugin');
const pluginVersion = JSON.parse(pluginJson).VersionName;
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