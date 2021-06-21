// Fill out your copyright notice in the Description page of Project Settings.


#include "SGameState.h"
#include "Net/UnrealNetwork.h"


ASGameState::ASGameState()
{
	SetReplicates(true);
}

void ASGameState::SetWaveState(EWaveState NewState)
{
	if (Role == ROLE_Authority)
	{
		EWaveState OldState = WaveState;
		WaveState = NewState;

		//call on server
		OnRep_WaveState(OldState); // in BP
	}

}

void ASGameState::OnRep_WaveState(EWaveState OldState)
{

	WaveStateChange(WaveState,OldState); // in BP (NewValue, OldValue)
}

void ASGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps)const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASGameState, WaveState); //replicate CurrentWeapon  to every ASCharacter

}
