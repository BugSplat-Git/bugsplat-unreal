[![bugsplat-github-banner-basic-outline](https://user-images.githubusercontent.com/20464226/149019306-3186103c-5315-4dad-a499-4fd1df408475.png)](https://bugsplat.com)
<br/>
# <div align="center">BugSplat</div> 
### **<div align="center">Crash and error reporting built for busy developers.</div>**
<div align="center">
    <a href="https://twitter.com/BugSplatCo">
        <img alt="Follow @bugsplatco on Twitter" src="https://img.shields.io/twitter/follow/bugsplatco?label=Follow%20BugSplat&style=social">
    </a>
    <a href="https://discord.gg/K4KjjRV5ve">
        <img alt="Join BugSplat on Discord" src="https://img.shields.io/discord/664965194799251487?label=Join%20Discord&logo=Discord&style=social">
    </a>
</div>

## 👋 Introduction

BugSplat's Unreal plugin provides crash and exception reporting for Unreal projects. BugSplat provides you with invaluable insight into the issues tripping up your users.

In order to use this package please make sure you have completed the following checklist:
* [Sign Up](https://app.bugsplat.com/v2/sign-up) as a new BugSplat user
* [Log In](https://app.bugsplat.com/auth0/login) to your account

## 🏗 Installation 

You can choose to add BugSplat to your Unreal Project manually through git, or by downloading it through the Unreal Marketplace.

### Install to project 
1. Navigate to your project folder, which contains your [ProjectName].uproject file.
2. If it does not already exist, create a folder called Plugins.
3. Place bugsplat-unreal into the Plugins folder.
4. Ensure you can access the BugSplat plugin through Edit > BugSplat.

### Install from MarketPlace
(TODO Include link to marketplace and some helpful steps)

## ⚙️ Usage

BugSplat's Unreal integration modifies crash report configuration settings and automatically uploads symbol files during build time. To get started, access the BugSplat configuration menu through Edit > BugSplat.

Fill in your application and user settings. Note that a Client ID and Client Secret can be generated on the BugSplat [Integrations](https://app.bugsplat.com/v2/settings/database/integrations) page.

// Helpful Image Here //

Once you are happy with your configuration settings, you can choose where we will write our crash report configuration settings to depending on your build process.

If you would like BugSplat to be configured for every new packaged build, select 'Add to Global Ini'. Note that this will modify DefaultEngine.ini for the CrashReportClient and affects ALL other projects using the same engine build. However, this will only need to be repeated if your configuration settings ever change.

// Helpful Image Here //

If you would like to add BugSplat after a build has been packaged, select 'Add to Local Ini'. You will be prompted to provide the location of your packaged build. Note that you will need to repeat this step for every instance of your packaged game if you did not add your configuration settings to the Global Ini.

// Helpful Image Here //

If you would like BugSplat to automatically upload symbol files during package time, select 'Add Post Build Symbol Upload'. 

## 🧑‍💻 Contributing

BugSplat ❤️s open source! If you feel that this package can be improved, please open an [Issue](https://github.com/BugSplat-Git/bugsplat-unreal/issues). If you have an awesome new feature you'd like to implement, we'd love to merge your [Pull Request](https://github.com/BugSplat-Git/bugsplat-unreal/pulls). You can also send us an [email](mailto:support@bugsplat.com), join us on [Discord](https://discord.gg/K4KjjRV5ve), or message us via the in-app chat on [bugsplat.com](https://bugsplat.com).
