
#pragma once

#include "CoreMinimal.h"
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

	//Tracks Whether or not our local manifest file is up to date with the one hosted on our website
	bool bIsDownloadManifestUpToDate;

public:
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;
	
	UFUNCTION(BlueprintCallable, Category = "Patching", meta = (DeploymentName = "Patcher-Live", ContentBuildId = "Patcher-Live"))
	void UpdateBuild(const FString& DeploymentName, const FString& ContentBuildId, FChunkDownloaderSubsystemCallback Callback);

	/** Starts the game patching process. Returns false if the patching manifest is not up to date. */
	UFUNCTION(BlueprintCallable, Category = "Patching")
	void PatchGame(const TArray<int32>& ChunkIds, FChunkDownloaderSubsystemCallback Callback);


	UFUNCTION(BlueprintCallable, Category = "Patching")
	void RegisterMounts(FChunkDownloaderSubsystemCallback Callback);



	UFUNCTION(BlueprintPure, Category = "Patching|Stats")
	void GetLoadingProgress(int32& FilesDownloaded, int32& TotalFilesToDownload, float& DownloadPercent, int32& ChunksMounted, int32& TotalChunksToMount, float& MountPercent) const;

};
