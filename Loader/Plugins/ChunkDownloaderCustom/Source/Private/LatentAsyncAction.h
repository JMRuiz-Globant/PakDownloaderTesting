
#pragma once

#include "LatentActions.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "LatentAsyncAction.generated.h"

// Simple delegate to use for the async action output pins.
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FLatentAsyncActionEvent);

/**
 * Simple implementation of an async action that also registers itself as a latent action to take advantage of their blueprint node UUID provider.
 * 
 * This could be done with a custom implementation of UK2Node_AsyncAction to generate the UUID in the same way as the "LatentInfo" metadata.
 * However, since the "LatentInfo" metadata seems to work fine even without providing the "Latent" metadata itself, we can just use it.
 * 
 * It's very likely that this is all doable with just a latent action, but it's not clear how to tweak the output exec pins.
 * 
 * All in all, this is a slightly hacky but functional combination of both methods, so there.
 */
UCLASS(Abstract, meta=(HideThen))
class ULatentAsyncAction : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	virtual void SetReadyToDestroy() override;

	/** Handle when this action is being destroyed to ensure that the action is canceled and child classes can clean up. */
	virtual void BeginDestroy() override;

	/** Cancel an asynchronous action, this attempts to cancel any lower level processes and also prevents delegates from being fired */
	UFUNCTION(BlueprintCallable, Category = "Async Action")
	virtual void Cancel();

	/** Returns true if this action is still active and has not completed or been cancelled */
	UFUNCTION(BlueprintCallable, Category = "Async Action")
	virtual bool IsActive() const;

	/** This should be called prior to broadcasting delegates back into the event graph, this ensures the action is still valid */
	virtual bool ShouldBroadcastDelegates() const;

	/** Returns true if this action is registered with a valid game instance */
	bool IsRegistered() const;

	/** Wrapper function to get a timer manager for scheduling callbacks */
	class FTimerManager* GetTimerManager() const;

	inline UObject* GetCallbacktarget() const { return CallbackTarget.Get(); }
	inline int32 GetUUID() const { return UUID; }

private:
	void RemoveLatentAction(UGameInstance* GameInstance);

	TWeakObjectPtr<UObject> CallbackTarget;
	int32 UUID{ INDEX_NONE };

	class FLatentAsyncAction : public FPendingLatentAction
	{
	private:
		friend class ULatentAsyncAction;
		TWeakObjectPtr<ULatentAsyncAction> AsyncAction;
	public:
		FLatentAsyncAction(ULatentAsyncAction* inAsyncAction) :AsyncAction(inAsyncAction) {}
		virtual void UpdateOperation(FLatentResponse& Response) override { Response.DoneIf(!AsyncAction.IsValid()); }
	};

protected:

	virtual void RegisterWithGameInstance(UGameInstance* GameInstance) override;

	// Factory function template to call from child classes.
	template<typename T>
	static typename TEnableIf<TIsDerivedFrom<T, ULatentAsyncAction>::Value, T*>::Type Create(UGameInstance* inGameInstance, UObject* inCallbackTarget, int32 inUUID, bool bForce = false)
	{
		if (!inGameInstance || !inCallbackTarget || inUUID == INDEX_NONE) { return nullptr; }

		auto LatentActionManager = inGameInstance->GetLatentActionManager();
		if (auto LatentAction = LatentActionManager.FindExistingAction<FLatentAsyncAction>(inCallbackTarget, inUUID))
		{
			if (auto OldAsyncAction = LatentAction->AsyncAction.Get()) 
			{
				if (!bForce) { return nullptr; }
				OldAsyncAction->SetReadyToDestroy();
			}
		}

		if (auto Result = NewObject<T>())
		{
			Result->CallbackTarget = inCallbackTarget;
			Result->UUID = inUUID;
			Result->RegisterWithGameInstance(inGameInstance);
			return Result;
		}

		return nullptr;
	}
};

#if UE_ENABLE_INCLUDE_ORDER_DEPRECATED_IN_5_2
#include "CoreMinimal.h"
#endif