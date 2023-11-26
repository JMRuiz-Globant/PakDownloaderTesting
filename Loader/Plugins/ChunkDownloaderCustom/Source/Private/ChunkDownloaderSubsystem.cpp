// Fill out your copyright notice in the Description page of Project Settings.

#include "ChunkDownloaderSubsystem.h"
#include "ChunkDownloader.h"

void UChunkDownloaderSubsystem::Initialize(FSubsystemCollectionBase& Collection) {
	const FString& PlatformName = FPlatformProperties::IniPlatformName();
	int32 TargetDownloadsInFlight = FGenericPlatformMisc::NumberOfCores();
	FChunkDownloader::GetOrCreate()->Initialize(PlatformName, TargetDownloadsInFlight);
}

void UChunkDownloaderSubsystem::Deinitialize() {
	FChunkDownloader::Shutdown();
}

bool UChunkDownloaderSubsystem::LoadCachedBuild(const FString& DeploymentName)
{
	return FChunkDownloader::GetChecked()->LoadCachedBuild(DeploymentName);
}

void UChunkDownloaderSubsystem::UpdateBuild(const FString& DeploymentName, const FString& ContentBuildId, FCallbackDelegate Callback)
{
	FChunkDownloader::GetChecked()->UpdateBuild(DeploymentName, ContentBuildId, [Callback](bool bSuccess) { Callback.ExecuteIfBound(bSuccess); });
}

void UChunkDownloaderSubsystem::UpdateBuild(const FString& DeploymentName, const FString& ContentBuildId, FCallback Callback)
{
	FChunkDownloader::GetChecked()->UpdateBuild(DeploymentName, ContentBuildId, Callback);
}

FString UChunkDownloaderSubsystem::GetContentBuildId() const
{
	return FChunkDownloader::GetChecked()->GetContentBuildId();
}

FString UChunkDownloaderSubsystem::GetDeploymentName() const
{
	return FChunkDownloader::GetChecked()->GetDeploymentName();
}

TArray<FString> UChunkDownloaderSubsystem::GetBuildBaseUrls() const
{
	return FChunkDownloader::GetChecked()->GetBuildBaseUrls();
}

TArray<FPakManifestEntry> UChunkDownloaderSubsystem::GetLocalManifest() const
{
	return FChunkDownloader::GetChecked()->GetLocalManifest();
}

void UChunkDownloaderSubsystem::DumpLoadedChunks()
{
	FChunkDownloader::GetChecked()->DumpLoadedChunks();
}

FString UChunkDownloaderSubsystem::ChunkStatusToString(EChunkStatus Status)
{
	return FChunkDownloader::ChunkStatusToString(Status);
}

EChunkStatus UChunkDownloaderSubsystem::GetChunkStatus(int32 ChunkId) const
{
	return FChunkDownloader::GetChecked()->GetChunkStatus(ChunkId);
}

void UChunkDownloaderSubsystem::GetAllChunkIds(TArray<int32>& OutChunkIds) const
{
	FChunkDownloader::GetChecked()->GetAllChunkIds(OutChunkIds);
}

void UChunkDownloaderSubsystem::MountChunks(const TArray<int32>& ChunkIds, FCallbackDelegate Callback)
{
	FChunkDownloader::GetChecked()->MountChunks(ChunkIds, [Callback](bool bSuccess) { Callback.ExecuteIfBound(bSuccess); });
}

void UChunkDownloaderSubsystem::MountChunks(const TArray<int32>& ChunkIds, FCallback Callback)
{
	FChunkDownloader::GetChecked()->MountChunks(ChunkIds, Callback);
}

void UChunkDownloaderSubsystem::MountChunk(int32 ChunkId, FCallbackDelegate Callback)
{
	FChunkDownloader::GetChecked()->MountChunk(ChunkId, [Callback](bool bSuccess) { Callback.ExecuteIfBound(bSuccess); });
}

void UChunkDownloaderSubsystem::MountChunk(int32 ChunkId, FCallback Callback)
{
	FChunkDownloader::GetChecked()->MountChunk(ChunkId, Callback);
}

void UChunkDownloaderSubsystem::DownloadChunks(const TArray<int32>& ChunkIds, FCallbackDelegate Callback, int32 Priority)
{
	FChunkDownloader::GetChecked()->DownloadChunks(ChunkIds, [Callback](bool bSuccess) { Callback.ExecuteIfBound(bSuccess); }, Priority);
}

void UChunkDownloaderSubsystem::DownloadChunks(const TArray<int32>& ChunkIds, FCallback Callback, int32 Priority)
{
	FChunkDownloader::GetChecked()->DownloadChunks(ChunkIds, Callback, Priority);
}

void UChunkDownloaderSubsystem::DownloadChunk(int32 ChunkId, FCallbackDelegate Callback, int32 Priority)
{
	FChunkDownloader::GetChecked()->DownloadChunk(ChunkId, [Callback](bool bSuccess) { Callback.ExecuteIfBound(bSuccess); }, Priority);
}

void UChunkDownloaderSubsystem::DownloadChunk(int32 ChunkId, FCallback Callback, int32 Priority)
{
	FChunkDownloader::GetChecked()->DownloadChunk(ChunkId, Callback, Priority);
}

int32 UChunkDownloaderSubsystem::FlushCache()
{
	return FChunkDownloader::GetChecked()->FlushCache();
}

int32 UChunkDownloaderSubsystem::ValidateCache()
{
	return FChunkDownloader::GetChecked()->ValidateCache();
}

void UChunkDownloaderSubsystem::BeginLoadingMode(FCallbackDelegate Callback)
{
	FChunkDownloader::GetChecked()->BeginLoadingMode([Callback](bool bSuccess) { Callback.ExecuteIfBound(bSuccess); });
}

void UChunkDownloaderSubsystem::BeginLoadingMode(FCallback Callback)
{
	FChunkDownloader::GetChecked()->BeginLoadingMode(Callback);
}

void UChunkDownloaderSubsystem::GetLoadingStats(FChunkStats& Stats) const
{
	Stats = FChunkDownloader::GetChecked()->GetLoadingStats();
}

int32 UChunkDownloaderSubsystem::GetNumDownloadRequests() const
{
	return FChunkDownloader::GetChecked()->GetNumDownloadRequests();
}

void UChunkDownloaderSubsystem::GetChunkContentPaths(int32 ChunkId, TArray<FString>& Content, bool bCookedOnly)
{
	FChunkDownloader::GetChecked()->GetChunkContent(ChunkId, Content, bCookedOnly);
}

void UChunkDownloaderSubsystem::GetChunkContent(int32 ChunkId, TSubclassOf<UObject> Class, TArray<TSoftObjectPtr<UObject>>& Content)
{
	FChunkDownloader::GetChecked()->GetChunkContent(ChunkId, Class, Content);
}




