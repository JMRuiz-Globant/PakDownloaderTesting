#pragma once

#include "ChunkDownloaderCommon.generated.h"

USTRUCT(BlueprintType, meta=(
	HasNativeBreak="ChunkDownloaderCustom.ChunkDownloaderCommonUtils.BreakPakManifestEntry", 
	HasNativeMake = "ChunkDownloaderCustom.ChunkDownloaderCommonUtils.MakePakManifestEntry"))
struct CHUNKDOWNLOADERCUSTOM_API FPakManifestEntry
{
	GENERATED_BODY()

	// unique name of the pak file (not path, i.e. no folder)
	FString FileName;
	
	// final size of the file in bytes
	uint64 FileSize = 0;

	// unique ID representing a particular version of this pak file
	// when it is used for validation (not done on golden path, but can be requested) this is assumed 
	// to be a SHA1 hash if it begins with "SHA1:" otherwise it's considered just a unique ID.
	FString FileVersion;

	// chunk ID this pak file is assigned to
	int32 ChunkId = -1;

	// URL for this pak file (relative to CDN root, includes build-specific folder)
	FString RelativeUrl;
};

USTRUCT(BlueprintType, meta = (
	HasNativeBreak = "ChunkDownloaderCustom.ChunkDownloaderCommonUtils.BreakChunkStats",
	HasNativeMake = "ChunkDownloaderCustom.ChunkDownloaderCommonUtils.MakeChunkStats"))
struct CHUNKDOWNLOADERCUSTOM_API FChunkStats
{
	GENERATED_BODY()

	// number of pak files downloaded
	int32 FilesDownloaded = 0;
	int32 TotalFilesToDownload = 0;

	// number of bytes downloaded
	uint64 BytesDownloaded = 0;
	uint64 TotalBytesToDownload = 0;

	// number of chunks mounted (chunk is an ordered array of paks)
	int32 ChunksMounted = 0;
	int32 TotalChunksToMount = 0;

	// UTC time that loading began (for rate estimates)
	FDateTime LoadingStartTime = FDateTime::MinValue();
	FText LastError;
};

UENUM(BlueprintType)
enum class EChunkStatus : uint8
{
	Mounted, // chunk is cached locally and mounted in RAM
	Cached, // chunk is fully cached locally but not mounted
	Downloading, // chunk is partially cached locally, not mounted, download in progress
	Partial, // chunk is partially cached locally, not mounted, download NOT in progress
	Remote, // no local caching has started
	Unknown, // no paks are included in this chunk, can consider it either an error or fully mounted depending
};

UCLASS()
class CHUNKDOWNLOADERCUSTOM_API UChunkDownloaderCommonUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Utilities|Pak Manifest Entry", meta = (CompactNodeTitle = "->"))
	static void BreakPakManifestEntry(UPARAM(ref) FPakManifestEntry& PakFile, FString& FileName, FString& FileSize, FString& FileVersion, int32& ChunkId, FString& RelativeUrl)
	{
		FileName = PakFile.FileName;
		FileSize = FString::Printf(TEXT("%llu"), PakFile.FileSize);
		FileVersion = PakFile.FileVersion;
		ChunkId = PakFile.ChunkId;
		RelativeUrl = PakFile.RelativeUrl;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Utilities|Pak Manifest Entry")
	static void MakePakManifestEntry(FPakManifestEntry& PakFile, FString FileName, FString FileSize, FString FileVersion, int32 ChunkId, FString RelativeUrl)
	{
		PakFile.FileName = FileName;
		PakFile.FileSize = FCString::Strtoui64(*FileSize, NULL, 10);
		PakFile.FileVersion = FileVersion;
		PakFile.ChunkId = ChunkId;
		PakFile.RelativeUrl = RelativeUrl;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Utilities|Chunk Stats", meta = (CompactNodeTitle="->"))
	static void BreakChunkStats(UPARAM(ref) FChunkStats& Stats, int32& FilesDownloaded, int32& TotalFilesToDownload, FString& BytesDownloaded, FString& TotalBytesToDownload, int32& ChunksMounted, int32& TotalChunksToMount, FDateTime& LoadingStartTime, FText& LastError)
	{
		FilesDownloaded = Stats.FilesDownloaded;
		TotalFilesToDownload = Stats.TotalFilesToDownload;
		BytesDownloaded = FString::Printf(TEXT("%llu"), Stats.BytesDownloaded);
		TotalBytesToDownload = FString::Printf(TEXT("%llu"), Stats.TotalBytesToDownload);
		ChunksMounted = Stats.ChunksMounted;
		TotalChunksToMount = Stats.TotalChunksToMount;
		LoadingStartTime = Stats.LoadingStartTime;
		LastError = Stats.LastError;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Utilities|Chunk Stats")
	static void MakeChunkStats(FChunkStats& Stats, int32 FilesDownloaded, int32 TotalFilesToDownload, FString BytesDownloaded, FString TotalBytesToDownload, int32 ChunksMounted, int32 TotalChunksToMount, FDateTime LoadingStartTime, FText LastError)
	{
		Stats.FilesDownloaded = FilesDownloaded;
		Stats.TotalFilesToDownload = TotalFilesToDownload;
		Stats.BytesDownloaded = FCString::Strtoui64(*BytesDownloaded, NULL, 10);
		Stats.TotalBytesToDownload = FCString::Strtoui64(*TotalBytesToDownload, NULL, 10);
		Stats.ChunksMounted = ChunksMounted;
		Stats.TotalChunksToMount = TotalChunksToMount;
		Stats.LoadingStartTime = LoadingStartTime;
		Stats.LastError = LastError;
	}
};

#if UE_ENABLE_INCLUDE_ORDER_DEPRECATED_IN_5_2
#include "CoreMinimal.h"
#endif