// Fill out your copyright notice in the Description page of Project Settings.

#include "LatentAsyncAction.h"

void ULatentAsyncAction::SetReadyToDestroy()
{
	ClearFlags(RF_StrongRefOnFrame);

	UGameInstance* OldGameInstance = RegisteredWithGameInstance.Get();
	if (OldGameInstance)
	{
		OldGameInstance->UnregisterReferencedObject(this);
		RemoveLatentAction(OldGameInstance);
	}
}

void ULatentAsyncAction::BeginDestroy()
{
	Cancel();
	Super::BeginDestroy();
}

void ULatentAsyncAction::Cancel()
{
	// Child classes should override this
	SetReadyToDestroy();
}

bool ULatentAsyncAction::IsActive() const
{
	return ShouldBroadcastDelegates();
}

bool ULatentAsyncAction::ShouldBroadcastDelegates() const
{
	return IsRegistered();
}

bool ULatentAsyncAction::IsRegistered() const
{
	return RegisteredWithGameInstance.IsValid();
}

class FTimerManager* ULatentAsyncAction::GetTimerManager() const
{
	if (RegisteredWithGameInstance.IsValid())
	{
		return &RegisteredWithGameInstance->GetTimerManager();
	}

	return nullptr;
}

void ULatentAsyncAction::RemoveLatentAction(UGameInstance* GameInstance)
{
	if (auto LatentAction = GameInstance->GetLatentActionManager().FindExistingAction<FLatentAsyncAction>(CallbackTarget.Get(), UUID))
	{
		LatentAction->AsyncAction.Reset();
	}
}

void ULatentAsyncAction::RegisterWithGameInstance(UGameInstance* GameInstance)
{
	if (GameInstance)
	{
		UGameInstance* OldGameInstance = RegisteredWithGameInstance.Get();
		if (OldGameInstance)
		{
			OldGameInstance->UnregisterReferencedObject(this);
			RemoveLatentAction(OldGameInstance);
		}
		GameInstance->GetLatentActionManager().AddNewAction(CallbackTarget.Get(), UUID, new FLatentAsyncAction(this));
		GameInstance->RegisterReferencedObject(this);
		RegisteredWithGameInstance = GameInstance;
	}
}