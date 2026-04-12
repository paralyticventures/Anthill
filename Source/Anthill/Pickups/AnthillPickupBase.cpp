// Fill out your copyright notice in the Description page of Project Settings.

#include "AnthillPickupBase.h"
#include "Anthill/AnthillGameInstance.h"
#include "Anthill/Characters/AnthillCharacterBase.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"

AAnthillPickupBase::AAnthillPickupBase()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	CollisionSphere->SetupAttachment(RootComponent);
	CollisionSphere->SetSphereRadius(50.f);
	CollisionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
}

FString AAnthillPickupBase::GetPickupSaveKey() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return FString();
	}
	const FString LevelName = UGameplayStatics::GetCurrentLevelName(World);
	if (!SavePickupIdOverride.IsNone())
	{
		return FString::Printf(TEXT("%s|%s"), *LevelName, *SavePickupIdOverride.ToString());
	}
	return FString::Printf(TEXT("%s|%s"), *LevelName, *GetFName().ToString());
}

void AAnthillPickupBase::RegisterPickupCollectedForSave()
{
	if (!HasAuthority())
	{
		return;
	}
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	if (UAnthillGameInstance* GI = Cast<UAnthillGameInstance>(World->GetGameInstance()))
	{
		GI->AddCollectedPickupKey(GetPickupSaveKey());
	}
}

void AAnthillPickupBase::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		if (UWorld* World = GetWorld())
		{
			if (UAnthillGameInstance* GI = Cast<UAnthillGameInstance>(World->GetGameInstance()))
			{
				if (GI->IsPickupKeyCollected(GetPickupSaveKey()))
				{
					Destroy();
					return;
				}
			}
		}
	}

	CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AAnthillPickupBase::OnSphereOverlap);
}

void AAnthillPickupBase::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority())
	{
		return;
	}
	AAnthillCharacterBase* Character = Cast<AAnthillCharacterBase>(OtherActor);
	if (!Character)
	{
		return;
	}

	if (bAddToInventory)
	{
		float V1 = 0.f, V2 = 0.f;
		switch (PickupType)
		{
		case EPickupType::Heal:        V1 = HealAmount; break;
		case EPickupType::MaxStamina:  V1 = MaxStaminaAmount; break;
		case EPickupType::MaxHealth:   V1 = MaxHealthAmount; break;
		case EPickupType::AttackBuff:  V1 = AttackBuffMultiplier; V2 = AttackBuffDurationSeconds; break;
		default: break;
		}
		if (Character->AddItemToInventory(PickupType, V1, V2))
		{
			RegisterPickupCollectedForSave();
			Destroy();
		}
		return;
	}

	switch (PickupType)
	{
	case EPickupType::Heal:
		Character->Heal(HealAmount);
		break;
	case EPickupType::MaxStamina:
		Character->AddMaxStamina(MaxStaminaAmount);
		break;
	case EPickupType::MaxHealth:
		Character->AddMaxHealth(MaxHealthAmount);
		break;
	case EPickupType::AttackBuff:
		Character->SetAttackBuff(AttackBuffMultiplier, AttackBuffDurationSeconds);
		break;
	default:
		break;
	}

	RegisterPickupCollectedForSave();
	Destroy();
}
