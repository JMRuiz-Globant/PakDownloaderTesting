
#include "MiscBlueprintUtils.h"
#include "Engine/LevelStreaming.h"
#include "Kismet/GameplayStatics.h"





void ULevelLoadSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	UClass* BPClass = StaticLoadClass(UObject::StaticClass(), NULL, TEXT("/ChunkDownloaderCustom/BP_LevelLoadSubsystem.BP_LevelLoadSubsystem_C"), NULL, LOAD_None, NULL);
	if (BPClass) 
	{
		BPObject = NewObject<UObject>(this, BPClass);
		if (BPObject)
		{
			UE_LOG(LogTemp, Log, TEXT("LevelLoadSubsystem wraps its BP object!"));

			if (UFunction* BPInitialize = BPObject->FindFunction(TEXT("Initialize")))
			{
				if (BPInitialize->ParmsSize == 0)
				{
					BPObject->ProcessEvent(BPInitialize, nullptr);
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("LevelLoadSubsystem BP object's \"Initialize\" function can't have parameters nor return value!"));
				}
			}
			return;
		}
	}

	UE_LOG(LogTemp, Error, TEXT("LevelLoadSubsystem couldn't spawn BP object!"));
}

void ULevelLoadSubsystem::Deinitialize()
{
	if (BPObject)
	{
		if (UFunction* BPDeinitialize = BPObject->FindFunction(TEXT("Deinitialize")))
		{
			if (BPDeinitialize->ParmsSize == 0)
			{
				BPObject->ProcessEvent(BPDeinitialize, nullptr);
			}
			else 
			{
				UE_LOG(LogTemp, Error, TEXT("LevelLoadSubsystem BP object's \"Deinitialize\" function can't have parameters nor return value!"));
			}
		}

		BPObject->ConditionalBeginDestroy();
		BPObject = nullptr;
	}
}

void ULevelLoadUtils::GetAllActorsInLevel(ULevel* Level, TArray<AActor*>& OutActors)
{
	OutActors.Reset();
	if (IsValid(Level))
	{
		OutActors = Level->Actors;
	}
}

void ULevelLoadUtils::GetAllActorsInStreamingLevel(ULevelStreaming* Level, TArray<AActor*>& OutActors)
{
	OutActors.Reset();
	if (IsValid(Level) && IsValid(Level->GetLoadedLevel()))
	{
		OutActors = Level->GetLoadedLevel()->Actors;
	}
}

void ULevelLoadUtils::FilterActorsByTag(const TArray<AActor*>& InActors, FName Tag, TArray<AActor*>& OutActors)
{
	OutActors.Reset();
	if (!Tag.IsNone())
	{
		for (AActor* Actor : InActors)
		{
			if (IsValid(Actor) && Actor->ActorHasTag(Tag))
			{
				OutActors.Add(Actor);
			}
		}
	}
}

EValidity ULevelLoadUtils::FindActorWithTag(const TArray<AActor*>& InActors, FName Tag, AActor*& OutActor)
{	
	OutActor = nullptr;
	if (!Tag.IsNone())
	{
		for (AActor* Actor : InActors)
		{
			if (IsValid(Actor) && Actor->ActorHasTag(Tag))
			{
				OutActor = Actor;
				return EValidity::IsValid;
			}
		}
	}
	return EValidity::IsNotValid;
}

void ULevelLoadUtils::FilterActorsByClass(const TArray<AActor*>& InActors, TSubclassOf<AActor> ActorClass, bool bByTag, FName Tag, TArray<AActor*>& OutActors)
{
	OutActors.Reset();
	if (ActorClass && (!bByTag || !Tag.IsNone()))
	{
		for (AActor* Actor : InActors)
		{
			if (IsValid(Actor) && Actor->IsA(ActorClass) && (!bByTag || Actor->ActorHasTag(Tag)))
			{
				OutActors.Add(Actor);
			}
		}
	}
}

EValidity ULevelLoadUtils::FindActorOfClass(const TArray<AActor*>& InActors, TSubclassOf<AActor> ActorClass, bool bByTag, FName Tag, AActor*& OutActor)
{
	OutActor = nullptr;
	if (ActorClass && (!bByTag || !Tag.IsNone()))
	{
		for (AActor* Actor : InActors)
		{
			if (IsValid(Actor) && Actor->IsA(ActorClass) && (!bByTag || Actor->ActorHasTag(Tag)))
			{
				OutActor = Actor;
				return EValidity::IsValid;
			}
		}
	}
	return EValidity::IsNotValid;
}

void ULevelLoadUtils::FilterActorsByInterface(const TArray<AActor*>& InActors, TSubclassOf<UInterface> Interface, bool bByTag, FName Tag, TArray<AActor*>& OutActors)
{
	OutActors.Reset();
	if (Interface && (!bByTag || !Tag.IsNone()))
	{
		for (AActor* Actor : InActors)
		{
			if (IsValid(Actor) && Actor->GetClass()->ImplementsInterface(Interface) && (!bByTag || Actor->ActorHasTag(Tag)))
			{
				OutActors.Add(Actor);
			}
		}
	}
}


EValidity ULevelLoadUtils::FindActorWithInterface(const TArray<AActor*>& InActors, TSubclassOf<UInterface> Interface, bool bByTag, FName Tag, AActor*& OutActor)
{
	OutActor = nullptr;
	if (Interface && (!bByTag || !Tag.IsNone()))
	{
		for (AActor* Actor : InActors)
		{
			if (IsValid(Actor) && Actor->GetClass()->ImplementsInterface(Interface) && (!bByTag || Actor->ActorHasTag(Tag)))
			{
				OutActor = Actor;
				return EValidity::IsValid;
			}
		}
	}
	return EValidity::IsNotValid;
}


void ULevelLoadUtils::RepairUWorldSettings(UWorld* World)
{
	// Copy from private function UWorld::RepairWorldSettings
	if (World)
	{
		ULevel* PersistentLevel = World->PersistentLevel;
		AWorldSettings* ExistingWorldSettings = PersistentLevel->GetWorldSettings(false);

		if (ExistingWorldSettings == nullptr && PersistentLevel->Actors.Num() > 0)
		{
			ExistingWorldSettings = Cast<AWorldSettings>(PersistentLevel->Actors[0]);
			if (ExistingWorldSettings)
			{
				// This means the WorldSettings member just wasn't initialized, get that resolved
				PersistentLevel->SetWorldSettings(ExistingWorldSettings);
			}
		}

		// If for some reason we don't have a valid WorldSettings object go ahead and spawn one to avoid crashing.
		// This will generally happen if a map is being moved from a different project.
		if (ExistingWorldSettings == nullptr || ExistingWorldSettings->GetClass() != GEngine->WorldSettingsClass)
		{
			// Rename invalid WorldSettings to avoid name collisions
			if (ExistingWorldSettings)
			{
				ExistingWorldSettings->Rename(nullptr, PersistentLevel, REN_ForceNoResetLoaders);
			}

			bool bClearOwningWorld = false;

			if (PersistentLevel->OwningWorld == nullptr)
			{
				bClearOwningWorld = true;
				PersistentLevel->OwningWorld = World;
			}

			FActorSpawnParameters SpawnInfo;
			SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			SpawnInfo.Name = GEngine->WorldSettingsClass->GetFName();
			AWorldSettings* const NewWorldSettings = World->SpawnActor<AWorldSettings>(GEngine->WorldSettingsClass, SpawnInfo);

			// If there was an existing actor, copy its properties to the new actor (the it will be destroyed by SetWorldSettings)
			if (ExistingWorldSettings)
			{
				NewWorldSettings->UnregisterAllComponents();
				UEngine::FCopyPropertiesForUnrelatedObjectsParams CopyParams;
				CopyParams.bNotifyObjectReplacement = true;
				UEngine::CopyPropertiesForUnrelatedObjects(ExistingWorldSettings, NewWorldSettings, CopyParams);
				NewWorldSettings->RegisterAllComponents();
			}

			PersistentLevel->SetWorldSettings(NewWorldSettings);

			// Re-sort actor list as we just shuffled things around.
			PersistentLevel->SortActorList();

			if (bClearOwningWorld)
			{
				PersistentLevel->OwningWorld = nullptr;
			}
		}

		// Now that we have set the proper world settings, clean up any other stay that may have accumulated due to legacy behaviors
		if (PersistentLevel->Actors.Num() > 1)
		{
			for (int32 Index = 1, ActorNum = PersistentLevel->Actors.Num(); Index < ActorNum; ++Index)
			{
				AActor* Actor = PersistentLevel->Actors[Index];
				if (Actor != nullptr && Actor->IsA<AWorldSettings>())
				{
					Actor->Destroy();
				}
			}
		}

		check(World->GetWorldSettings());
	}
}

void ULevelLoadUtils::BindPostLoadMapWithWorld(const FPostLoadWorld& ConstEvent, FName LevelName, bool bSingle, FMulticastDelegateHandle& Handle)
{
	// event is const just to take advantage of BP not allowing ref terms without pins, so this is fine.	
	FPostLoadWorld Event = ConstEvent;
	FString Map;

	// If the level name isn't explicitly empty, make sure it points to a valid map.
	if (!LevelName.IsNone())
	{	
		FURL TestURL(nullptr, *LevelName.ToString(), ETravelType::TRAVEL_Absolute);
		if (!GEngine->MakeSureMapNameIsValid(TestURL.Map))
		{
			UE_LOG(LogLevel, Warning, TEXT("WARNING: The map '%s' does not exist."), *TestURL.Map);
			return;
		}
		Map = TestURL.Map;
	}

	// If we want to tweak the event's behavior, we will wrap it with a lambda.
	if (!Map.IsEmpty() || bSingle)
	{
		// Shared pointer to the handle. It will be shared with the lambda function and allow it to unbind itself even if the handle wasn't saved by the user.
		TSharedPtr<FDelegateHandle> SharedHandle = MakeShareable(new FDelegateHandle());
		*SharedHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddLambda([Event, Map, bSingle, Handle = SharedHandle](TSoftObjectPtr<UWorld> LoadedLevel)
			{
				if (Map.IsEmpty())
				{
					Event.ExecuteIfBound(LoadedLevel.Get());
				}
				else
				{
					FURL TestURL(nullptr, *FPackageName::ObjectPathToPackageName(LoadedLevel.ToString()), ETravelType::TRAVEL_Absolute);
					verify(!GEngine->MakeSureMapNameIsValid(TestURL.Map));
					const FString TestMapName = UWorld::RemovePIEPrefix(TestURL.Map);
					if (Map == TestMapName)
					{
						Event.ExecuteIfBound(LoadedLevel.Get());
					}
				}
				if (bSingle && Handle.IsValid())
				{
					FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(*Handle);
				}
			});

		// The handle can be copied without worry outside the shared pointer. If it's called by the user, the pointer will be deallocated when the delegate is removed from its array.
		Handle = *SharedHandle;
	}
	else
	{
		Handle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddUFunction(Event.GetUObject(), Event.GetFunctionName());
	}
}

void ULevelLoadUtils::BindPostLoadMapWithWorldBySoftObjectPtr(const FPostLoadWorld& ConstEvent, TSoftObjectPtr<UWorld> Level, bool bSingle, FMulticastDelegateHandle& Handle)
{
	const FName LevelName = FName(*FPackageName::ObjectPathToPackageName(Level.ToString()));
	ULevelLoadUtils::BindPostLoadMapWithWorld(ConstEvent, LevelName, bSingle, Handle);
}

bool ULevelLoadUtils::UnbindPostLoadMapWithWorld(FMulticastDelegateHandle& Handle)
{
	return FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(Handle);
}

UActorOverlapAction* UActorOverlapAction::WaitForOverlap(FLatentActionInfo LatentInfo, AActor* Target, AActor* TriggeringActor, bool& bInvalidate)
{
	if (Target)
	{
		if (auto Result = ULatentAsyncAction::Create<UActorOverlapAction>(Target->GetGameInstance(), LatentInfo.CallbackTarget, LatentInfo.UUID))
		{
			Result->Target = Target;
			Result->TriggeringActor = TriggeringActor;
			Result->bInvalidate = &bInvalidate;
			*Result->bInvalidate = false;
			return Result;
		}
	}
	return nullptr;
}

void UActorOverlapAction::Activate()
{
	Ticker = FTimerDelegate::CreateWeakLambda(this, [this]() {
		if (!*bInvalidate && Target.IsValid() && TriggeringActor.IsValid())
		{
			if (Target->IsOverlappingActor(TriggeringActor.Get()))
			{
				Complete(true);
				return;
			}
			GetTimerManager()->SetTimerForNextTick(Ticker); 
			return;
		}
		Cancel();
	});

	Ticker.ExecuteIfBound();
}



void UActorOverlapAction::Complete(bool Success)
{
	if (Success)
	{
		*bInvalidate = true;
		OnCompleted.Broadcast();
	}
	Cancel();
}

UActorOverlapPlayerAction* UActorOverlapPlayerAction::WaitForPlayerOverlap(FLatentActionInfo LatentInfo, AActor* Target, bool& bInvalidate)
{
	if (Target)
	{
		if (auto Result = ULatentAsyncAction::Create<UActorOverlapPlayerAction>(Target->GetGameInstance(), LatentInfo.CallbackTarget, LatentInfo.UUID))
		{
			Result->Target = Target;
			Result->TriggeringActor = UGameplayStatics::GetPlayerPawn(Target, 0);
			Result->bInvalidate = &bInvalidate;
			*Result->bInvalidate = false;
			return Result;
		}
	}
	return nullptr;
}

void UActorOverlapPlayerAction::Activate()
{
	Ticker = FTimerDelegate::CreateWeakLambda(this, [this]() {
		if (!*bInvalidate && Target.IsValid() && TriggeringActor.IsValid())
		{
			if (Target->IsOverlappingActor(TriggeringActor.Get()))
			{
				Complete(true);
				return;
			}
			GetTimerManager()->SetTimerForNextTick(Ticker);
			return;
		}
		Cancel();
		});

	Ticker.ExecuteIfBound();
}



void UActorOverlapPlayerAction::Complete(bool Success)
{
	if (Success)
	{
		*bInvalidate = true;
		OnCompleted.Broadcast();
	}
	Cancel();
}


UActorOverlapsAction* UActorOverlapsAction::WaitForOverlaps(FLatentActionInfo LatentInfo, AActor* Target, AActor* TriggeringActor, bool& bInvalidate)
{
	if (Target)
	{
		if (auto Result = ULatentAsyncAction::Create<UActorOverlapsAction>(Target->GetGameInstance(), LatentInfo.CallbackTarget, LatentInfo.UUID))
		{
			Result->Target = Target;
			Result->TriggeringActor = TriggeringActor;
			Result->bIsOverlapping = false;
			Result->bInvalidate = &bInvalidate;
			*Result->bInvalidate = false;
			return Result;
		}
	}
	return nullptr;
}

void UActorOverlapsAction::Activate()
{
	Ticker = FTimerDelegate::CreateWeakLambda(this, [this]() {
		if (!*bInvalidate && Target.IsValid() && TriggeringActor.IsValid())
		{
			if (bIsOverlapping != Target->IsOverlappingActor(TriggeringActor.Get()))
			{
				bIsOverlapping ^= true;
				(bIsOverlapping ? OnBeginOverlap : OnEndOverlap).Broadcast();
			}
			else if (bIsOverlapping)
			{
				OnOverlapUpdate.Broadcast();
			}
			GetTimerManager()->SetTimerForNextTick(Ticker);
			return;
		}
		Cancel();
		});

	Ticker.ExecuteIfBound();
}

UActorOverlapsPlayerAction* UActorOverlapsPlayerAction::WaitForPlayerOverlaps(FLatentActionInfo LatentInfo, AActor* Target, bool& bInvalidate)
{
	if (Target)
	{
		if (auto Result = ULatentAsyncAction::Create<UActorOverlapsPlayerAction>(Target->GetGameInstance(), LatentInfo.CallbackTarget, LatentInfo.UUID))
		{
			Result->Target = Target;
			Result->TriggeringActor = UGameplayStatics::GetPlayerPawn(Target, 0);
			Result->bIsOverlapping = false;
			Result->bInvalidate = &bInvalidate;
			*Result->bInvalidate = false;
			return Result;
		}
	}
	return nullptr;
}

void UActorOverlapsPlayerAction::Activate()
{
	Ticker = FTimerDelegate::CreateWeakLambda(this, [this]() {
		if (!*bInvalidate && Target.IsValid() && TriggeringActor.IsValid())
		{
			if (bIsOverlapping != Target->IsOverlappingActor(TriggeringActor.Get()))
			{
				bIsOverlapping ^= true;
				(bIsOverlapping ? OnBeginOverlap : OnEndOverlap).Broadcast();
			}
			else if (bIsOverlapping)
			{
				OnOverlapUpdate.Broadcast();
			}
			GetTimerManager()->SetTimerForNextTick(Ticker);
			return;
		}
		Cancel();
		});

	Ticker.ExecuteIfBound();
}

