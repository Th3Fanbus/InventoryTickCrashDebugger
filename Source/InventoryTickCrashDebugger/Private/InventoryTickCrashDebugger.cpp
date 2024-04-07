/* SPDX-License-Identifier: MPL-2.0 */

#include "InventoryTickCrashDebugger.h"
#include "FGInventoryComponent.h"
#include "Buildables/FGBuildable.h"
#include "Replication/FGReplicationDetailActor.h"
#include "Patching/NativeHookManager.h"
#include "FGPlayerController.h"
#include "Player/SMLRemoteCallObject.h"

DEFINE_LOG_CATEGORY_STATIC(LogInvCrashDbg, Log, All);

namespace Th3
{
	static FORCEINLINE FString GetNameSafe(const UObject* Obj)
	{
		return Obj ? Obj->GetName() : TEXT("NULL");
	}

	static FORCEINLINE FString GetPathSafe(const UObject* Obj)
	{
		return Obj ? Obj->GetPathName() : TEXT("NULL");
	}

	static FORCEINLINE FString InvItemToStr(const FInventoryItem& Item)
	{
		if (Item.HasState()) {
			return FString::Printf(TEXT("[%s (%d) ---> %s]"), *Th3::GetNameSafe(Item.GetItemClass()), Item.GetItemStackSize(), *Th3::GetNameSafe(Item.ItemState.Get()));
		} else {
			return FString::Printf(TEXT("[%s (%d)]"), *Th3::GetNameSafe(Item.GetItemClass()), Item.GetItemStackSize());
		}
	}

	static void ShowRestartRequired(UObject* WorldContext)
	{
		if (not IsValid(WorldContext)) {
			UE_LOG(LogInvCrashDbg, Error, TEXT("INVALID WORLD CONTEXT"));
			return;
		}
		FString Message;
		Message += TEXT("=== RESTART REQUIRED ===");
		Message += TEXT("\nCrash avoided. Save the game to a NEW save file (DO NOT OVERWRITE ANY EXISTING SAVES), ");
		Message += TEXT("then quit the game and post the FactoryGame.log to the Satisfactory Modding discord. ");
		Message += TEXT("Do exactly as instructed. Do NOT ignore this message or you risk SAVE CORRUPTION.");
		Message += TEXT("\n=== YOU HAVE BEEN WARNED ===");
		AFGPlayerController* PlayerController = Cast<AFGPlayerController>(UGameplayStatics::GetPlayerController(WorldContext, 0));
		if (IsValid(PlayerController)) {
			USMLRemoteCallObject* RemoteCallObject = Cast<USMLRemoteCallObject>(PlayerController->GetRemoteCallObjectOfClass(USMLRemoteCallObject::StaticClass()));
			if (IsValid(RemoteCallObject)) {
				RemoteCallObject->SendChatMessage(Message, FLinearColor::Red);
			}
			UFGBlueprintFunctionLibrary::AddPopupWithCloseDelegate(
				PlayerController,
				FText::FromString(TEXT("=== RESTART REQUIRED ===")),
				FText::FromString(Message),
				FPopupClosed()
			);
		}
	}
}

class HFGInventoryComponent
{
public:
	static void OnItemsRemoved(TCallScope<void(__cdecl*)(UFGInventoryComponent*, int32, int32, FInventoryItem)>& scope, UFGInventoryComponent* self, int32 idx, int32 num, FInventoryItem item)
	{
		if (not self) {
			UE_LOG(LogInvCrashDbg, Error, TEXT("SELF IS NULL???"));
			return;
		}
		if (not IsValid(item.GetItemClass())) {
			uint32 ThreadID = FPlatformTLS::GetCurrentThreadId();
			FString ThreadName = FThreadManager::Get().GetThreadName(ThreadID);
			UE_LOG(LogInvCrashDbg, Error, TEXT("INVALID INVENTORY ITEM CAUGHT"));
			UE_LOG(LogInvCrashDbg, Error, TEXT("ON THREAD '%s' (%u)"), *ThreadName, ThreadID);
			UE_LOG(LogInvCrashDbg, Error, TEXT("FUNCTION CALL: %s(%s, %d, %d, %s)"), *FString(__func__), *Th3::GetNameSafe(self), idx, num, *Th3::InvItemToStr(item));
			UE_LOG(LogInvCrashDbg, Error, TEXT("    COMPONENT: %s"), *Th3::GetPathSafe(self));
			UE_LOG(LogInvCrashDbg, Error, TEXT("    COMPCLASS: %s"), *Th3::GetPathSafe(self->GetClass()));
			UE_LOG(LogInvCrashDbg, Error, TEXT("    INV OWNER: %s"), *Th3::GetPathSafe(self->GetOwner()));
			if (AActor* Owner = self->GetOwner()) {
				UE_LOG(LogInvCrashDbg, Error, TEXT("    OWNER CLS: %s"), *Th3::GetPathSafe(Owner->GetClass()));
				UE_LOG(LogInvCrashDbg, Error, TEXT("    OWNER LOC: %s"), *Owner->GetActorLocation().ToString());
				if (AFGReplicationDetailActor* ReplicationActor = Cast<AFGReplicationDetailActor>(Owner)) {
					AFGBuildable* Buildable = ReplicationActor->GetOwningBuildable();
					UE_LOG(LogInvCrashDbg, Error, TEXT("    BUILDABLE: %s"), *Th3::GetPathSafe(Buildable));
					UE_LOG(LogInvCrashDbg, Error, TEXT("    BUILD CLS: %s"), *Th3::GetPathSafe(Buildable->GetClass()));
					UE_LOG(LogInvCrashDbg, Error, TEXT("    BUILD LOC: %s"), *Buildable->GetActorLocation().ToString());
				}
			}
			UE_LOG(LogInvCrashDbg, Error, TEXT("CANCELLING EXEC TO AVOID CRASH"));
			scope.Cancel();
			UObject* WorldContext = IsValid(self->GetOwner()) ? static_cast<UObject*>(self->GetOwner()) : static_cast<UObject*>(self);
			AsyncTask(ENamedThreads::GameThread, [WorldContext]() {
				Th3::ShowRestartRequired(WorldContext);
			});
		}
	}
};

void FInventoryTickCrashDebuggerModule::StartupModule()
{
	if (not WITH_EDITOR) {
		SUBSCRIBE_UOBJECT_METHOD(UFGInventoryComponent, OnItemsRemoved, &HFGInventoryComponent::OnItemsRemoved);
	}
}

void FInventoryTickCrashDebuggerModule::ShutdownModule()
{
}

IMPLEMENT_MODULE(FInventoryTickCrashDebuggerModule, InventoryTickCrashDebugger)