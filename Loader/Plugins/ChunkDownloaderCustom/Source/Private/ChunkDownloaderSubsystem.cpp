// Fill out your copyright notice in the Description page of Project Settings.

#include "ChunkDownloaderSubsystem.h"
#include "ChunkDownloader.h"

void UChunkDownloaderSubsystem::Initialize(FSubsystemCollectionBase& Collection) {
	const FString& PlatformName = FPlatformProperties::IniPlatformName();
	int32 TargetDownloadsInFlight = FGenericPlatformMisc::NumberOfCores();
	FChunkDownloaderCustom::GetOrCreate()->Initialize(PlatformName, TargetDownloadsInFlight);
}

void UChunkDownloaderSubsystem::Deinitialize() {
	FChunkDownloaderCustom::Shutdown();
}

bool UChunkDownloaderSubsystem::LoadCachedBuild(const FString& DeploymentName)
{
	return FChunkDownloaderCustom::GetChecked()->LoadCachedBuild(DeploymentName);
}

void UChunkDownloaderSubsystem::UpdateBuild(const FString& DeploymentName, const FString& ContentBuildId, bool bPreloadCachedBuild, FCallbackDelegate Callback)
{
	FChunkDownloaderCustom::GetChecked()->UpdateBuild(DeploymentName, ContentBuildId, [Callback](bool bSuccess) { Callback.ExecuteIfBound(bSuccess); }, bPreloadCachedBuild);
}

void UChunkDownloaderSubsystem::UpdateBuild(const FString& DeploymentName, const FString& ContentBuildId, bool bPreloadCachedBuild, FCallback Callback)
{
	FChunkDownloaderCustom::GetChecked()->UpdateBuild(DeploymentName, ContentBuildId, Callback, bPreloadCachedBuild);
}

FString UChunkDownloaderSubsystem::GetContentBuildId() const
{
	return FChunkDownloaderCustom::GetChecked()->GetContentBuildId();
}

FString UChunkDownloaderSubsystem::GetDeploymentName() const
{
	return FChunkDownloaderCustom::GetChecked()->GetDeploymentName();
}

TArray<FString> UChunkDownloaderSubsystem::GetBuildBaseUrls() const
{
	return FChunkDownloaderCustom::GetChecked()->GetBuildBaseUrls();
}

TArray<FPakManifestEntry> UChunkDownloaderSubsystem::GetLocalManifest() const
{
	return FChunkDownloaderCustom::GetChecked()->GetLocalManifest();
}

void UChunkDownloaderSubsystem::DumpLoadedChunks()
{
	FChunkDownloaderCustom::GetChecked()->DumpLoadedChunks();
}

FString UChunkDownloaderSubsystem::ChunkStatusToString(EChunkStatus Status)
{
	return FChunkDownloaderCustom::ChunkStatusToString(Status);
}

EChunkStatus UChunkDownloaderSubsystem::GetChunkStatus(int32 ChunkId) const
{
	return FChunkDownloaderCustom::GetChecked()->GetChunkStatus(ChunkId);
}

void UChunkDownloaderSubsystem::GetAllChunkIds(TArray<int32>& OutChunkIds) const
{
	FChunkDownloaderCustom::GetChecked()->GetAllChunkIds(OutChunkIds);
}

void UChunkDownloaderSubsystem::UnmountChunks(const TArray<int32>& ChunkIds, FCallbackDelegate Callback)
{
	FChunkDownloaderCustom::GetChecked()->UnmountChunks(ChunkIds, [Callback](bool bSuccess) { Callback.ExecuteIfBound(bSuccess); });
}

void UChunkDownloaderSubsystem::UnmountChunks(const TArray<int32>& ChunkIds, FCallback Callback)
{
	FChunkDownloaderCustom::GetChecked()->UnmountChunks(ChunkIds, Callback);
}

void UChunkDownloaderSubsystem::UnmountChunk(int32 ChunkId, FCallbackDelegate Callback)
{
	FChunkDownloaderCustom::GetChecked()->UnmountChunk(ChunkId, [Callback](bool bSuccess) { Callback.ExecuteIfBound(bSuccess); });
}

void UChunkDownloaderSubsystem::UnmountChunk(int32 ChunkId, FCallback Callback)
{
	FChunkDownloaderCustom::GetChecked()->UnmountChunk(ChunkId, Callback);
}

void UChunkDownloaderSubsystem::MountChunks(const TArray<int32>& ChunkIds, bool bPreScanAssets, FCallbackDelegate Callback)
{
	FChunkDownloaderCustom::GetChecked()->MountChunks(ChunkIds, [Callback](bool bSuccess) { Callback.ExecuteIfBound(bSuccess); }, bPreScanAssets);
}

void UChunkDownloaderSubsystem::MountChunks(const TArray<int32>& ChunkIds, bool bPreScanAssets, FCallback Callback)
{
	FChunkDownloaderCustom::GetChecked()->MountChunks(ChunkIds, Callback, bPreScanAssets);
}

void UChunkDownloaderSubsystem::MountChunk(int32 ChunkId, bool bPreScanAssets, FCallbackDelegate Callback)
{
	FChunkDownloaderCustom::GetChecked()->MountChunk(ChunkId, [Callback](bool bSuccess) { Callback.ExecuteIfBound(bSuccess); }, bPreScanAssets);
}

void UChunkDownloaderSubsystem::MountChunk(int32 ChunkId, bool bPreScanAssets, FCallback Callback)
{
	FChunkDownloaderCustom::GetChecked()->MountChunk(ChunkId, Callback, bPreScanAssets);
}

void UChunkDownloaderSubsystem::DownloadChunks(const TArray<int32>& ChunkIds, FCallbackDelegate Callback, int32 Priority)
{
	FChunkDownloaderCustom::GetChecked()->DownloadChunks(ChunkIds, [Callback](bool bSuccess) { Callback.ExecuteIfBound(bSuccess); }, Priority);
}

void UChunkDownloaderSubsystem::DownloadChunks(const TArray<int32>& ChunkIds, FCallback Callback, int32 Priority)
{
	FChunkDownloaderCustom::GetChecked()->DownloadChunks(ChunkIds, Callback, Priority);
}

void UChunkDownloaderSubsystem::DownloadChunk(int32 ChunkId, FCallbackDelegate Callback, int32 Priority)
{
	FChunkDownloaderCustom::GetChecked()->DownloadChunk(ChunkId, [Callback](bool bSuccess) { Callback.ExecuteIfBound(bSuccess); }, Priority);
}

void UChunkDownloaderSubsystem::DownloadChunk(int32 ChunkId, FCallback Callback, int32 Priority)
{
	FChunkDownloaderCustom::GetChecked()->DownloadChunk(ChunkId, Callback, Priority);
}

int32 UChunkDownloaderSubsystem::FlushCache()
{
	return FChunkDownloaderCustom::GetChecked()->FlushCache();
}

int32 UChunkDownloaderSubsystem::ValidateCache()
{
	return FChunkDownloaderCustom::GetChecked()->ValidateCache();
}

void UChunkDownloaderSubsystem::BeginLoadingMode(FCallbackDelegate Callback)
{
	FChunkDownloaderCustom::GetChecked()->BeginLoadingMode([Callback](bool bSuccess) { Callback.ExecuteIfBound(bSuccess); });
}

void UChunkDownloaderSubsystem::BeginLoadingMode(FCallback Callback)
{
	FChunkDownloaderCustom::GetChecked()->BeginLoadingMode(Callback);
}

void UChunkDownloaderSubsystem::GetLoadingStats(FChunkStats& Stats) const
{
	Stats = FChunkDownloaderCustom::GetChecked()->GetLoadingStats();
}

int32 UChunkDownloaderSubsystem::GetNumDownloadRequests() const
{
	return FChunkDownloaderCustom::GetChecked()->GetNumDownloadRequests();
}

int32 UChunkDownloaderSubsystem::ScanAssetsInChunk(int32 ChunkId)
{
	return FChunkDownloaderCustom::GetChecked()->ScanAssetsInChunk(ChunkId);
}

void UChunkDownloaderSubsystem::GetChunkContentPaths(int32 ChunkId, TArray<FString>& Content, bool bCookedOnly)
{
	Content.Empty();
	FChunkDownloaderCustom::GetChecked()->InspectChunkContent(ChunkId, [&Content](const FString&, const FString& PackagePath)
		{
			Content.Add(PackagePath);
			return true;
		}, bCookedOnly);
}

void UChunkDownloaderSubsystem::GetChunkContentPackageNames(int32 ChunkId, TArray<FString>& Content)
{
	Content.Empty();
	FChunkDownloaderCustom::GetChecked()->InspectChunkContent(ChunkId, [&Content](const FString& PackageStr, const FString&)
		{
			Content.Add(PackageStr);
			return true;
		});
}

void UChunkDownloaderSubsystem::GetAssetsInChunk(int32 ChunkId, TArray<TSoftObjectPtr<UObject>>& Assets, bool bPreScanAssets, FName PackageName, FName PackagePath, bool bRecursivePath, TSubclassOf<UObject> Class, bool bRecursiveClass)
{
	FChunkDownloaderCustom::GetChecked()->GetChunkContent(ChunkId, Assets, bPreScanAssets, PackageName, PackagePath, bRecursivePath, Class, bRecursiveClass);
}

void UChunkDownloaderSubsystem::FindAssetInChunk(int32 ChunkId, TSoftObjectPtr<UObject>& Asset, bool bPreScanAssets, FName PackageName, FName PackagePath, bool bRecursivePath, TSubclassOf<UObject> Class, bool bRecursiveClass)
{
	TArray<TSoftObjectPtr<UObject>> Assets;
	FChunkDownloaderCustom::GetChecked()->GetChunkContent(ChunkId, Assets, bPreScanAssets, PackageName, PackagePath, bRecursivePath, Class, bRecursiveClass);
	Asset = Assets.Num() > 0 ? Assets[0] : nullptr;
}



