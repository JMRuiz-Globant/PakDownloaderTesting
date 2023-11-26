// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ChunkDownloaderCommon.h"

template<typename TTask> class FAsyncTask;
class IHttpRequest;
class FDownload;

DECLARE_MULTICAST_DELEGATE_TwoParams(FPlatformChunkInstallMultiDelegate, uint32, bool);

/**
 * This is a duplicate of Epic's ChunkDownloader plugin, done with the purpose of expanding its functionality to handle pak files that aren't part of the original project's build.
 * 
 * The original plugin's aim was to provide a system to handle the packaging and distribution of a project's files, and works under several expectations that are only valid for pak files
 * that were generated in the whole project's packaging process. For example, it assumes all mount points used by the pak files are tracked and registered automatically by the project.
 * It also assumes that their contents are tracked and handled by the Asset Registry automatically.
 * 
 * These assumptions aren't true for external pak files, and while the missing code to handle them isn't particularily lengthy or complex, the original plugin keeps most of its contents private.
 * This makes it necessary to work around it and perform redundant tasks to allow for this additional functionality. Extending the class isn't possible either,
 * since the plugin includes multiple classes that make explicit use of this class and give no room for inheritance or injection.
 * 
 * Fortunately, the plugin itself is very self-contained so it's possible to exchange it for this duplicate without breaking anything.
 * It should even be possible to have both plugins active at the same time, although this is unnecessary since this one performs everytghing the other does.
 */
class CHUNKDOWNLOADERCUSTOM_API FChunkDownloader : public TSharedFromThis<FChunkDownloader>
{
public:
	~FChunkDownloader();
	typedef TFunction<void(bool bSuccess)> FCallback;

	// static getters
	static TSharedPtr<FChunkDownloader> Get();
	static TSharedRef<FChunkDownloader> GetChecked();
	static TSharedRef<FChunkDownloader> GetOrCreate();
	static void Shutdown();

public:

	// initialize the download manager (populates the list of cached pak files from disk). Call only once.
	void Initialize(const FString& PlatformName, int32 TargetDownloadsInFlight);

	// unmount all chunks and cancel any downloads in progress (preserving partial downloads). 
	// Call only once, don't reuse this object, make a new one.
	void Finalize();

	// try to load a cached build ID from disk (good to do before updating build so it can possibly no-op)
	bool LoadCachedBuild(const FString& DeploymentName);

	// set the the content build id
	// if the content build id has changed, we pull the new BuildManifest from CDN and load it.
	// the client should compare ContentBuildId with its current embedded build id to determine if this content is 
	// even compatible BEFORE calling this function. e.g. ContentBuildId="v1.4.22-r23928293" we might consider BUILD_VERSION="1.4.1" 
	// compatible but BUILD_VERSION="1.3.223" incompatible (needing an update)
	void UpdateBuild(const FString& DeploymentName, const FString& ContentBuildId, const FCallback& Callback);

	// get the current status of the specified chunk
	EChunkStatus GetChunkStatus(int32 ChunkId) const;

	// return a list of all chunk IDs in the current manifest
	void GetAllChunkIds(TArray<int32>& OutChunkIds) const;

	// Download and mount all chunks then fire the callback (convenience wrapper managing multiple MountChunk calls)
	void MountChunks(const TArray<int32>& ChunkIds, const FCallback& Callback);

	// download all pak files, then asynchronously mount them in order (in order among themselves, async with game thread). 
	void MountChunk(int32 ChunkId, const FCallback& Callback);

	// Download (Cache) all pak files in these chunks then fire the callback (convenience wrapper managing multiple DownloadChunk calls)
	void DownloadChunks(const TArray<int32>& ChunkIds, const FCallback& Callback, int32 Priority = 0);

	// download all pak files in the chunk, but don't mount. Callback is fired when all paks have finished caching 
	// (whether success or failure). Downloads will retry forever, but might fail due to space issues.
	void DownloadChunk(int32 ChunkId, const FCallback& Callback, int32 Priority = 0);

	// flush any cached files (on disk) that are not currently being downloaded to or mounting (does not unmount the corresponding pak files).
	// this will include full and partial downloads, but not active downloads.
	int32 FlushCache();

	// validate all fully cached files (blocking) by attempting to read them and check their Version hash.
	// this automatically deletes any files that don't match. Returns the number of files deleted.
	// in this case best to return to a simple update map and reinitialize ChunkDownloader (or restart).
	int32 ValidateCache();

	// Snapshot stats and enter into loading screen mode (pauses all background downloads). Fires callback when all non-background 
	// downloads have completed. If no downloads/mounts are currently queued by the end of the frame, callback will fire next frame.
	void BeginLoadingMode(const FCallback& Callback);

	// if mounted, inspect the files contained in the chunk and export it as an array of file paths.
	// by default only cooked assets will be listed (.uasset and .umap files), but a flag can be set to export all contents, including .uexp files.
	bool GetChunkContent(int32 ChunkId, TArray<FString>& Content, bool bCookedOnly) const;

	// if mounted, inspect the files contained in the chunk and export it as an array of soft object pointers of the desired class.
	bool GetChunkContent(int32 ChunkId, const TSubclassOf<UObject>& Class, TArray<TSoftObjectPtr<UObject>>& Content) const;

	// Called whenever a chunk mounts (success or failure). ONLY USE THIS IF YOU WANT TO PASSIVELY LISTEN FOR MOUNTS (otherwise use the proper request callback on MountChunk)
	FPlatformChunkInstallMultiDelegate OnChunkMounted;
	// Called whenever a chunk registers (success or failure). ONLY USE THIS IF YOU WANT TO PASSIVELY LISTEN FOR REGISTRATIONS (otherwise use the proper request callback on RegisterChunk)
	FPlatformChunkInstallMultiDelegate OnChunkRegistered;

	// called each time a download attempt finishes (success or failure). ONLY USE THIS IF YOU WANT TO PASSIVELY LISTEN. Downloads retry until successful.
	TFunction<void(const FString& FileName, const FString& Url, uint64 SizeBytes, const FTimespan& DownloadTime, int32 HttpStatus)> OnDownloadAnalytics;

	// get the current content build ID
	inline const FString& GetContentBuildId() const { return ContentBuildId; }
	// get the most recent deployment name
	inline const FString& GetDeploymentName() const { return LastDeploymentName; }
	// get the most recent build base URLs
	inline const TArray<FString>& GetBuildBaseUrls() const { return BuildBaseUrls; }

	// get the last parsed data in the cached manifest, as saved by SaveLocalManifest
	// 
	// NOTE: Should be called only after Initialize, and before Finalize
	// If called before LoadCachedBuild and/or UpdateBuild, the provided data will have invalid ChunkID and RelativeUrl props
	// This is because Initialize parses the LocalManifest.txt file directly, which doesn't save them.
	inline const TArray<FPakManifestEntry>& GetLocalManifest() const { return LastLocalManifest; }

	// get the current loading stats (generally only useful if you're in loading mode see BeginLoadingMode)
	inline const FChunkStats& GetLoadingStats() const { return LoadingModeStats; }

	// get current number of download requests, so we know whether download is in progress. Downloading Requests will be removed from this array in it's FDownloadCustom::OnCompleted callback.
	inline int32 GetNumDownloadRequests() const { return DownloadRequests.Num(); }

	static void DumpLoadedChunks();

	// chunk status as logable string
	static const TCHAR* ChunkStatusToString(EChunkStatus Status);

protected:
	friend class UChunkDownloaderSubsystem;
	friend class FChunkDownloaderCustomModule;
	friend class FChunkDownloaderPlatformWrapper;
	friend class FDownload;

	FChunkDownloader();

	static bool WriteStringAsUtf8TextFile(const FString& FileText, const FString& FilePath);
	static bool CheckFileSha1Hash(const FString& FullPathOnDisk, const FString& Sha1HashStr);

	// Take in the path to a manifest text file in and parse its contents to build an array of FPakFileEntries to keep track of the pak files that are expected to be downloaded and mounted.
	// Optionally, a pointer to map of strings to strings can be provided to output the properties included in the manifest, used mainly for file versioning.
	static TArray<FPakManifestEntry> ParseManifest(const FString& ManifestPath, TMap<FString, FString>* Properties = nullptr);

	static FString GetRootDir(const FString& MountPoint);
	static bool IsMountingToRoot(const FString& MountPoint);
	static bool IsMountPointRegistered(const FString& MountPoint);

	// Take in a the mount point of a pak file (as provided by FPakFile::GetMountPoint()) and try to figure out the root direction to register it with (with FPackageName::RegisterMountPoint()). 
	// For pak files that were packaged together with the main project, this is not necessary because all required mount points are tracked and registered automatically.
	// But for pak files that are generated outside the main project, there's no way for it to track them, and registration must be made explicitly.
	static bool ParseRootDir(const FString& MountPoint, FString& Result);

private:
	class FMultiCallback;

	enum class ERegistryStatus : uint8 { Untracked, Registered, Unregistered };

	// entry per pak file 
	// CUSTOM: renamed because there is an FPakFile class in IPlatformFilePak.h and unlike Epic's ChunkDownloader plugin, we need to use it.
	struct FPakFileRecord
	{
		FPakManifestEntry Entry;
		bool bIsCached = false;
		bool bIsMounted = false;
		bool bIsEmbedded = false;
		ERegistryStatus IsRegistered = ERegistryStatus::Untracked;

		// Pointer to the mounted pak file object.
		TRefCountPtr<class FPakFile> Pak;

		// grows as the file is downloaded. See Entry.FileSize for the target size
		uint64 SizeOnDisk = 0;

		// async download
		int32 Priority = 0;
		TSharedPtr<FDownload> Download;
		TArray<FChunkDownloader::FCallback> PostDownloadCallbacks;
	};

	// represents an async mount
	class FPakMountWork;
	struct FPakMountWorkResult
	{
		FPakMountWorkResult(class FPakFile* Pak) : Pak(Pak) {}
		TRefCountPtr<FPakFile> Pak;
		ERegistryStatus IsRegistered = ERegistryStatus::Untracked;
	};
	typedef FAsyncTask<FPakMountWork> FMountTask;

	// entry per chunk
	struct FChunk
	{
		int32 ChunkId = -1;
		bool bIsMounted = false;
		TArray<TSharedRef<FPakFileRecord>> PakFiles;

		inline bool IsCached() const
		{
			for (const auto& PakFile : PakFiles)
			{
				if (!PakFile->bIsCached)
				{
					return false;
				}
			}
			return true;
		}

		inline bool IsRegistered() const
		{
			checkf(bIsMounted, TEXT("Unmounted PakFiles are always flagged as untracked so this would return a false positive. Always make sure the chunk is mounted before calling this."))
				for (const auto& PakFile : PakFiles)
				{
					if (PakFile->IsRegistered == ERegistryStatus::Unregistered)
					{
						return false;
					}
				}
			return true;
		}

		// async mount
		FMountTask* MountTask = nullptr;
	};

	void SetContentBuildId(const FString& DeploymentName, const FString& NewContentBuildId);

	// block waiting for any pending mounts to finish
	// then collect garbage to clean up any references to chunks.
	// then create entries for any new chunks
	// for any chunks that change, cancel downloads and unmount invalid paks (and any after invalid paks).
	// If a changed chunk was mounted, then inline mount all non-mounted paks in the new list (in order)  
	// then unload any chunks that no longer exist (cancel downloads and unmount all paks)
	void LoadManifest(const TArray<FPakManifestEntry>& PakFiles);

	void TryLoadBuildManifest(int32 TryNumber);
	void TryDownloadBuildManifest(int32 TryNumber);
	void SaveLocalManifest(bool bForce);

	void WaitForMounts();

	bool UpdateLoadingMode();
	void ComputeLoadingStats();

	void UnmountPakFile(const TSharedRef<FPakFileRecord>& PakFile);
	void CancelDownload(const TSharedRef<FPakFileRecord>& PakFile, bool bResult);

	void DownloadPakFileInternal(const TSharedRef<FPakFileRecord>& PakFile, const FCallback& Callback, int32 Priority);
	void MountChunkInternal(FChunk& Chunk, const FCallback& Callback);
	void DownloadChunkInternal(const FChunk& Chunk, const FCallback& Callback, int32 Priority);
	void CompleteMountTask(FChunk& Chunk);

	bool UpdateMountTasks(float dts);
	void ExecuteNextTick(const FCallback& Callback, bool bSuccess);

	void IssueDownloads();

private:

	// cumulative stats for loading screen mode
	FChunkStats LoadingModeStats;
	TArray<FCallback> PostLoadCallbacks;
	int32 LoadingCompleteLatch = 0;

	FCallback UpdateBuildCallback;

	// platform name (determines the manifest)
	FString PlatformName;

	// folders to save pak files into on disk
	FString CacheFolder;

	// content folder where we can find some chunks shipped with the build
	FString EmbeddedFolder;

	// build specific ID and URL paths
	FString LastDeploymentName;
	FString ContentBuildId;
	TArray<FString> BuildBaseUrls;

	// a copy of the data in the local manifest, updated everytime SaveLocalManifest() is called.
	TArray<FPakManifestEntry> LastLocalManifest;

	// chunk id to chunk record
	TMap<int32, TSharedRef<FChunk>> Chunks;

	// pak file name to pak file record
	TMap<FString, TSharedRef<FPakFileRecord>> PakFiles;

	// pak files embedded in the build (immutable, compressed)
	TMap<FString, FPakManifestEntry> EmbeddedPaks;

	// do we need to save the manifest (done whenever new downloads have started)
	bool bNeedsManifestSave = false;

	// handle for the per-frame mount ticker in the main thread
	FTSTicker::FDelegateHandle MountTicker;

	// manifest download request
	TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> ManifestRequest;

	// maximum number of downloads to allow concurrently
	int32 TargetDownloadsInFlight = 1;

	// list of pak files that have been requested
	TArray<TSharedRef<FPakFileRecord>> DownloadRequests;
};

#if UE_ENABLE_INCLUDE_ORDER_DEPRECATED_IN_5_2
#include "CoreMinimal.h"
#endif
