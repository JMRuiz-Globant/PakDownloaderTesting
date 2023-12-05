
#pragma once

#include "LatentAsyncAction.h"
#include "ChunkDownloaderSubsystem.h"
#include "ChunkDownloaderLatentActions.generated.h"

/**
 * Base class for the callback functions to avoid boilerplate code.
 */
UCLASS(Abstract)
class CHUNKDOWNLOADERCUSTOM_API UCDL_AsyncActionBase : public ULatentAsyncAction
{
    GENERATED_BODY()

public:
	// Factory function
	// UFUNCTION(BlueprintCallable, Category = "Patching (Latent)", meta=(BlueprintInternalUseOnly="true", LatentInfo="LatentInfo", DefaultToSelf="Target"))
	// static UCDl_AsyncActionBase* FactoryFunction(FLatentActionInfo LatentInfo, UChunkDownloaderCustomSubsystem* Target
	// , <... params ...>
	//);

public:

	// Called to trigger the action once the delegates have been bound.
	virtual void Activate() override;
	// Called to execute a delegate and cancel the action.
	virtual void Complete(bool Success);

    // Build update succeeded.
    UPROPERTY(BlueprintAssignable, meta=(DisplayName="Success"))
	FLatentAsyncActionEvent OnSuccess;

    // Build update failed.
    UPROPERTY(BlueprintAssignable, meta = (DisplayName="Failure"))
	FLatentAsyncActionEvent OnFailure;

protected:
	typedef TFunction<void(bool bSuccess)> FCallback;
	typedef TFunction<bool(const FCallback& Callback)> FFunction;

	FFunction Function;
};

UCLASS(meta=(DisplayName="Chunk Downloader"))
class CHUNKDOWNLOADERCUSTOM_API UCDL_UpdateBuild_AsyncAction : public UCDL_AsyncActionBase
{
    GENERATED_BODY()

public:
	// set the the content build id, optionally preload cached build
	// if the content build id has changed, we pull the new BuildManifest from CDN and load it.
	// the client should compare ContentBuildId with its current embedded build id to determine if this content is 
	// even compatible BEFORE calling this function. e.g. ContentBuildId="v1.4.22-r23928293" we might consider BUILD_VERSION="1.4.1" 
	// compatible but BUILD_VERSION="1.3.223" incompatible (needing an update)
    UFUNCTION(BlueprintCallable, Category = "Chunk Downloader|Latent", meta=(BlueprintInternalUseOnly="true", LatentInfo = "LatentInfo", DefaultToSelf = "Target"))
    static UCDL_UpdateBuild_AsyncAction* UpdateBuild(FLatentActionInfo LatentInfo, UChunkDownloaderSubsystem* Target
		, const FString& DeploymentName, const FString& ContentBuildId, bool bPreloadCachedBuild = true
	);
};

UCLASS(meta=(DisplayName="Chunk Downloader"))
class CHUNKDOWNLOADERCUSTOM_API UCDL_UnmountChunks_AsyncAction : public UCDL_AsyncActionBase
{
    GENERATED_BODY()

public:

	// Unmount all chunks then fire the callback (convenience wrapper managing multiple UnmountChunk calls)
    UFUNCTION(BlueprintCallable, Category = "Chunk Downloader|Latent", meta=(BlueprintInternalUseOnly="true", LatentInfo="LatentInfo", DefaultToSelf="Target"))
	static UCDL_UnmountChunks_AsyncAction* UnmountChunks(FLatentActionInfo LatentInfo, UChunkDownloaderSubsystem* Target
		, const TArray<int32>& ChunkIds
	);
};

UCLASS(meta=(DisplayName="Chunk Downloader"))
class CHUNKDOWNLOADERCUSTOM_API UCDL_UnmountChunk_AsyncAction : public UCDL_AsyncActionBase
{
    GENERATED_BODY()

public:

	// Asynchronously unmount chunk in reverse order (in order among themselves, async with game thread).
    UFUNCTION(BlueprintCallable, Category = "Chunk Downloader|Latent", meta=(BlueprintInternalUseOnly="true", LatentInfo="LatentInfo", DefaultToSelf="Target"))
	static UCDL_UnmountChunk_AsyncAction* UnmountChunk(FLatentActionInfo LatentInfo, UChunkDownloaderSubsystem* Target
		, int32 ChunkId
	);
};

UCLASS(meta=(DisplayName="Chunk Downloader"))
class CHUNKDOWNLOADERCUSTOM_API UCDL_MountChunks_AsyncAction : public UCDL_AsyncActionBase
{
    GENERATED_BODY()

public:

	// Download and mount all chunks then fire the callback (convenience wrapper managing multiple MountChunk calls)
    UFUNCTION(BlueprintCallable, Category = "Chunk Downloader|Latent", meta=(BlueprintInternalUseOnly="true", LatentInfo="LatentInfo", DefaultToSelf="Target"))
	static UCDL_MountChunks_AsyncAction* MountChunks(FLatentActionInfo LatentInfo, UChunkDownloaderSubsystem* Target
		, const TArray<int32>& ChunkIds
	);
};

UCLASS(meta=(DisplayName="Chunk Downloader"))
class CHUNKDOWNLOADERCUSTOM_API UCDL_MountChunk_AsyncAction : public UCDL_AsyncActionBase
{
    GENERATED_BODY()

public:

	// download all pak files, then asynchronously mount them in order (in order among themselves, async with game thread).
    UFUNCTION(BlueprintCallable, Category = "Chunk Downloader|Latent", meta=(BlueprintInternalUseOnly="true", LatentInfo="LatentInfo", DefaultToSelf="Target"))
	static UCDL_MountChunk_AsyncAction* MountChunk(FLatentActionInfo LatentInfo, UChunkDownloaderSubsystem* Target
		, int32 ChunkId
	);
};

UCLASS(meta=(DisplayName="Chunk Downloader"))
class CHUNKDOWNLOADERCUSTOM_API UCDL_DownloadChunks_AsyncAction : public UCDL_AsyncActionBase
{
    GENERATED_BODY()

public:

	// Download (Cache) all pak files in these chunks then fire the callback (convenience wrapper managing multiple DownloadChunk calls)
    UFUNCTION(BlueprintCallable, Category = "Chunk Downloader|Latent", meta=(BlueprintInternalUseOnly="true", LatentInfo="LatentInfo", DefaultToSelf="Target"))
	static UCDL_DownloadChunks_AsyncAction* DownloadChunks(FLatentActionInfo LatentInfo, UChunkDownloaderSubsystem* Target
		, const TArray<int32>& ChunkIds, int32 Priority = 0
	);
};

UCLASS(meta=(DisplayName="Chunk Downloader"))
class CHUNKDOWNLOADERCUSTOM_API UCDL_DownloadChunk_AsyncAction : public UCDL_AsyncActionBase
{
    GENERATED_BODY()

public:

	// download all pak files in the chunk, but don't mount. Callback is fired when all paks have finished caching 
	// (whether success or failure). Downloads will retry forever, but might fail due to space issues.
    UFUNCTION(BlueprintCallable, Category = "Chunk Downloader|Latent", meta=(BlueprintInternalUseOnly="true", LatentInfo="LatentInfo", DefaultToSelf="Target"))
	static UCDL_DownloadChunk_AsyncAction* DownloadChunk(FLatentActionInfo LatentInfo, UChunkDownloaderSubsystem* Target
		, int32 ChunkId, int32 Priority = 0
	);
};

#if UE_ENABLE_INCLUDE_ORDER_DEPRECATED_IN_5_2
#include "CoreMinimal.h"
#endif