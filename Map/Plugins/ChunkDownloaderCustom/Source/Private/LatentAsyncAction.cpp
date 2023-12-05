// Fill out your copyright notice in the Description page of Project Settings.

#include "LatentAsyncAction.h"

void ULatentAsyncAction::SetReadyToDestroy()
{
	ClearFlags(RF_StrongRefOnFrame);
	
	RemoveLatentAction();

	if (UGameInstance* OldGameInstance = RegisteredWithGameInstance.Get())
	{
		OldGameInstance->UnregisterReferencedObject(this);
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

class FTimerManager* ULatentAsyncAction::GetTimerManager() const
{	
	return RegisteredWithGameInstance.IsValid() ? &RegisteredWithGameInstance->GetTimerManager() : nullptr;
}

void ULatentAsyncAction::RemoveLatentAction()
{
	if (Proxy) 
	{
		Proxy->AsyncAction.Reset();
		Proxy = nullptr;
	}
}

void ULatentAsyncAction::OnLatentActionDeleted(FLatentAsyncActionProxy* LatentAction)
{
	if (LatentAction == Proxy) { SetReadyToDestroy(); }
}

void ULatentAsyncAction::RegisterWithGameInstance(UGameInstance* GameInstance)
{
	if (GameInstance)
	{
		RemoveLatentAction();

		if (UGameInstance* OldGameInstance = RegisteredWithGameInstance.Get())
		{
			OldGameInstance->UnregisterReferencedObject(this);
		}

		Proxy = new FLatentAsyncActionProxy(this);
		if (Proxy) {
			GameInstance->GetLatentActionManager().AddNewAction(CallbackTarget.Get(), UUID, Proxy);
			GameInstance->RegisterReferencedObject(this);
			RegisteredWithGameInstance = GameInstance;
		}
	}
}