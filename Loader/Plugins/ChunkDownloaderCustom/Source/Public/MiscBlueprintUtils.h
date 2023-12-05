
#pragma once
#include "LatentAsyncAction.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MiscBlueprintUtils.generated.h"


class ULevel;

UENUM(BlueprintType)
enum class EValidity : uint8
{
	IsValid,
	IsNotValid
};


USTRUCT(BlueprintType)
struct FMulticastDelegateHandle
{
	GENERATED_BODY()
	FMulticastDelegateHandle() : Handle(FDelegateHandle()) {}
	FMulticastDelegateHandle(const FDelegateHandle& inHandle) : Handle(inHandle) {}
	FMulticastDelegateHandle(FDelegateHandle&& inHandle) : Handle(inHandle) {}

	operator FDelegateHandle&() { return Handle; }
	FMulticastDelegateHandle& operator=(const FDelegateHandle& inHandle)
	{
		if (&Handle != &inHandle) {
			Handle = inHandle;
		}
		return *this;
	}
	FMulticastDelegateHandle& operator=(FDelegateHandle&& inHandle)
	{
		if (&Handle != &inHandle) {
			Handle = inHandle;
		}
		return *this;
	}

	FDelegateHandle Handle;
};

UCLASS()
class UMiscBlueprintUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	/**
	* Trunk statement.
	* The bool return value will be set to the condition that received input last.
	*/
	UFUNCTION(BlueprintCallable, Category = "Flow Control", meta = (ExpandBoolAsExecs = "Condition", ReturnDisplayName = "Condition"))
	static bool Trunk(bool Condition) { return Condition; }

	// Return true if the interface is usable : non-null and not pending kill
	UFUNCTION(BlueprintPure, Category = "Utilities")
	static bool IsValidInterface(TSubclassOf<UInterface> Interface);

};

FORCEINLINE_DEBUGGABLE bool UMiscBlueprintUtils::IsValidInterface(TSubclassOf<UInterface> Interface)
{
	return ::IsValid(Interface);
}

UCLASS(Meta = (DisplayName = "Level Loader"))
class ULevelLoadSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	/** Implement this for initialization of instances of the system */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Implement this for deinitialization of instances of the system */
	virtual void Deinitialize() override;


	UFUNCTION(BlueprintPure, meta=(AsClass="/ChunkDownloaderCustom/BP_LevelLoadSubsystem.BP_LevelLoadSubsystem_C", InternalUseParam="AsClass", DeterminesOutputType ="AsClass", DynamicOutputParam ="ReturnValue", AutoCreateRefTerm="AsClass", CompactNodeTitle="->"))
	UObject* Get(const TSubclassOf<UObject>& AsClass) { return BPObject; }
private:
	
	UPROPERTY()
	TObjectPtr<UObject> BPObject;

};

DECLARE_DYNAMIC_DELEGATE_OneParam(FPostLoadWorld, UWorld*, LoadedWorld);

UCLASS()
class ULevelLoadUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	/**
	 *	Get all Actors in the specified ULevel.
	 *	@param	OutActors	Output array of Actors.
	 */
	UFUNCTION(BlueprintPure, Category = "Actor")
	static void GetAllActorsInLevel(ULevel* Level,TArray<AActor*>& OutActors);
	
	/**
	 *	Get all Actors in the specified ULevelStreaming.
	 *	@param	OutActors	Output array of Actors.
	 */
	UFUNCTION(BlueprintPure, Category = "Actor")
	static void GetAllActorsInStreamingLevel(ULevelStreaming* Level, TArray<AActor*>& OutActors);

	/**
	 *	Find all Actors with the specified tag in the provided array.
	 *	This is a slow operation, use with caution e.g. do not use every frame.
	 *	@param	Tag			Tag to find. Must be specified or result array will be empty.
	 *	@param	OutActors	Output array of Actors with the specified tag.
	 */
	UFUNCTION(BlueprintCallable, Category = "Actor", meta=(DeterminesOutputType="InActors", DynamicOutputParam="OutActors"))
	static void FilterActorsByTag(const TArray<AActor*>& InActors, FName Tag, TArray<AActor*>& OutActors);

	/**
	 *	Find the first actor with the specified tag in the provided array.
	 *	This is a slow operation, use with caution e.g. do not use every frame.
	 *	@param	Tag			Tag to find. Must be specified or result array will be empty.
	 *	@return				Actor with the specified tag.
	 */
	UFUNCTION(BlueprintCallable, Category = "Actor", meta = (ExpandEnumAsExecs = "ReturnValue", DeterminesOutputType="InActors", DynamicOutputParam="OutActor"))
	static EValidity FindActorWithTag(const TArray<AActor*>& InActors, FName Tag, AActor*& OutActor);

	/**
	 *	Find all Actors of the specified class, optionally with the specified tag, in the provided array.
	 *	This is a slow operation, use with caution e.g. do not use every frame.
	 *	@param	bByTag		If true, also filter by tag. Must be specified or result array will be empty.
	 *	@param	Tag			Tag to find, if filter by tag is enabled. Must be specified or result array will be empty.
	 *	@param	ActorClass	Class of Actor to find. Must be specified or result array will be empty.
	 *	@param	OutActors	Output array of Actors of the specified tag.
	 */
	UFUNCTION(BlueprintCallable, Category = "Actor", meta = (DeterminesOutputType="ActorClass", DynamicOutputParam="OutActors", AdvancedDisplay="bByTag,Tag"))
	static void FilterActorsByClass(const TArray<AActor*>& InActors, TSubclassOf<AActor> ActorClass, bool bByTag, FName Tag, TArray<AActor*>& OutActors);

	/**
	 *	Find the first actor of the specified class, optionally with the specified tag, in the provided array.
	 *	This is a slow operation, use with caution e.g. do not use every frame.
	 *	@param	bByTag		If true, also filter by tag. Must be specified or result array will be empty.
	 *	@param	Tag			Tag to find, if filter by tag is enabled. Must be specified or result array will be empty.
	 *	@param	ActorClass	Class of Actor to find. Must be specified or result array will be empty.
	 *	@return				Actor of the specified class with the specified tag.
	 */
	UFUNCTION(BlueprintCallable, Category = "Actor", meta = (ExpandEnumAsExecs = "ReturnValue", DeterminesOutputType="ActorClass", DynamicOutputParam="OutActor", AdvancedDisplay = "bByTag,Tag"))
	static EValidity FindActorOfClass(const TArray<AActor*>& InActors, TSubclassOf<AActor> ActorClass, bool bByTag, FName Tag, AActor*& OutActor);

	/**
	 *	Find all Actors with the specified interface, optionally with the specified tag, in the provided array.
	 *	This is a slow operation, use with caution e.g. do not use every frame.
	 *	@param	bByTag		If true, also filter by tag. Must be specified or result array will be empty.
	 *	@param	Tag			Tag to find, if filter by tag is enabled. Must be specified or result array will be empty.
	 *	@param	Interface	Interface to find. Must be specified or result array will be empty.
	 *	@param	OutActors	Output array of Actors of the specified interface.
	 */
	UFUNCTION(BlueprintCallable, Category = "Actor", meta = (DeterminesOutputType = "Interface", DynamicOutputParam = "OutActors", AdvancedDisplay = "bByTag,Tag"))
	static void FilterActorsByInterface(const TArray<AActor*>& InActors, TSubclassOf<UInterface> Interface, bool bByTag, FName Tag, TArray<AActor*>& OutActors);

	/**
	 *	Find the first actor with the specified interface,, optionally with the specified tag in the provided array.
	 *	This is a slow operation, use with caution e.g. do not use every frame.
	 *	@param	bByTag		If true, also filter by tag. Must be specified or result array will be empty.
	 *	@param	Tag			Tag to find, if filter by tag is enabled. Must be specified or result array will be empty.
	 *	@param	Interface	Interface to find. Must be specified or result array will be empty.
	 *	@return				Actor with the specified interface.
	 */
	UFUNCTION(BlueprintCallable, Category = "Actor", meta = (ExpandEnumAsExecs = "ReturnValue", DeterminesOutputType = "Interface", DynamicOutputParam = "OutActor", AdvancedDisplay = "bByTag,Tag"))
	static EValidity FindActorWithInterface(const TArray<AActor*>& InActors, TSubclassOf<UInterface> Interface, bool bByTag, FName Tag, AActor*& OutActor);

	UFUNCTION(BlueprintCallable, Category="Utilities|World", meta=(DisplayName="RepairWorldSettings"))
	static void RepairUWorldSettings(UWorld* World);


	UFUNCTION(BlueprintCallable, Category="Utilities|World", meta=(DisplayName="Bind Level Load (by Name)", AutoCreateRefTerm="Handle", AdvancedDisplay="LevelName,bSingle,Handle", bSingle="true"))
	static void BindPostLoadMapWithWorld(const FPostLoadWorld& Event, FName LevelName, bool bSingle, UPARAM(ref) FMulticastDelegateHandle& Handle);

	UFUNCTION(BlueprintCallable, Category="Utilities|World", meta=(DisplayName="Bind Level Load (by Object Reference)", AutoCreateRefTerm="Handle", AdvancedDisplay="Level,bSingle,Handle", bSingle="true"))
	static void BindPostLoadMapWithWorldBySoftObjectPtr(const FPostLoadWorld& Event, TSoftObjectPtr<UWorld> Level, bool bSingle, UPARAM(ref) FMulticastDelegateHandle& Handle);

	UFUNCTION(BlueprintCallable, Category="Utilities|World", meta=(DisplayName="Unbind PostLoadMapWithWorld"))
	static bool UnbindPostLoadMapWithWorld(UPARAM(ref) FMulticastDelegateHandle& Handle);
	
	// Create a delegate bound to a static function.
	// The reference must be a valid static UFunction. For example, this function would be "/Script/ChunkDownloaderCustom.LevelLoadUtils.CreateStaticDelegate". 
	// @param FunctionName	The reference to a static UFunction, in the form "/root/module.class.functionName".
	UFUNCTION(BlueprintPure, CustomThunk, Category="Utilities", meta=(DisplayName="Create Static Event", CustomStructureParam="Event"))
	static void CreateStaticDelegate(FName FunctionRef, int32& Event);
	DECLARE_FUNCTION(execCreateStaticDelegate)
	{
		P_GET_PROPERTY(FNameProperty, FunctionRef);
		P_GET_PROPERTY_REF(FDelegateProperty, Event);
		FDelegateProperty* EventProp = CastField<FDelegateProperty>(Stack.MostRecentProperty);

		if (!EventProp)
		{
			UE_LOG(LogTemp, Error, TEXT("CreateStaticDelegate -> Incompatible type linked to event pin!"));
			return;
		}
 
 		P_FINISH;
		P_NATIVE_BEGIN;

		// If the reference name was input correctly, it should be of the form "/<RootDir>/<ModuleName>.<ClassName>.<MemberName>".
		// Splitting the name by the last dot should give us the value needed for the class' FSoftClassPath and also the function name.
		FString ClassName, FunctionName;
		FunctionRef.ToString().Split(TEXT("."), &ClassName, &FunctionName, ESearchCase::IgnoreCase, ESearchDir::FromEnd);

		if (UClass* Class = FSoftClassPath(ClassName).TryLoadClass<UObject>())
		{
			// Found class!

			if (UFunction* Function = Class->FindFunctionByName(FName(FunctionName)))
			{
				if (!Function->IsSignatureCompatibleWith(EventProp->SignatureFunction))
				{
					UE_LOG(LogTemp, Error, TEXT("CreateStaticDelegate -> Found function \"%s\" in class \"%s\" is incompatible with delegate signature!"), *FunctionName, *Class->GetFName().ToString());
				}
				else if (!Function->HasAllFunctionFlags(FUNC_Static))
				{
					UE_LOG(LogTemp, Error, TEXT("CreateStaticDelegate -> Found function \"%s\" in class \"%s\" is not static! Use regular Create Event node."), *FunctionName, *Class->GetFName().ToString());

				}
				else if (Function->HasAllFunctionFlags(FUNC_BlueprintPure))
				{
					UE_LOG(LogTemp, Error, TEXT("CreateStaticDelegate -> Found function \"%s\" in class \"%s\" is pure! It can't be executed as a delegate."), *FunctionName, *Class->GetFName().ToString());
				}
				else {
					// Found func!!
					Event.BindUFunction(Class->GetDefaultObject(), Function->GetFName());
				}
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("CreateStaticDelegate -> Couldn't find a function named \"%s\" in class \"%s\"!"), *FunctionName, *Class->GetFName().ToString());
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("CreateStaticDelegate -> Couldn't find a valid class! Reference path parses as \"%s\""), *ClassName);
		}
		P_NATIVE_END;
	}


};


UCLASS(meta=(DisplayName="Actor"))
class UActorOverlapAction : public ULatentAsyncAction
{
	GENERATED_BODY()

public:
	
	// Wait until there's a collision overlap between the given target and a triggering actor. Fires only once.
	// @param Target			The actor that will have its overlaps checked (IsOverlappingActor()), Equivalent to "Overlapping Actor" on the OnActorBeginOverlap event.
	// @param TriggeringActor	The actor that will be queried in the Target's IsOverlappingActor() call. Equivalent to "Other Actor" on the OnActorBeginOverlap event.
	// @param bInvalidate		A boolean reference that can be used as a handle to abort the wait before it completes by setting it to true. It is automatically set to false when the node is called, and to true when the node completes. This means that if a handle is shared between multiple nodes, the first one that completes will invalidate all the rest.
	UFUNCTION(BlueprintCallable, Category = "Actor|Latent", meta=(BlueprintInternalUseOnly="true", LatentInfo = "LatentInfo", DefaultToSelf = "Target", 
		AutoCreateRefTerm="bInvalidate", 
		AdvancedDisplay="bInvalidate"))
    static UActorOverlapAction* WaitForOverlap(FLatentActionInfo LatentInfo, 
		AActor* Target, 
		AActor* TriggeringActor,
		UPARAM(ref) bool& bInvalidate
	);

	// Called to trigger the action once the delegates have been bound.
	virtual void Activate() override;

	// Called to execute a delegate and cancel the action.
	virtual void Complete(bool Success);

    // Overlap happened.
    UPROPERTY(BlueprintAssignable, meta=(DisplayName="Completed"))
	FLatentAsyncActionEvent OnCompleted;

private:
	FTimerDelegate Ticker;
	TWeakObjectPtr<AActor> Target;
	TWeakObjectPtr<AActor> TriggeringActor;
	bool* bInvalidate;
};


UCLASS(meta=(DisplayName="Actor"))
class UActorOverlapPlayerAction : public ULatentAsyncAction
{
	GENERATED_BODY()

public:
	// Wait until there's a collision overlap between the given target and the player pawn. Fires only once.
	// @param Target			The actor that will have its overlaps checked (IsOverlappingActor()), Equivalent to "Overlapping Actor" on the OnActorBeginOverlap event.
	// @param bInvalidate		A boolean reference that can be used as a handle to abort the wait before it completes by setting it to true. It is automatically set to false when the node is called, and to true when the node completes. This means that if a handle is shared between multiple nodes, the first one that completes will invalidate all the rest.
	UFUNCTION(BlueprintCallable, Category = "Actor|Latent", meta=(BlueprintInternalUseOnly="true", LatentInfo = "LatentInfo", DefaultToSelf = "Target", AutoCreateRefTerm="bInvalidate", AdvancedDisplay="bInvalidate"))
    static UActorOverlapPlayerAction* WaitForPlayerOverlap(FLatentActionInfo LatentInfo, AActor* Target, UPARAM(ref) bool& bInvalidate);

	// Called to trigger the action once the delegates have been bound.
	virtual void Activate() override;

	// Called to execute a delegate and cancel the action.
	virtual void Complete(bool Success);

    // Overlap happened.
    UPROPERTY(BlueprintAssignable, meta=(DisplayName="Completed"))
	FLatentAsyncActionEvent OnCompleted;

private:
	FTimerDelegate Ticker;
	TWeakObjectPtr<AActor> Target;
	TWeakObjectPtr<AActor> TriggeringActor;
	bool* bInvalidate;
};



UCLASS(meta=(DisplayName="Actor"))
class UActorOverlapsAction : public ULatentAsyncAction
{
	GENERATED_BODY()

public:
	
	// Wait until there's a collision overlap between the given target and a triggering actor. Fires repeatedly, needs explicit invalidation.
	// @param Target			The actor that will have its overlaps checked (IsOverlappingActor()), Equivalent to "Overlapping Actor" on the OnActorBeginOverlap event.
	// @param TriggeringActor	The actor that will be queried in the Target's IsOverlappingActor() call. Equivalent to "Other Actor" on the OnActorBeginOverlap event.
	// @param bInvalidate		A boolean reference that can be used as a handle to abort the wait before it completes by setting it to true. It is automatically set to false when the node is called.
	UFUNCTION(BlueprintCallable, Category = "Actor|Latent", meta=(BlueprintInternalUseOnly="true", LatentInfo = "LatentInfo", DefaultToSelf = "Target", 
		AutoCreateRefTerm="bInvalidate", 
		AdvancedDisplay="bInvalidate"))
    static UActorOverlapsAction* WaitForOverlaps(FLatentActionInfo LatentInfo, 
		AActor* Target, 
		AActor* TriggeringActor,
		UPARAM(ref) bool& bInvalidate
	);

	// Called to trigger the action once the delegates have been bound.
	virtual void Activate() override;

    // Overlap happened.
    UPROPERTY(BlueprintAssignable, meta=(DisplayName="Begin"))
	FLatentAsyncActionEvent OnBeginOverlap;
	//Overlap keeps happening.
	UPROPERTY(BlueprintAssignable, meta=(DisplayName="Update"))
	FLatentAsyncActionEvent OnOverlapUpdate;
	// Overlap ceased to happen.
	UPROPERTY(BlueprintAssignable, meta=(DisplayName="End"))
	FLatentAsyncActionEvent OnEndOverlap;

private:
	FTimerDelegate Ticker;
	TWeakObjectPtr<AActor> Target;
	TWeakObjectPtr<AActor> TriggeringActor;
	bool bIsOverlapping;
	bool* bInvalidate;
};


UCLASS(meta=(DisplayName="Actor"))
class UActorOverlapsPlayerAction : public ULatentAsyncAction
{
	GENERATED_BODY()

public:
	// Wait until there's a collision overlap between the given target and the player pawn. Fires repeatedly, needs explicit invalidation.
	// @param Target			The actor that will have its overlaps checked (IsOverlappingActor()), Equivalent to "Overlapping Actor" on the OnActorBeginOverlap event.
	// @param bInvalidate		A boolean reference that can be used as a handle to abort the wait before it completes by setting it to true. It is automatically set to false when the node is called.
	UFUNCTION(BlueprintCallable, Category = "Actor|Latent", meta=(BlueprintInternalUseOnly="true", LatentInfo = "LatentInfo", DefaultToSelf = "Target", AutoCreateRefTerm="bInvalidate", AdvancedDisplay="bInvalidate"))
    static UActorOverlapsPlayerAction* WaitForPlayerOverlaps(FLatentActionInfo LatentInfo, AActor* Target, UPARAM(ref) bool& bInvalidate);

	// Called to trigger the action once the delegates have been bound.
	virtual void Activate() override;

    // Overlap happened.
    UPROPERTY(BlueprintAssignable, meta=(DisplayName="Begin"))
	FLatentAsyncActionEvent OnBeginOverlap;
	//Overlap keeps happening.
	UPROPERTY(BlueprintAssignable, meta=(DisplayName="Update"))
	FLatentAsyncActionEvent OnOverlapUpdate;
	// Overlap ceased to happen.
	UPROPERTY(BlueprintAssignable, meta=(DisplayName="End"))
	FLatentAsyncActionEvent OnEndOverlap;

private:
	FTimerDelegate Ticker;
	TWeakObjectPtr<AActor> Target;
	TWeakObjectPtr<AActor> TriggeringActor;
	bool bIsOverlapping;
	bool* bInvalidate;
};

#if UE_ENABLE_INCLUDE_ORDER_DEPRECATED_IN_5_2
#include "CoreMinimal.h"
#endif
