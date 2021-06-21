// Fill out your copyright notice in the Description page of Project Settings.


#include "STrackerBot.h"
#include "Components/StaticMeshComponent.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "DrawDebugHelpers.h"
#include "HealthComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Particles/ParticleSystem.h"
#include "Components/SphereComponent.h"
#include "SCharacter.h"
#include "TimerManager.h"
#include "Sound/SoundBase.h"

static int32 DebugTrackerBotDrawing = 0; //Global variables have file scope
/** Console command
*/
FAutoConsoleVariableRef CVARDebugTrackerBotDrawing(TEXT("COOP.DebugTrackerBot"), DebugTrackerBotDrawing, TEXT("Draw Debug Lines for TrackerBot: 0 - hidden 1 - visible"), ECVF_Cheat);
/*
		FAutoConsoleVariableRef - type console variable

		CVARDebugWeaponDrawing  - variable name

		DebugWeaponDrawing  - which variable command set when exec

		TEXT("Draw Debug Lines for Weapons 0 - hidden 1- visible") - helpers describe how command works

		ECVF_Cheat - only work in editor will not work in game
*/

// Sets default values
ASTrackerBot::ASTrackerBot()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;
	MeshComp->SetCanEverAffectNavigation(false);
	MeshComp->SetSimulatePhysics(true);

	HealthComp = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	HealthComp->OnHealthChanged.AddDynamic(this, &ASTrackerBot::HandleTakeDamage);

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetSphereRadius(200);
	SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComp->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	SphereComp->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	SphereComp->SetupAttachment(GetRootComponent());

	MovementForce = 1000;
	bUseVelocityChange = false;
	RequiredDistanceToTarget = 100;

	ExplosionRadius = 350;
	ExplosionDamage = 50;
	SelfDamageInterval = 0.25f;

	bExploded = false;
	bStartedSelfDestruction = false;

	PowerLevel = 0;
}

// Called when the game starts or when spawned
void ASTrackerBot::BeginPlay()
{
	Super::BeginPlay();

	//find initial move to
	if (Role == ROLE_Authority)
	{
		NextPathPoint = GetNextPathPoint();

		FTimerHandle TimerHandle_CheckPowerLevel;
		GetWorldTimerManager().SetTimer(TimerHandle_CheckPowerLevel, this, &ASTrackerBot::OnCheckNearbyBots, 1.0f, true);
	}

	//OnActorBeginOverlap.AddDynamic(this,&ASTrackerBot::fiunctionName);


}


// Called every frame
void ASTrackerBot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if(Role == ROLE_Authority && !bExploded)
	{

		float DistanceToTarget = (GetActorLocation() - NextPathPoint).Size(); //.Size() vector length
		if (DistanceToTarget <= RequiredDistanceToTarget) //GetActorLocation().Equals(NextPathPoint) compare vector
		{
			NextPathPoint = GetNextPathPoint();

			if (DebugTrackerBotDrawing > 0)
			{
				DrawDebugString(GetWorld(), GetActorLocation(), "Target Reached!");
			}

		}
		else
		{
			FVector ForceDirection = NextPathPoint - GetActorLocation();
			//ForceDirection.GetSafeNormal();
			ForceDirection.Normalize();
			ForceDirection *= MovementForce;

			MeshComp->AddForce(ForceDirection, NAME_None, bUseVelocityChange);

			if (DebugTrackerBotDrawing > 0)
			{
				DrawDebugDirectionalArrow(GetWorld(), GetActorLocation(), GetActorLocation() + ForceDirection, 32, FColor::Blue, false, 0.0f, 0, 2.0f);
			}
		}

	}
	if (DebugTrackerBotDrawing > 0)
	{
		DrawDebugSphere(GetWorld(), NextPathPoint, 30, 12, FColor::Green, false, 0.0f, 1.0f);
	}
}

void ASTrackerBot::HandleTakeDamage(UHealthComponent * OwningHealthComp, float Health, float HealthMAX, const UDamageType * DamageType, AController * InstigatedBy, AActor * DamageCauser)
{
	//Explode

	//@TODO: Pulse the material on hit
	if (MatInst == nullptr)
	{
		MatInst = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComp->GetMaterial(0));

	}

	if (MatInst)
	{
		MatInst->SetScalarParameterValue(TEXT("LastTimeDamageTaken"), GetWorld()->TimeSeconds);
	}

	if (Health <= 0.0f)
	{
		SelfDestruct();
	}

	//UE_LOG(LogTemp, Warning, TEXT("Health %s of %s"), *FString::SanitizeFloat(Health), *GetName());
}

FVector ASTrackerBot::GetNextPathPoint()
{
	AActor* BestTarget = nullptr;
	float NearestTargetDistance = FLT_MAX; //FLT_MAX  return max float value

	for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; ++It)
	{
		APawn* TestPawn = It->Get();
		if (TestPawn == nullptr || UHealthComponent::IsFriendly(this,TestPawn))
		{
			continue;
		}

		UHealthComponent* TestPawnHealthComp = Cast<UHealthComponent>(TestPawn->GetComponentByClass(UHealthComponent::StaticClass()));
		if (TestPawnHealthComp && TestPawnHealthComp->GetHealth() > 0)
		{
			float Distance = (TestPawn->GetActorLocation() - GetActorLocation()).Size();

			if (Distance < NearestTargetDistance)
			{
				BestTarget = TestPawn;
				NearestTargetDistance = Distance;
			}
		}
	}

	//ACharacter* PlayerPawn = UGameplayStatics::GetPlayerCharacter(this, 0);
	//FindPathToActorSynchronously - find all point in the same time, points lead to the target actor
	
	if (BestTarget)
	{
		UNavigationPath* NavPath = UNavigationSystemV1::FindPathToActorSynchronously(this, GetActorLocation(), BestTarget);

		GetWorldTimerManager().ClearTimer(TimerHandle_RefreshPath);
		GetWorldTimerManager().SetTimer(TimerHandle_RefreshPath, this, &ASTrackerBot::RefreshPath, 5.0f, false);

		if ( NavPath && NavPath->PathPoints.Num() > 1)
		{
			return NavPath->PathPoints[1]; //return next point  in the path
		}

	}

	//Failed to find path
	return GetActorLocation();
}



void ASTrackerBot::NotifyActorBeginOverlap(AActor * OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if (!bStartedSelfDestruction && !bExploded)
	{
		ASCharacter* PlayerPawn = Cast<ASCharacter>(OtherActor);

		if (PlayerPawn && !UHealthComponent::IsFriendly(this,PlayerPawn))
		{
			if (Role == ROLE_Authority)
			{
				GetWorldTimerManager().SetTimer(TimerHandle_SelfDamage, this, &ASTrackerBot::DamageSelf, SelfDamageInterval, true, 0.0f);

			}
				bStartedSelfDestruction = true;

			UGameplayStatics::SpawnSoundAttached(SelfDestructSound, GetRootComponent());
		}
	}
}

void ASTrackerBot::SelfDestruct()
{
	if (bExploded) return;

	bExploded = true;

	if (ExplosionEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());
	}
	UGameplayStatics::PlaySoundAtLocation(this, ExplosionSound, GetActorLocation());

	MeshComp->SetVisibility(false);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (Role == ROLE_Authority)
	{
		TArray<AActor*> IgnoredActors;
		IgnoredActors.Add(this);
		float ActualDamage = ExplosionDamage + (ExplosionDamage * PowerLevel);

		if (DebugTrackerBotDrawing > 0)
		{
			DrawDebugSphere(GetWorld(), GetActorLocation(), ActualDamage, 12, FColor::Red, false, 2.0f, 0, 1.0f);
		}
		UGameplayStatics::ApplyRadialDamage(this, ExplosionDamage, GetActorLocation(), ExplosionRadius, nullptr, IgnoredActors, this, GetInstigatorController(), true);
	
		//Destroy();
		SetLifeSpan(2.f);
	}
}

void ASTrackerBot::DamageSelf()
{

	//Start self destruction sequence
	UGameplayStatics::ApplyDamage(this, 20, GetInstigatorController(), this, nullptr);
}

void ASTrackerBot::OnCheckNearbyBots()
{
	const float Radius = 600;
	//tempolary collision shape for overlaps
	FCollisionShape CollShape; // collider
	CollShape.SetSphere(Radius);

	FCollisionObjectQueryParams QueryParams; //only playes and AI Bots, list of selected object type
	QueryParams.AddObjectTypesToQuery(ECollisionChannel::ECC_PhysicsBody);
	QueryParams.AddObjectTypesToQuery(ECollisionChannel::ECC_Pawn);

	TArray<FOverlapResult>Overlaps; //outparameter
	GetWorld()->OverlapMultiByObjectType(Overlaps, GetActorLocation(), FQuat::Identity, QueryParams, CollShape); //get overlapping actor of type

	if (DebugTrackerBotDrawing > 0)
	{
		DrawDebugSphere(GetWorld(), GetActorLocation(), Radius, 12, FColor::White, false, 1.0f);
	}
	int32 NrOfBots = 0;
	for (FOverlapResult Result : Overlaps)//FOverlapResult actor and component that overlap
	{
		ASTrackerBot* Bot = Cast<ASTrackerBot>(Result.GetActor()); // get actor ref
		if (Bot && Bot != this)
		{
			NrOfBots++;
		}
	}
	const int32 MaxPowerLevel = 4;
	PowerLevel = FMath::Clamp(NrOfBots, 0, MaxPowerLevel);
	
	//update material color
	if (MatInst == nullptr)
	{
		MatInst = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComp->GetMaterial(0));
		//create new material and set it

	}
	if (MatInst)
	{
		float Alpha = PowerLevel / (float)MaxPowerLevel;

		MatInst->SetScalarParameterValue("PowerLevelAlpha", Alpha); // set parameter
	}
	if (DebugTrackerBotDrawing > 0)
	{
		DrawDebugString(GetWorld(), FVector(0, 0, 0), FString::FromInt(PowerLevel), this, FColor::White, 1.0f, true);
	}
}

void ASTrackerBot::RefreshPath()
{
	NextPathPoint = GetNextPathPoint();
}
