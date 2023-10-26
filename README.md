# PakDownloaderTesting
A simple Unreal Engine 5 example setup to test the usage of the ChunkDownloader plugin.

This is based mainly on the guidelines found in this video tutorial: [ChunkDownloader Explained | Inside Unreal](https://www.youtube.com/watch?v=h3A8qVb2VFk). Our main purpose with this example is to explore how to load packaged (`*.pak`) files that were specifically created in a different project to the one that we intend them to be loaded into. By default, the ChunkDownloader plugin expects pak files produced in the same project, and benefits from certain optimizations and data gathering done at build time. To be able to load external pak files we need to do these tasks manually, and there's little information online to describe this process.

## Map project
The [Map folder](Map) contains a small project with some content prepared to package into two pak files. The main focus of this example is being able to load the `*.umap` files within them. For this purpose, this project contains two examples: one is located in the project's main Content folder (called [`TestPakMap`](Map/Content/TestPak/TestPakMap.umap)), and another one is located in a content plugin within the same project (called [`TestPakPluginMap`](Map/Plugins/TestPakPluginMap/Content/TestPakPlugin/TestPakPluginMap.umap)). There are also modified material instance assets used by the geometry in those levels, to easily show if the dependencies were properly linked.

Packaging this project will produce an executable with a bunch of files, but only the pak files located in `/PakMap/Content/Paks/` are relevant, specifically `pakchunk1000-Windows.pak` and `pakchunk1001-Windows.pak`. The video tutorial above ([29:12](https://www.youtube.com/watch?v=h3A8qVb2VFk&t=1752s)) describes where to place these files and how to create the `BuildManifest-Windows.txt` file that is required by the plugin.

> [!WARNING]
> Make sure that case, spacing, tabs and line breaks in the BuildManifest file are **exactly** as described. The text parser used by the ChunkDownloader plugin is rather simple. Don't add extra lines, use single tabs between fields, and also add a single space before and after equal signs.

These files are meant to be downloaded by the main project, and by default the idea is to set them in a locally hosted website using Windows's Internet Information Services (IIS) Manager. There's a section on the above video tutorial ([37:43](https://www.youtube.com/watch?v=h3A8qVb2VFk&t=2263s)) that describes this.

 There is also a [zip file](DownloadablePakFiles.zip) that contains the pak and manifest files already built (with UE5.2.1), and placed in the appropriate folder hierarchy. To use them you just need to extract the zip in the locally hosted website directory (usually `C:\inetpub\wwwroot\`).



## Loader project

The [Loader folder](Loader) contains the main project with the code that implements the ChunkDownloader plugin. Executing the built project will show a modified 3rd Person example project map with two colored spots ahead. Stepping on either of them will perform an "OpenLevel" command to the levels mentioned previously.

All the logic of this example is contained in the main level's LevelBlueprint, which calls functions from the [subsystem](Loader/Source/PakLoader/ChunkDownloaderSubsystem.h) that implements the ChunkDownloader plugin. This is a very bare bones implementation intended to focus on the two specific requirements that we need to complement the plugin with in order to load external pak files, and we are forgoing the use of many functions that would be required in a more robust implementation.

There's also a [zip file](Executable.zip) that contains a pre-built version of the project (with UE5.2.1). To test it you just need to extract the zip and execute.

### Explanation

When packaging a project, a global registry of all the assets and directories expected to exist will be stored in a file called `AssetRegistry.bin`. This will include even content that is intended to be packed into pak files, and when the game is executed, it will read this file at startup and generate mounting points in the virtual file system used by the engine.

This means that when the ChunkDownloader plugin mounts a pak file created from the same project, its contents will be linked to their expected folder structure and validated by the asset registry automatically, without needing further handling. However, if we try to load an externally created pak file, the main project would have no prior knowledge of its contents and folder structure. No mounting point will exist in the virtual file system, nor will the asset registry know about them. We need to expand the plugin's functionality to do this.

Unfortunately, the ChunkDownloader plugin hasn't been coded in a way that is easy to extend. Pretty much all of the functions and properties needed are either private or defined only in the cpp file, and the module that instantiates it doesn't expose a way to swap its expected class. basically we have three options available:
1. Modifying the engine code. If we are using a compiled version of the engine's source code, we can simply modify the original class to add the stuff we need, but this usually goes far beyond the scope of most projects.
2. Duplicating the whole plugin, and then add our required modifications, to use in place of the original ChunkDownloader. This is probably the most advisable option, but must be handled with care because many "plugins" included in the engine end having unexpected dependencies.
3. Work with the original plugin and try to complement it at some performance cost, which is the case of this example. You can check the comments in the  [cpp file](Loader/Source/PakLoader/ChunkDownloaderSubsystem.cpp) for a more thorough explanation and also suggestions.


