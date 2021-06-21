// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

USTRUCT() //contains information of a single hitscan weapon lineTrace
struct FHitScanTrace
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TEnumAsByte<EPhysicalSurface> SurfaceType;// change enum tu byte, else is bug
	UPROPERTY()
	FVector_NetQuantize TraceTo; //FVector_Net Quantize prepared to send throw net, less precise but work faster
	
};


UCLASS()
class COOPGAME_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeapon();

protected:

	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USkeletalMeshComponent* MeshComponent;

	void PlayFireEffect(FVector TracerEndPoint);

	void PlayImpatEffect(EPhysicalSurface SurfaceType, FVector ImpactPoint);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf< class UDamageType>  DamageType;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon | Shoot")
	FName MuzzleSocketName;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon | Shoot")
	FName TracerTargetName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon | ParticleSystem")
	class UParticleSystem* MuzzleEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon | ParticleSystem")
	UParticleSystem* DefaultImpactEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon | ParticleSystem")
	UParticleSystem* FleshImpactEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon | ParticleSystem")
	UParticleSystem* GravelImpactEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon | ParticleSystem")
	UParticleSystem* GrassImpactEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon | ParticleSystem")
	UParticleSystem* SandImpactEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon | ParticleSystem")
	UParticleSystem* TracerEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<class UCameraShake> FireCamShake;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float BaseDamage;

	FTimerHandle TimerHandle_TimeBetweenShots;

	float LastFireTime;

	/*Bullets per minute*/
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float RateOfFire;

	/*Buller Spreed in Degrees*/
	UPROPERTY(EditDefaultsOnly, Category = "Weapon", meta = (ClampMin = 0.0f))
	float BulletSpread;

	//from RateOfFire
	float TimeBetweenShots;

	UPROPERTY(ReplicatedUsing = OnRep_HitScanTrace)// call function OnRep_HitScanTrace to replicate
	FHitScanTrace HitScaneTrace;

	UFUNCTION()
	void OnRep_HitScanTrace();

public:
	virtual void Fire();

	UFUNCTION(Server,Reliable,WithValidation)// Server function run always on server, Reliable guarantee it will be fire
	void ServerFire();

	void StartFire();
	void StopFire();

	//friend class ASCharacter;

};
