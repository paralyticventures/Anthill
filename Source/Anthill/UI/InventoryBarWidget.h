// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Anthill/Pickups/AnthillPickupBase.h"
#include "Anthill/Characters/AnthillCharacterBase.h"
#include "InventoryBarWidget.generated.h"

class UBorder;

/**
 * Widget paska przedmiotów (EQ). W Blueprint: utwórz Widget dziedziczący z tej klasy,
 * dodaj np. 8 slotów (Image/Border) i powiąż z SlotTypes / SelectedSlotIndex.
 * Odświeżanie: RefreshFromCharacter() (np. z NativeTick).
 */
UCLASS()
class ANTHILL_API UInventoryBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Odświeża dane z aktualnej postaci gracza (wywołaj z Blueprint lub używaj NativeTick). */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void RefreshFromCharacter();

	/** Typ przedmiotu w każdym slocie (Empty = brak przedmiotu – nie używaj EPickupType jako "pusty", sprawdź IsSlotValid). */
	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	TArray<EPickupType> SlotTypes;

	/** Czy dany slot ma przedmiot (indeks 0..GetSlotCount()-1). */
	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	TArray<bool> SlotValid;

	/** Indeks aktualnie wybranego slotu (0..7). */
	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	int32 SelectedSlotIndex = 0;

	/** Liczba slotów (zgodna z Character::InventorySlotCount). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	int32 GetSlotCount() const { return AAnthillCharacterBase::InventorySlotCount; }

	/** Czy w danym slocie (0..7) jest przedmiot. Użyj zamiast Slot Valid + Get element. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	bool IsSlotValid(int32 SlotIndex) const;

	/** Typ przedmiotu w danym slocie (0..7). Sprawdź IsSlotValid przed użyciem. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	EPickupType GetSlotType(int32 SlotIndex) const;

	/** Ustaw referencję do Bordera dla slotu (wywołaj w Event Construct: Set Slot Border(0, Inv1), Set Slot Border(1, Inv2), ...). */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetSlotBorder(int32 SlotIndex, UBorder* Border);

	/** Zwraca Border dla slotu (0..7). Użyj w pętli zamiast 8 osobnych gałęzi. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	UBorder* GetSlotBorder(int32 SlotIndex) const;

protected:
	TArray<UBorder*> SlotBorders;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
};
