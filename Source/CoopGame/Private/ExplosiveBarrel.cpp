// Fill out your copyright notice in the Description page of Project Settings.


#include "ExplosiveBarrel.h"
#include "HealthComponent.h"
#include "Components/StaticMeshComponent.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"


// Sets default values
AExplosiveBarrel::AExplosiveBarrel()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	HealthComponent = CreateDefaultSubobject <UHealthComponent>(TEXT("HealthComponent"));
	HealthComponent->OnHealthChanged.AddDynamic(this, &AExplosiveBarrel::OnHealthChanged); // OnHealthChanged works only on server in HealthComponent

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetSimulatePhysics(true);
	MeshComponent->SetCollisionObjectType(ECC_PhysicsBody);
	RootComponent = MeshComponent;

	RadialForceComponent = CreateDefaultSubobject<URadialForceComponent>(TEXT("RadialForceComponent"));
	RadialForceComponent->SetupAttachment(GetRootComponent());
	RadialForceComponent->Radius = 250;
	RadialForceComponent->bImpulseVelChange = true;
	RadialForceComponent->bAutoActivate = false;
	RadialForceComponent->bIgnoreOwningActor = true;
	ExplosionImpulse = 400;

	SetReplicates(true);
	SetReplicateMovement(true);
}

//Called when the game starts or when spawned
void AExplosiveBarrel::BeginPlay()
{
	Super::BeginPlay();
	
}

//Work only on sever becouse OnHealthChanged is run only on server
void AExplosiveBarrel::OnHealthChanged(UHealthComponent * OwningHealthComp, float Health, float HealthMAX, const UDamageType * DamageType, AController * InstigatedBy, AActor * DamageCauser)
{
	if (!bExploded)
	{
		if (Health <= 0.0f)
		{
			bExploded = true;
			OnRep_Exploded(); // need to call if server, for client will be trigger when value bExploded changed

			FVector Boostintensity = FVector::UpVector*ExplosionImpulse;
			MeshComponent->AddImpulse(Boostintensity, NAME_None, true);
			RadialForceComponent->FireImpulse();//trigger radial force //push everything arounds on sides after explosion
		}
	}
}

void AExplosiveBarrel::OnRep_Exploded()
{
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());
	MeshComponent->SetMaterial(0, ExplodedMaterial);
}

// Called every frame
void AExplosiveBarrel::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AExplosiveBarrel::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps)const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AExplosiveBarrel, bExploded); //replicate bExploded  to every ASCharacter
}

