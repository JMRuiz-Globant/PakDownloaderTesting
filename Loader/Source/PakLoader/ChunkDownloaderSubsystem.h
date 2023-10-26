
#pragma once

#include "CoreMinimal.h"
#include "ChunkDownloader.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ChunkDownloaderSubsystem.generated.h"

DECLARE_DYNAMIC_DELEGATE_OneParam(FChunkDownloaderSubsystemCallback, bool, Succeeded);

/**
 * 
 */
UCLASS()
class UChunkDownloaderSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

protected:

	/** Tracks Whether or not our local manifest file is up to date with the one hosted on our website. */
	bool bIsDownloadManifestUpToDate;

public:
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;

	UFUNCTION(BlueprintPure, Category = "Patching|Stats")
	void GetLoadingProgress(int32& FilesDownloaded, int32& TotalFilesToDownload, float& DownloadPercent, int32& ChunksMounted, int32& TotalChunksToMount, float& MountPercent) const;	
	
	/** Updates the manifest file. */
	UFUNCTION(BlueprintCallable, Category = "Patching", meta = (DeploymentName = "Patcher-Live", ContentBuildId = "Patcher-Live"))
	void UpdateBuild(const FString& DeploymentName, const FString& ContentBuildId, FChunkDownloaderSubsystemCallback Callback);

	/** Starts the game patching process for the given chunk IDs. */
	UFUNCTION(BlueprintCallable, Category = "Patching")
	void PatchGame(const TArray<int32>& ChunkIds, FChunkDownloaderSubsystemCallback Callback);

	/** Creates a mounting point for the pak files and registers their contents with the Asset Registry. */
	UFUNCTION(BlueprintCallable, Category = "Patching")
	void RegisterMounts(FChunkDownloaderSubsystemCallback Callback);

private:

	/** 
	 * This is a copy of the function with the same name found in ChunkDownloader.cpp.
	 * We need it to identify our pak files, and this is where the chunk IDs are associated to the files themselves.
	 */
	static TArray<FPakFileEntry> ParseManifest(const FString& ManifestPath, TMap<FString, FString>* Properties = nullptr);
	
	/**
	 * This function takes a pak file's default mount point (accessible through FPakFile::GetMountPoint()) and parses it to provide the root directory required to mount it in the proper place.
	 * There's no simple method to do this since we need to be able to deal with pak files generated in other projects, which means we can't do quick comparisions with directories available in the FPaths class.
	*/
	static FString ParseRootDir(const FString& MountPoint);

};
