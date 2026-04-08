// Fill out your copyright notice in the Description page of Project Settings.

#include "InventoryBarWidget.h"
#include "Anthill/Characters/AnthillCharacterBase.h"
#include "Components/Border.h"
#include "Components/Image.h"

void UInventoryBarWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	if (IsDesignTime())
	{
		return;
	}
	RefreshFromCharacter();
}

void UInventoryBarWidget::RefreshFromCharacter()
{
	AAnthillCharacterBase* Character = nullptr;
	if (APlayerController* PC = GetOwningPlayer())
	{
		Character = Cast<AAnthillCharacterBase>(PC->GetPawn());
	}

	const int32 N = GetSlotCount();
	SlotTypes.SetNum(N);
	SlotValid.SetNum(N);
	SlotStackCounts.SetNum(N);

	if (!Character)
	{
		for (int32 i = 0; i < N; ++i)
		{
			SlotValid[i] = false;
			SlotTypes[i] = EPickupType::Heal;
			SlotStackCounts[i] = 0;
		}
		SelectedSlotIndex = 0;
		return;
	}

	SelectedSlotIndex = Character->GetSelectedInventorySlotIndex();
	for (int32 i = 0; i < N; ++i)
	{
		const FInventorySlot InvSlot = Character->GetInventorySlot(i);
		SlotValid[i] = InvSlot.bValid;
		SlotTypes[i] = InvSlot.Type;
		SlotStackCounts[i] = InvSlot.bValid ? InvSlot.StackCount : 0;
	}
}

bool UInventoryBarWidget::IsSlotValid(int32 SlotIndex) const
{
	if (SlotIndex < 0 || SlotIndex >= SlotValid.Num()) return false;
	return SlotValid[SlotIndex];
}

EPickupType UInventoryBarWidget::GetSlotType(int32 SlotIndex) const
{
	if (SlotIndex < 0 || SlotIndex >= SlotTypes.Num()) return EPickupType::Heal;
	return SlotTypes[SlotIndex];
}

int32 UInventoryBarWidget::GetSlotStackCount(int32 SlotIndex) const
{
	if (SlotIndex < 0 || SlotIndex >= SlotStackCounts.Num()) return 0;
	return SlotStackCounts[SlotIndex];
}

void UInventoryBarWidget::SetSlotBorder(int32 SlotIndex, UBorder* Border)
{
	if (SlotIndex < 0) return;
	if (SlotIndex >= SlotBorders.Num())
	{
		SlotBorders.SetNum(SlotIndex + 1);
	}
	SlotBorders[SlotIndex] = Border;
}

UBorder* UInventoryBarWidget::GetSlotBorder(int32 SlotIndex) const
{
	if (SlotIndex < 0 || SlotIndex >= SlotBorders.Num()) return nullptr;
	return SlotBorders[SlotIndex];
}

void UInventoryBarWidget::SetSlotIconImage(int32 SlotIndex, UImage* Image)
{
	if (SlotIndex < 0) return;
	if (SlotIndex >= SlotIconImages.Num())
	{
		SlotIconImages.SetNum(SlotIndex + 1);
	}
	SlotIconImages[SlotIndex] = Image;
}

UImage* UInventoryBarWidget::GetSlotIconImage(int32 SlotIndex) const
{
	if (SlotIndex < 0 || SlotIndex >= SlotIconImages.Num()) return nullptr;
	return SlotIconImages[SlotIndex];
}
