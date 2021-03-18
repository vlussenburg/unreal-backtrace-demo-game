// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "BacktraceGameGameMode.h"
#include "BacktraceGameHUD.h"
#include "BacktraceGameCharacter.h"
#include "UObject/ConstructorHelpers.h"

ABacktraceGameGameMode::ABacktraceGameGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = ABacktraceGameHUD::StaticClass();
}
