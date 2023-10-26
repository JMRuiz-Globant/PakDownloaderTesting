// Fill out your copyright notice in the Description page of Project Settings.

#include "ChunkDownloaderSubsystem.h"
#include "HttpModule.h"
#include "IPlatformFilePak.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Logging/LogMacros.h"

#define LOCTEXT_NAMESPACE "LogChunkDownloaderSubsystem"

DEFINE_LOG_CATEGORY_STATIC(LogChunkDownloaderSubsystem, Log, All);

void UChunkDownloaderSubsystem::Initialize(FSubsystemCollectionBase &Collection) {
	Super::Initialize(Collection);

	// Default initialization for a Windows machine with 8 cores. 
	FChunkDownloader::GetOrCreate()->Initialize("Windows", 8);
}

void UChunkDownloaderSubsystem::Deinitialize() {
	Super::Deinitialize();
	FChunkDownloader::Shutdown();
}

void UChunkDownloaderSubsystem::GetLoadingProgress(int32& DLed, int32& ToDL, float& DLPercent, int32& Mounted, int32& ToMount, float& MountPercent) const {
	//Get a reference to ChunkDownloader
	TSharedRef<FChunkDownloader> Downloader = FChunkDownloader::GetChecked();

	//Get the loading stats struct
	FChunkDownloader::FStats LoadingStats = Downloader->GetLoadingStats();

	//Get the bytes downloaded and bytes to download
	DLed = LoadingStats.BytesDownloaded;
	ToDL = LoadingStats.TotalBytesToDownload;

	//Get the number of chunks mounted and chunks to download
	Mounted = LoadingStats.ChunksMounted;
	ToMount = LoadingStats.TotalChunksToMount;

	//Calculate the download and mount percent using the above stats
	DLPercent = (float)DLed / (float)ToDL;
	MountPercent = (float)Mounted / (float)ToMount;
}

void UChunkDownloaderSubsystem::UpdateBuild(const FString& DeploymentName, const FString& ContentBuildId, FChunkDownloaderSubsystemCallback Callback)
{
	auto ChunkDownloader = FChunkDownloader::GetChecked();
	
	// Try to load cached build. This populates some of the ChunkDownloader internal properties and can avoid downloading files that were cached on a prior execution and are still valid.
	ChunkDownloader->LoadCachedBuild(DeploymentName);

	// Update the build manifest file.
	bIsDownloadManifestUpToDate = false;
	ChunkDownloader->UpdateBuild(DeploymentName, ContentBuildId, [this, Callback](bool bSuccess) {
		bIsDownloadManifestUpToDate = bSuccess;
		Callback.ExecuteIfBound(bSuccess);
		});
}

void UChunkDownloaderSubsystem::PatchGame(const TArray<int32>& ChunkIds, FChunkDownloaderSubsystemCallback Callback)
{
	// Only attempt if Update Build was called first.
	if (!bIsDownloadManifestUpToDate) {
		UE_LOG(LogChunkDownloaderSubsystem, Display, TEXT("Cannot patch the game before updating the manifest."));
		Callback.ExecuteIfBound(false);
		return;
	}

	// Also only do if we got at least one ChunkId.
	if (ChunkIds.Num() <= 0) {
		UE_LOG(LogChunkDownloaderSubsystem, Display, TEXT("No ChunkIds to patch."));
		Callback.ExecuteIfBound(false);
		return;
	}

	auto ChunkDownloader = FChunkDownloader::GetChecked();

	// Try to download and mount the chosen chunks.
	ChunkDownloader->MountChunks(ChunkIds, [Callback](bool bSuccess) {
		Callback.ExecuteIfBound(bSuccess);
		});
}




void UChunkDownloaderSubsystem::RegisterMounts(FChunkDownloaderSubsystemCallback Callback)
{
	// Only attempt if Update Build was called first.
	if (!bIsDownloadManifestUpToDate) {
		UE_LOG(LogChunkDownloaderSubsystem, Display, TEXT("Cannot patch the game before updating the manifest."));
		Callback.ExecuteIfBound(false);
		return;
	}

	/*
	This function is necessary due to the fact that the FChunkDownloader class is too opaque for our requirements.
	Ideally, we would have access to the FChunkDownloader::PakFiles property, which contains literally ALL the data we need, it maps each pak file to its chunk ID, keeps track of its load status and even URL.
	But since we have no means of accessing it, we need to somehow rebuild this information.

	We will do this first by getting the list of mounted pak files from the FPakPlatformFile manager, which will list all of them, even those that aren't handled by us.
	Then we will get the ChunkDownloader's manifest data using a duplicate of the function it uses internally for this purpose, and compare both lists to get which of our pak files were mounted.
	*/

	FPakPlatformFile* PakFileManager = (FPakPlatformFile*)(FPlatformFileManager::Get().FindPlatformFile(TEXT("PakFile")));

	// Get the filenames of all pak files that are mounted.
	TArray<FString> MountedPaks;
	PakFileManager->GetMountedPakFilenames(MountedPaks);


	// Parse the cached build manifest. With this we can identify the IDs of all pak files.
	const FString CacheFolder = FPaths::ProjectPersistentDownloadDir() / TEXT("PakCache/");
	const TArray<FPakFileEntry> CachedManifest = UChunkDownloaderSubsystem::ParseManifest(CacheFolder / TEXT("CachedBuildManifest.txt"));

	// Keep all the root directories in an array that we can use later.
	TArray<FString> PakRootDirs;

	// Iterate the parsed manifest data, and check for mounted paks.
	for (const FPakFileEntry& CachedFileEntry : CachedManifest) {
		const FString PakFileName = CachedFileEntry.FileName;
		const FString PakFilePath = CacheFolder / PakFileName;
		if (MountedPaks.Contains(PakFilePath)) {
			TRefCountPtr<FPakFile> PakFile = new FPakFile(PakFileManager, *PakFilePath, false);
			FPakFile* Pak = PakFile.GetReference();
			if (Pak && Pak->IsValid()) {


				/*
				At this point we know this is one of our pak files that's already mounted.
				Now, we need to figure out where to mount it, which implies some complexity if the file was generated outside the main project.

				If we could extend the FChunkDownloader class, the best place to do this would be directly in the FChunkDownloader::MountChunk() function or somewhere close.
				This would save us from unnecessary checkups.
				*/

				const FString PakMountPoint = Pak->GetMountPoint();
				const FString PakRootDir = UChunkDownloaderSubsystem::ParseRootDir(PakMountPoint);
				if (!PakRootDir.IsEmpty()) {
					FPackageName::RegisterMountPoint(PakRootDir, PakMountPoint);
					PakRootDirs.AddUnique(PakRootDir);
				}
				else {
					UE_LOG(LogChunkDownloaderSubsystem, Display, TEXT("No valid root dir could be chosen for pak file %s with mount point %s."), *PakFileName, *PakMountPoint);
				}
			}
		}

	}
				
	/*
	Once all the mount points have been created, we still need the Asset Registry to scan the directories and validate assets. We do this afterwards to avoid repeated calls, which can be performance-intensive if there are many assets.
	*/

	// Add the mounted directories to the Asset Registry. This is necessary to be able to load maps and secondary asset dependencies.
	if (PakRootDirs.Num() > 0) {
		IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
		AssetRegistry.ScanPathsSynchronous(PakRootDirs, true);
	}

	Callback.ExecuteIfBound(PakRootDirs.Num() > 0);
}

TArray<FPakFileEntry> UChunkDownloaderSubsystem::ParseManifest(const FString& ManifestPath, TMap<FString, FString>* Properties) {
	int32 ExpectedEntries = -1;
	TArray<FPakFileEntry> Entries;
	IFileHandle* ManifestFile = IPlatformFile::GetPlatformPhysical().OpenRead(*ManifestPath);
	if (ManifestFile != nullptr) {
		int64 FileSize = ManifestFile->Size();
		if (FileSize > 0) {
			UE_LOG(LogChunkDownloaderSubsystem, Display, TEXT("Found manifest at %s"), *ManifestPath);

			// read the whole file into a buffer (expecting UTF-8 so null terminate)
			char* FileBuffer = new char[FileSize + 8]; // little extra since we're forcing null term in places outside bounds of a field
			if (ManifestFile->Read((uint8*)FileBuffer, FileSize)) {
				FileBuffer[FileSize] = '\0';

				// make buffers for stuff read from each line
				char NameBuffer[512] = { 0 };
				uint64 FinalFileLen = 0;
				char VersionBuffer[512] = { 0 };
				int32 ChunkId = -1;
				char RelativeUrl[2048] = { 0 };

				// line end
				int LineNum = 0;
				int NextLineStart = 0;
				while (NextLineStart < FileSize) {
					int LineStart = NextLineStart;
					++LineNum;

					// find the end of the line
					int LineEnd = LineStart;
					while (LineEnd < FileSize && FileBuffer[LineEnd] != '\n' && FileBuffer[LineEnd] != '\r') {
						++LineEnd;
					}

					// find the end of the line
					NextLineStart = LineEnd + 1;
					while (NextLineStart < FileSize && (FileBuffer[NextLineStart] == '\n' || FileBuffer[NextLineStart] == '\r')) {
						++NextLineStart;
					}

					// see if this is a property
					if (FileBuffer[LineStart] == '$') {
						// parse the line
						char* NameStart = &FileBuffer[LineStart + 1];
						char* NameEnd = FCStringAnsi::Strstr(NameStart, " = ");
						if (NameEnd != nullptr) {
							char* ValueStart = NameEnd + 3;
							char* ValueEnd = &FileBuffer[LineEnd];
							*NameEnd = '\0';
							*ValueEnd = '\0';

							FString Name = FUTF8ToTCHAR(NameStart, NameEnd - NameStart + 1).Get();
							FString Value = FUTF8ToTCHAR(ValueStart, ValueEnd - ValueStart + 1).Get();
							if (Properties != nullptr) {
								Properties->Add(Name, Value);
							}

							if (Name == TEXT("NUM_ENTRIES")) {
								ExpectedEntries = FCString::Atoi(*Value);
							}
						}
						continue;
					}

					// parse the line
#if PLATFORM_WINDOWS || PLATFORM_MICROSOFT
					if (!ensure(sscanf_s(&FileBuffer[LineStart], "%511[^\t]\t%llu\t%511[^\t]\t%d\t%2047[^\r\n]",
						NameBuffer, (int)sizeof(NameBuffer),
						&FinalFileLen,
						VersionBuffer, (int)sizeof(VersionBuffer),
						&ChunkId,
						RelativeUrl, (int)sizeof(RelativeUrl)
					) == 5))
#else
					if (!ensure(sscanf(&FileBuffer[LineStart], "%511[^\t]\t%llu\t%511[^\t]\t%d\t%2047[^\r\n]",
						NameBuffer,
						&FinalFileLen,
						VersionBuffer,
						&ChunkId,
						RelativeUrl
					) == 5))
#endif
					{
						UE_LOG(LogChunkDownloaderSubsystem, Display, TEXT("Manifest parse error at %s:%d"), *ManifestPath, LineNum);
						continue;
					}

					// add a new pak file entry
					FPakFileEntry Entry;
					Entry.FileName = UTF8_TO_TCHAR(NameBuffer);
					Entry.FileSize = FinalFileLen;
					Entry.FileVersion = UTF8_TO_TCHAR(VersionBuffer);
					if (ChunkId >= 0) {
						Entry.ChunkId = ChunkId;
						Entry.RelativeUrl = UTF8_TO_TCHAR(RelativeUrl);
					}
					Entries.Add(Entry);
				}

				// all done
				delete[] FileBuffer;
			}
			else {
				UE_LOG(LogChunkDownloaderSubsystem, Display, TEXT("Read error loading manifest at %s"), *ManifestPath);
			}
		}
		else {
			UE_LOG(LogChunkDownloaderSubsystem, Display, TEXT("Empty manifest found at %s"), *ManifestPath);
		}

		// close the file
		delete ManifestFile;
	}
	else {
		UE_LOG(LogChunkDownloaderSubsystem, Display, TEXT("No manifest found at %s"), *ManifestPath);
	}

	if (ExpectedEntries >= 0 && ExpectedEntries != Entries.Num()) {
		UE_LOG(LogChunkDownloaderSubsystem, Display, TEXT("Corrupt manifest at %s (expected %d entries, got %d)"), *ManifestPath, ExpectedEntries, Entries.Num());
		Entries.Empty();
		if (Properties != nullptr) {
			Properties->Empty();
		}
	}

	return Entries;
}

FString UChunkDownloaderSubsystem::ParseRootDir(const FString& MountPoint)
{
	// The default mount point set within the pak file is usually the outermost directory of all the files contained.
	// To avoid breaking secondary asset links, we will mount the directories based off either the "/<ProjectName>/Content" folder or the "/<ProjectName>/Plugins/<PluginName>/Content" folder.

	FString LeftStr, RightStr;

	if (MountPoint.Split(TEXT("/Plugins/"), &LeftStr, &RightStr))
	{
		// A plugin directory should be something like "../../../<ProjectName>/Plugins/<PluginName>/Content/<etc>".
		// LeftStr  should contain now "../../../<ProjectName>".
		// RightStr should contain now "<PluginName>/Content/<etc>".

		if (RightStr.Split(TEXT("/Content/"), &LeftStr, &RightStr)) {

			// LeftStr  should contain now "<PluginName>".
			// RightStr should contain now "<etc>".

			// Root should be either "/<PluginName>/" or "/<PluginName>/<etc>/".
			return FPaths::RemoveDuplicateSlashes(FString::Printf(TEXT("/%s/%s/"), *LeftStr, *RightStr));
		}
	}
	else if (MountPoint.Split(TEXT("/Content/"), &LeftStr, &RightStr))
	{
		// A content directory should be something like "../../../<ProjectName>/Content/<etc>"
		// LeftStr  should contain now "../../../<ProjectName>".
		// RightStr should contain now "<etc>".

		// Root should be either "/Game/" or "/Game/<etc>/"
		return  FPaths::RemoveDuplicateSlashes(FString::Printf(TEXT("/Game/%s/"), *RightStr));
	}

	// It is possible to create a pak file with a mount point that isn't relative to a plugin or game content folders.
	// For example, if a pak file contains elements from plugins AND game content folders, the mount point will probably be something like "../../../<ProjectName>".
	// Handling such pak files is out of the scope of this example.

	return FString();
}

#undef LOCTEXT_NAMESPACE
