// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "HAL/Platform.h"

class FString;
template <typename FuncType> class TFunction;

typedef TFunction<void(int32 HttpStatus)> FDownloadComplete;
typedef TFunction<void(int32 BytesReceived)> FDownloadProgress;
typedef TFunction<void(void)> FDownloadCancel;

extern FDownloadCancel PlatformStreamDownloadChunk(const FString& Url, const FString& TargetFile, const FDownloadProgress& Progress, const FDownloadComplete& Callback);

#if UE_ENABLE_INCLUDE_ORDER_DEPRECATED_IN_5_2
#include "CoreMinimal.h"
#endif