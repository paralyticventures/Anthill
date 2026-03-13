// Fill out your copyright notice in the Description page of Project Settings.

#include "AnthillPickupBase.h"
#include "Anthill/Characters/AnthillCharacterBase.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Actor.h"

AAnthillPickupBase::AAnthillPickupBase()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	CollisionSphere->SetupAttachment(RootComponent);
	CollisionSphere->SetSphereRadius(50.f);
	CollisionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
}

void AAnthillPickupBase::BeginPlay()
{
	Super::BeginPlay();
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

	Destroy();
}
