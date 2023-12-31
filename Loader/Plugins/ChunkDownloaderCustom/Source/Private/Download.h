// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ChunkDownloader.h"
#include "PlatformStreamDownload.h"

class FDownloadChunk : public TSharedFromThis<FDownloadChunk>
{
public:
	FDownloadChunk(const TSharedRef<FChunkDownloaderCustom>& DownloaderIn, const TSharedRef<FChunkDownloaderCustom::FPakFileRecord>& PakFileIn);
	virtual ~FDownloadChunk();

	inline bool HasCompleted() const { return bHasCompleted; }
	inline int32 GetProgress() const { return LastBytesReceived; }

	void Start();
	void Cancel(bool bResult);

public:
	const TSharedRef<FChunkDownloaderCustom> Downloader;
	const TSharedRef<FChunkDownloaderCustom::FPakFileRecord> PakFile;
	const FString TargetFile;

protected:
	void UpdateFileSize();
	bool ValidateFile() const;
	bool HasDeviceSpaceRequired() const;
	void StartDownload(int TryNumber);
	void OnDownloadProgress(int32 BytesReceived);
	void OnDownloadComplete(const FString& Url, int TryNumber, int32 HttpStatus);
	void OnCompleted(bool bSuccess, const FText& ErrorText);

private:
	bool bIsCancelled = false;
	FDownloadCancel CancelCallback;
	bool bHasCompleted = false;
	FDateTime BeginTime;
	int32 LastBytesReceived = 0;
};

#if UE_ENABLE_INCLUDE_ORDER_DEPRECATED_IN_5_2
#include "CoreMinimal.h"
#endif
