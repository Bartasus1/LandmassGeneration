// Copyright Epic Games, Inc. All Rights Reserved.

#include "LandmassGenerationGameMode.h"
#include "LandmassGenerationCharacter.h"
#include "UObject/ConstructorHelpers.h"

ALandmassGenerationGameMode::ALandmassGenerationGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
