#include "ChunkDownloaderLatentActions.h"

void UCDL_AsyncActionBase::Activate()
{
	// This is a somewhat convoluted way to delegate the call to the desired function without having to store anything but a lambda.
	if (!Function([Action = TWeakObjectPtr<UCDL_AsyncActionBase>(this)](bool bSuccess)
		{
			if (Action.IsValid())
			{
				Action->Complete(bSuccess);
			}
		}))
	{
		Complete(false);
	}
}

void UCDL_AsyncActionBase::Complete(bool bSuccess)
{
	if (IsActive())
	{
		(bSuccess ? OnSuccess : OnFailure).Broadcast();
		Cancel();
	}
}

UCDL_UpdateBuild_AsyncAction* UCDL_UpdateBuild_AsyncAction::UpdateBuild(FLatentActionInfo LatentInfo, UChunkDownloaderSubsystem* Target, const FString& DeploymentName, const FString& ContentBuildId, bool bPreloadCachedBuild)
{
	if (Target)
	{
			if (auto Result = ULatentAsyncAction::Create<UCDL_UpdateBuild_AsyncAction>(Target->GetGameInstance(), LatentInfo.CallbackTarget, LatentInfo.UUID))
			{
				Result->Function = FFunction([Target = TWeakObjectPtr<UChunkDownloaderSubsystem>(Target), DeploymentName, ContentBuildId, bPreloadCachedBuild](const FCallback& Callback)
					{
						if (Target.IsValid())
						{
							if (bPreloadCachedBuild)
							{
								Target->LoadCachedBuild(DeploymentName);
							}
							Target->UpdateBuild(DeploymentName, ContentBuildId, Callback);
							return true;
						}
						return false;
					});
				return Result;
			}
		
	}
	return nullptr;
}

UCDL_MountChunks_AsyncAction* UCDL_MountChunks_AsyncAction::MountChunks(FLatentActionInfo LatentInfo, UChunkDownloaderSubsystem* Target, const TArray<int32>& ChunkIds)
{
	if (Target)
	{

			if (auto Result = ULatentAsyncAction::Create<UCDL_MountChunks_AsyncAction>(Target->GetGameInstance(), LatentInfo.CallbackTarget, LatentInfo.UUID))
			{
				Result->Function = FFunction([Target = TWeakObjectPtr<UChunkDownloaderSubsystem>(Target), ChunkIds](const FCallback& Callback)
					{
						if (Target.IsValid())
						{
							Target->MountChunks(ChunkIds, Callback);
							return true;
						}
						return false;
					});
				return Result;
			}
		
	}
	return nullptr;
}

UCDL_MountChunk_AsyncAction* UCDL_MountChunk_AsyncAction::MountChunk(FLatentActionInfo LatentInfo, UChunkDownloaderSubsystem* Target, int32 ChunkId)
{
	if (Target)
	{

			if (auto Result = ULatentAsyncAction::Create<UCDL_MountChunk_AsyncAction>(Target->GetGameInstance(), LatentInfo.CallbackTarget, LatentInfo.UUID))
			{
				Result->Function = FFunction([Target = TWeakObjectPtr<UChunkDownloaderSubsystem>(Target), ChunkId](const FCallback& Callback)
					{
						if (Target.IsValid())
						{
							Target->MountChunk(ChunkId, Callback);
							return true;
						}
						return false;
					});
				return Result;
			}
		
	}
	return nullptr;
}

UCDL_DownloadChunks_AsyncAction* UCDL_DownloadChunks_AsyncAction::DownloadChunks(FLatentActionInfo LatentInfo, UChunkDownloaderSubsystem* Target, const TArray<int32>& ChunkIds, int32 Priority)
{
	if (Target)
	{

			if (auto Result = ULatentAsyncAction::Create<UCDL_DownloadChunks_AsyncAction>(Target->GetGameInstance(), LatentInfo.CallbackTarget, LatentInfo.UUID))
			{
				Result->Function = FFunction([Target = TWeakObjectPtr<UChunkDownloaderSubsystem>(Target), ChunkIds, Priority](const FCallback& Callback)
					{
						if (Target.IsValid())
						{
							Target->DownloadChunks(ChunkIds, Callback, Priority);
							return true;
						}
						return false;
					});
				return Result;
			}
		
	}
	return nullptr;
}

UCDL_DownloadChunk_AsyncAction* UCDL_DownloadChunk_AsyncAction::DownloadChunk(FLatentActionInfo LatentInfo, UChunkDownloaderSubsystem* Target, int32 ChunkId, int32 Priority)
{
	if (Target)
	{

			if (auto Result = ULatentAsyncAction::Create<UCDL_DownloadChunk_AsyncAction>(Target->GetGameInstance(), LatentInfo.CallbackTarget, LatentInfo.UUID))
			{
				Result->Function = FFunction([Target = TWeakObjectPtr<UChunkDownloaderSubsystem>(Target), ChunkId, Priority](const FCallback& Callback)
					{
						if (Target.IsValid())
						{
							Target->DownloadChunk(ChunkId, Callback, Priority);
							return true;
						}
						return false;
					});
				return Result;
			}
		
	}
	return nullptr;
}


