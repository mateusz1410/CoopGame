// Fill out your copyright notice in the Description page of Project Settings.


#include "SPowerupActor.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ASPowerupActor::ASPowerupActor()
{
	PowerupInterval = 0.0f;
	TotalNrOfTicks = 0;
	TicksProcessed = 0.f;

	bIsPowerupActive = false;

	SetReplicates(true);

}


void ASPowerupActor::OnTickPowerup() //always on server
{
	TicksProcessed++;

	OnPowerupTicked();

	if (TicksProcessed >= TotalNrOfTicks)
	{
		OnExpired();

		bIsPowerupActive = false;

		OnRep_Powerupactive(); // is a server, want to exec also on server

		GetWorldTimerManager().ClearTimer(TimerHandle_PowerupTick);

	}
}

void ASPowerupActor::OnRep_Powerupactive()
{
	OnPowerupStateChanged(bIsPowerupActive);
}

void ASPowerupActor::ActivatePowerup(AActor* ActivateFor) //always a server
{
	OnActivated(ActivateFor);

	bIsPowerupActive = true;

	OnRep_Powerupactive(); // is a server, want to exec also on server


	if (PowerupInterval > 0.0f)
	{
		GetWorldTimerManager().SetTimer(TimerHandle_PowerupTick, this, &ASPowerupActor::OnTickPowerup, PowerupInterval, true);
	}
	else
	{
		OnTickPowerup();
	}
}

void ASPowerupActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps)const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASPowerupActor, bIsPowerupActive);

}