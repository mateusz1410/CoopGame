// Fill out your copyright notice in the Description page of Project Settings.


#include "GrenadeLauncher.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"


void AGrenadeLauncher::Fire()
{


	AActor* Shooter = GetOwner(); //owner
	if (Shooter && ProjectileClass)
	{
		FVector EyeLocation;
		FRotator EyeRotation;
		Shooter->GetActorEyesViewPoint(EyeLocation, EyeRotation); // function inside this is override GetPawnViewLocation
	
		FVector MuzzleLocation = MeshComponent->GetSocketLocation(MuzzleSocketName);

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		GetWorld()->SpawnActor<AActor>(ProjectileClass,MuzzleLocation, EyeRotation, SpawnParams);
	}

}