// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "SGameState.generated.h"

UENUM(BlueprintType)
enum class EWaveState : uint8
{
	EWS_WatingToStart UMETA(DisplayName = "WatingToStart"),
	
	EWS_WaveInProgress UMETA(DisplayName = "WaveInProgress"),

	EWS_WaitingToComplete UMETA(DisplayName = "WaitingToComplete"), //no longer spawn bot, wait for player to kill them

	EWS_WaveComplete UMETA(DisplayName = "WaveComplete"),

	EWS_GameOver UMETA(DisplayName = "GameOver"),



	EWS_MAX UMETA(DisplayName = "MAX")
};

/**
 * 
 */
UCLASS()
class COOPGAME_API ASGameState : public AGameStateBase
{
	GENERATED_BODY()

protected:

	UFUNCTION()
	void OnRep_WaveState(EWaveState OldState);

	UFUNCTION(BlueprintImplementableEvent, Category = "GameState" )
	void WaveStateChange(EWaveState NewState, EWaveState OldState);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_WaveState, Category = "GameState")
	EWaveState WaveState;

public:

	ASGameState();

	void SetWaveState(EWaveState NewState);

};
