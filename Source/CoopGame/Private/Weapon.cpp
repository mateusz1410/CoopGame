// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "CollisionQueryParams.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "CoopGame.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"

static int32 DebugWeaponDrawing = 0; //Global variables have file scope
/** Console command
*/
FAutoConsoleVariableRef CVARDebugWeaponDrawing(TEXT("COOP.DebugWeapons"),DebugWeaponDrawing,TEXT("Draw Debug Lines for Weapons: 0 - hidden 1 - visible"),ECVF_Cheat);
/*
		FAutoConsoleVariableRef - type console variable

		CVARDebugWeaponDrawing  - variable name

		DebugWeaponDrawing  - which variable command set when exec

		TEXT("Draw Debug Lines for Weapons 0 - hidden 1- visible") - helpers describe how command works

		ECVF_Cheat - only work in editor will not work in game
*/


AWeapon::AWeapon()
{
	MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	RootComponent = MeshComponent;

	MuzzleSocketName = "Muzzle";
	TracerTargetName = "Target"; // Target in particle effect

	BaseDamage = 20.f;
	RateOfFire = 600.f;
	BulletSpread = 2.f;

	SetReplicates(true);
	NetUpdateFrequency = 66.f; 
	MinNetUpdateFrequency = 33.0f;// to prevent lag, force show change (if ther is not big change replicate doesn't work)
	//default 2 ipdate per secon , changed to 33 upadte per second
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	TimeBetweenShots = 60/RateOfFire;
}

void AWeapon::OnRep_HitScanTrace() //call on client when HitScaneTrace changed, but no on owner COND_Skip Owner
{
	PlayFireEffect(HitScaneTrace.TraceTo); //just send to other client info where play shot FX
	PlayImpatEffect(HitScaneTrace.SurfaceType, HitScaneTrace.TraceTo);
}

void AWeapon::Fire()
{
	if (Role < ROLE_Authority) // if is a Clien ask server to do it
	{
		ServerFire();
	}
	//and if it's client it do it on his instance as well

	AActor* Shooter = GetOwner(); //owner
	if (Shooter) 
	{
		FVector EyeLocation;
		FRotator EyeRotation;
		Shooter->GetActorEyesViewPoint(EyeLocation,EyeRotation); // function inside this is override GetPawnViewLocation
		//for pawn return Eyes location but from SCharacter return CameraComponent location 

		FVector ShotDirection = EyeRotation.Vector();

		float HalfRad = FMath::DegreesToRadians(BulletSpread); 
		ShotDirection = FMath::VRandCone(ShotDirection, HalfRad, HalfRad); // recoil, bulletSpread (add to vector rand value from cone)

		FVector TraceEnd = EyeLocation + (ShotDirection * 10000);

		FHitResult OutHit;

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(Shooter); //ignore waepon owner ,shooter
		QueryParams.AddIgnoredActor(this); // ignore self
		QueryParams.bTraceComplex = true;////advance trace more expensive, bTraceComplex = true not box(collider hit) but mesh hit(100% precision)
		QueryParams.bReturnPhysicalMaterial = true;// check hit physical Material


		FVector TracerEndPoint = TraceEnd;
		EPhysicalSurface SurfaceType = SurfaceType_Default;
		if (GetWorld()->LineTraceSingleByChannel(OutHit, EyeLocation, TraceEnd, COLLISION_WEAPON, QueryParams))
		{
			//block hit

			AActor* HitActor = OutHit.GetActor();
			
			SurfaceType = UPhysicalMaterial::DetermineSurfaceType(OutHit.PhysMaterial.Get());

			float ActualDamage = BaseDamage;
			if (SurfaceType == SURFACE_FLESHVUNLERABLE)
			{
				UE_LOG(LogTemp, Warning, TEXT("Damage 80"));
				ActualDamage *= 4.0f;
			}

			UGameplayStatics::ApplyPointDamage(HitActor, ActualDamage, ShotDirection, OutHit, Shooter->GetInstigatorController(), Shooter, DamageType);

			PlayImpatEffect(SurfaceType, OutHit.ImpactPoint);


			TracerEndPoint = OutHit.ImpactPoint;
		}
		if(DebugWeaponDrawing > 0) //value set by console command
		{
			DrawDebugLine(GetWorld(), EyeLocation, TraceEnd, FColor::Green, false, 3.f, 0, 1.0f);
		}
		PlayFireEffect(TracerEndPoint); // play muzzle effect

		if (Role == ROLE_Authority)
		{
			HitScaneTrace.TraceTo = TracerEndPoint; // save data about shot from this cient 
			HitScaneTrace.SurfaceType = SurfaceType;
		}

		LastFireTime = GetWorld()->TimeSeconds;
	}

}

void AWeapon::ServerFire_Implementation()
{
	Fire();
}
bool AWeapon::ServerFire_Validate()
{
	return true;
}
void AWeapon::StartFire()
{
	float FirstDelay = FMath::Max(LastFireTime + TimeBetweenShots - GetWorld()->TimeSeconds,0.0f);
	GetWorldTimerManager().SetTimer(TimerHandle_TimeBetweenShots, this, &AWeapon::Fire, TimeBetweenShots, true, FirstDelay);
}

void AWeapon::StopFire()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_TimeBetweenShots);
}


void AWeapon::PlayFireEffect(FVector TracerEndPoint)
{
	if (MuzzleEffect)
	{
		UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComponent, MuzzleSocketName);
	}

	if (TracerEffect)
	{
		FVector MuzzleLocation = MeshComponent->GetSocketLocation(MuzzleSocketName);
		UParticleSystemComponent* TraceComponent = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TracerEffect, MuzzleLocation);
		if (TraceComponent)
		{
			TraceComponent->SetVectorParameter(TracerTargetName, TracerEndPoint); //in particle system is parameter Beam->Target i Beam->Source(from where to which point particle system works)
			// set end points of particle effect by set variable inside particle system.
		}
	}
	APawn* Shooter = Cast<APawn>(GetOwner());
	if (Shooter)
	{
		APlayerController* Controller = Cast<APlayerController>(Shooter->GetController());
		
		if (Controller)
		{
			Controller->ClientPlayCameraShake(FireCamShake);
		}
	}

}
void AWeapon::PlayImpatEffect(EPhysicalSurface SurfaceType, FVector ImpactPoint)
{
	UParticleSystem* SelectedEffect = nullptr;

	switch (SurfaceType)
	{ //names from CoopGame.h 
	case SURFACE_FLESHDEFAULT: SelectedEffect = FleshImpactEffect;  // SURFACE_FLESHDEFAULT macro in "CoopGame.h"
		break;
	case SURFACE_FLESHVUNLERABLE: SelectedEffect = FleshImpactEffect;
		break;
	case SURFACE_GRAVEL: SelectedEffect = GrassImpactEffect;
		break;
	case SURFACE_SAND: SelectedEffect = SandImpactEffect;
		break;
	case SURFACE_GRASS: SelectedEffect = GrassImpactEffect;
		break;
	default: SelectedEffect = DefaultImpactEffect;
		break;
	}

	if (SelectedEffect)
	{
		FVector MuzzleLocation = MeshComponent->GetSocketLocation(MuzzleSocketName);

		FVector  ShotDirection = ImpactPoint - MuzzleLocation;
		ShotDirection.Normalize();
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedEffect, ImpactPoint, ShotDirection.Rotation());
	}
}
void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps)const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AWeapon, HitScaneTrace,COND_SkipOwner); // not replicate to owner, is already maked for player Fire() 
}

