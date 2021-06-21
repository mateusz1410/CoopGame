// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SCharacter.generated.h"

UCLASS()
class COOPGAME_API ASCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void MoveForward(float Value); //Axis need value
	void MoveRight(float Value);
	void BeginCrouch();
	void EndCrounch();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UCameraComponent* CameraComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USpringArmComponent* SpringArmComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UHealthComponent* HealthComponent;


	bool bWantsToZoom;

	UPROPERTY(EditDefaultsOnly, Category = "CameraZoom")
	float ZoomedFOV;

	UPROPERTY(EditDefaultsOnly, Category = "CameraZoom", meta = (ClampMin = 0.1, ClampMax = 100 ))
	float ZoomInterpSpeed;

	float DefaultValueFOV;

	void BeginZoom();
	void EndZoom();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<class AWeapon> StarterWeaponClass;

	UPROPERTY(Replicated,VisibleAnywhere, BlueprintReadWrite, Category = "Weapon")
	class AWeapon* CurrentWeapon;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName WeaponAttachSocketName;


	UFUNCTION()
	void OnHealthChanged(UHealthComponent* OwningHealthComp, float Health, float HealthMAX, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	UPROPERTY(Replicated,BlueprintReadWrite,VisibleAnywhere, Category = "PlayerState")
	bool bDied;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual FVector GetPawnViewLocation()const override; // for pawn retrun eye location, for this character return CameraComponent->GetComponentLocation()
	
	UFUNCTION(BlueprintCallable, Category = "Player")
	void StartFire();
	
	UFUNCTION(BlueprintCallable, Category = "Player")
	void StopFire();

};
