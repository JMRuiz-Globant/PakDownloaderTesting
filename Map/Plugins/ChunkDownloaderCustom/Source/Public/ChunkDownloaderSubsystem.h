
#pragma once

#include "Subsystems/GameInstanceSubsystem.h"
#include "ChunkDownloaderSubsystem.generated.h"

UCLASS(Meta=(DisplayName="Chunk Downloader"))
class CHUNKDOWNLOADERCUSTOM_API UChunkDownloaderSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_DELEGATE_OneParam(FCallbackDelegate, bool, bSuccess);
	typedef TFunction<void(bool bSuccess)> FCallback;

	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;

	// try to load a cached build ID from disk (good to do before updating build so it can possibly no-op)
	UFUNCTION(BlueprintCallable, Category = "Chunk Downloader")
	bool LoadCachedBuild(const FString& DeploymentName);
	
	// set the the content build id
	// if the content build id has changed, we pull the new BuildManifest from CDN and load it.
	// the client should compare ContentBuildId with its current embedded build id to determine if this content is 
	// even compatible BEFORE calling this function. e.g. ContentBuildId="v1.4.22-r23928293" we might consider BUILD_VERSION="1.4.1" 
	// compatible but BUILD_VERSION="1.3.223" incompatible (needing an update)
    UFUNCTION(BlueprintCallable, Category = "Chunk Downloader", meta=(AdvancedDisplay="bPreloadCachedBuild"))
    void UpdateBuild(const FString& DeploymentName, const FString& ContentBuildId, bool bPreloadCachedBuild, FCallbackDelegate Callback);
	void UpdateBuild(const FString& DeploymentName, const FString& ContentBuildId, bool bPreloadCachedBuild, FCallback Callback);

	// get the current content build ID
	UFUNCTION(BlueprintPure, Category = "Chunk Downloader")
	FString GetContentBuildId() const;
	
	// get the most recent deployment name
	UFUNCTION(BlueprintPure, Category = "Chunk Downloader")
	FString GetDeploymentName() const;
	
	// get the most recent build base URLs
	UFUNCTION(BlueprintPure, Category = "Chunk Downloader")
	TArray<FString> GetBuildBaseUrls() const;

	// get the last parsed data in the cached manifest, as saved by SaveLocalManifest
	// If called before LoadCachedBuild and/or UpdateBuild, the provided data will have invalid ChunkID and RelativeUrl props
	// This is because before that only the LocalManifest.txt file is parsed, which doesn't save that data
	UFUNCTION(BlueprintPure, Category = "Chunk Downloader")
	TArray<FPakManifestEntry> GetLocalManifest() const;

	UFUNCTION(BlueprintCallable, Category = "Chunk Downloader")
	static void DumpLoadedChunks();

	// chunk status as logable string
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Utilities|String", meta = (DisplayName = "To String (Chunk Status)", CompactNodeTitle = "->", BlueprintAutocast))
	static FString ChunkStatusToString(EChunkStatus Status);

	// get the current status of the specified chunk
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chunk Downloader")
	EChunkStatus GetChunkStatus(int32 ChunkId) const;

	// return a list of all chunk IDs in the current manifest
	UFUNCTION(BlueprintCallable, Category = "Chunk Downloader")
	void GetAllChunkIds(TArray<int32>& OutChunkIds) const;

	// Unmount all chunks then fire the callback (convenience wrapper managing multiple UnmountChunk calls)
	UFUNCTION(BlueprintCallable, Category = "Chunk Downloader")
	void UnmountChunks(const TArray<int32>& ChunkIds, FCallbackDelegate Callback);
	void UnmountChunks(const TArray<int32>& ChunkIds, FCallback Callback);

	// Asynchronously unmount pak files in reverse order (in order among themselves, async with game thread).
	UFUNCTION(BlueprintCallable, Category = "Chunk Downloader")
	void UnmountChunk(int32 ChunkId, FCallbackDelegate Callback);
	void UnmountChunk(int32 ChunkId, FCallback Callback);

	// Download and mount all chunks then fire the callback (convenience wrapper managing multiple MountChunk calls)
	// @param bPreScanAssets	If true, assets contained in the chunk files will be scanned with the Asset Registry after mounting.
	UFUNCTION(BlueprintCallable, Category = "Chunk Downloader", meta=(AdvancedDisplay="bPreScanAssets"))
	void MountChunks(const TArray<int32>& ChunkIds, bool bPreScanAssets, FCallbackDelegate Callback);
	void MountChunks(const TArray<int32>& ChunkIds, bool bPreScanAssets, FCallback Callback);

	// download all pak files, then asynchronously mount them in order (in order among themselves, async with game thread).
	// @param bPreScanAssets	If true, assets contained in the chunk files will be scanned with the Asset Registry after mounting.
	UFUNCTION(BlueprintCallable, Category = "Chunk Downloader", meta = (AdvancedDisplay = "bPreScanAssets"))
	void MountChunk(int32 ChunkId, bool bPreScanAssets, FCallbackDelegate Callback);
	void MountChunk(int32 ChunkId, bool bPreScanAssets, FCallback Callback);

	// Download (Cache) all pak files in these chunks then fire the callback (convenience wrapper managing multiple DownloadChunk calls)
	UFUNCTION(BlueprintCallable, Category = "Chunk Downloader", meta = (AdvancedDisplay = "Priority"))
	void DownloadChunks(const TArray<int32>& ChunkIds, FCallbackDelegate Callback, int32 Priority = 0);
	void DownloadChunks(const TArray<int32>& ChunkIds, FCallback Callback, int32 Priority = 0);

	// download all pak files in the chunk, but don't mount. Callback is fired when all paks have finished caching 
	// (whether success or failure). Downloads will retry forever, but might fail due to space issues.
	UFUNCTION(BlueprintCallable, Category = "Chunk Downloader", meta = (AdvancedDisplay = "Priority"))
	void DownloadChunk(int32 ChunkId, FCallbackDelegate Callback, int32 Priority = 0);
	void DownloadChunk(int32 ChunkId, FCallback Callback, int32 Priority = 0);

	// flush any cached files (on disk) that are not currently being downloaded to or mounting (does not unmount the corresponding pak files).
	// this will include full and partial downloads, but not active downloads.
	UFUNCTION(BlueprintCallable, Category = "Chunk Downloader")
	int32 FlushCache();

	// validate all fully cached files (blocking) by attempting to read them and check their Version hash.
	// this automatically deletes any files that don't match. Returns the number of files deleted.
	// in this case best to return to a simple update map and reinitialize ChunkDownloader (or restart).
	UFUNCTION(BlueprintCallable, Category = "Chunk Downloader")
	int32 ValidateCache();

	// Snapshot stats and enter into loading screen mode (pauses all background downloads). Fires callback when all non-background 
	// downloads have completed. If no downloads/mounts are currently queued by the end of the frame, callback will fire next frame.
	UFUNCTION(BlueprintCallable, Category = "Chunk Downloader")
	void BeginLoadingMode(FCallbackDelegate Callback);
	void BeginLoadingMode(FCallback Callback);

	// get the current loading stats (generally only useful if you're in loading mode see BeginLoadingMode)
	UFUNCTION(BlueprintPure, Category = "Chunk Downloader|Stats")
	void GetLoadingStats(FChunkStats& Stats) const;

	// get current number of download requests, so we know whether download is in progress. Downloading Requests will be removed from this array in it's FDownloadCustom::OnCompleted callback.
	UFUNCTION(BlueprintPure, Category = "Chunk Downloader|Stats")
	int32 GetNumDownloadRequests() const;

	// if mounted, inspect the files contained in the chunk and scan them all with the Asset Registry, synchronously. Return the number of assets registered, which can be 0 if the Asset Registry was already aware of them from a prior scan.
	// if not properly mounted, returns -1.
	UFUNCTION(BlueprintCallable, Category = "Chunk Downloader")
	int32 ScanAssetsInChunk(int32 ChunkId);

	// if mounted, inspect the files contained in the chunk and export it as an array of file paths.
	// by default only cooked assets will be listed (.uasset and .umap files), but a flag can be set to export all contents, including .uexp files.
	UFUNCTION(BlueprintCallable, Category = "Chunk Downloader", meta=(AdvancedDisplay="bCookedOnly"))
	void GetChunkContentPaths(int32 ChunkId, TArray<FString>& Content, bool bCookedOnly = true);

	// if mounted, inspect the files contained in the chunk and export it as an array of package names.
	UFUNCTION(BlueprintCallable, Category = "Chunk Downloader")
	void GetChunkContentPackageNames(int32 ChunkId, TArray<FString>& Content);

	/** if mounted, inspect the files contained in the chunk and export it as an array of soft object reference.
	 * @param	bPreScanAssets	If set to true, perform a synchronous Asset Registry scan before searching for assets, to ensure that the game is aware of them.
	 * @param	PackageName		If not none, filter the pak contents for a specific package.
	 * @param	PackagePath		If not none, filter the pak contents for a specific folder.
	 * @param	bRecursivePath	If true and filtering by path, also search within folders inside the desired path. If false, only files in the exact directory will be considered.
	 * @param	Class			If valid, filter the pak contents for a specific class.
	 * @param	bRecursiveClass	If true and filtering by class, also include children of the desired class. If false, only assets of the exact class will be considered.
	 */
	UFUNCTION(BlueprintCallable, Category = "Chunk Downloader", meta = (AdvancedDisplay="2", AutoCreateRefTerm="Class", DeterminesOutputType="Class", DynamicOutputParam="Assets"))
	void GetAssetsInChunk(int32 ChunkId, TArray<TSoftObjectPtr<UObject>>& Assets, bool bPreScanAssets, FName PackageName, FName PackagePath, bool bRecursivePath, TSubclassOf<UObject> Class, bool bRecursiveClass);

	/** if mounted, inspect the files contained in the chunk and export the first one that matches the filters as a soft object reference.
	 * @param	bPreScanAssets	If set to true, perform a synchronous Asset Registry scan before searching for assets, to ensure that the game is aware of them.
	 * @param	PackageName		If not none, filter the pak contents for a specific package.
	 * @param	PackagePath		If not none, filter the pak contents for a specific folder.
	 * @param	bRecursivePath	If true and filtering by path, also search within folders inside the desired path. If false, only files in the exact directory will be considered.
	 * @param	Class			If valid, filter the pak contents for a specific class.
	 * @param	bRecursiveClass	If true and filtering by class, also include children of the desired class. If false, only assets of the exact class will be considered.
	 */
	UFUNCTION(BlueprintCallable, Category = "Chunk Downloader", meta = (AdvancedDisplay="2", AutoCreateRefTerm="Class", DeterminesOutputType="Class", DynamicOutputParam="Asset"))
	void FindAssetInChunk(int32 ChunkId, TSoftObjectPtr<UObject>& Asset, bool bPreScanAssets, FName PackageName, FName PackagePath, bool bRecursivePath, TSubclassOf<UObject> Class, bool bRecursiveClass);

};

#if UE_ENABLE_INCLUDE_ORDER_DEPRECATED_IN_5_2
#include "CoreMinimal.h"
#endif
