# PakDownloaderTesting
A simple Unreal Engine 5.2 example setup to test the usage of the ChunkDownloader plugin.

This is based mainly on the guidelines found in this video: https://www.youtube.com/watch?v=h3A8qVb2VFk

The "Map" folder contains a small project with some content prepared to package into two pak files. The "DownloadablePakFiles.zip" file contains the built pak files and manifest, ready to be placed in the local host directory. You can check where to place these files in this section of the above video: https://www.youtube.com/watch?v=h3A8qVb2VFk&t=2263s

Packaging this project will produce two pak files, named pakchunk1000-Windows.pak	and pakchunk1001-Windows.pak. They both contain mostly the same stuff (with different color schemes for easy identification, orange and purple), but the first one contains files located in the project's content folder while the second one contains files located in a plugin's content folder.

The "Loader" folder contains the main project with the code implementing the ChunkDownloader plugin. There's also an "Executable.zip" file, which contains a pre-built version of the project.

Executing the built project will show something like the 3rd Person Example Project, with two colored spots ahead. Stepping on either of them will perform an "OpenLevel" command to a level loaded in one of the pak files mentioned previously.




TODO: code explanation
