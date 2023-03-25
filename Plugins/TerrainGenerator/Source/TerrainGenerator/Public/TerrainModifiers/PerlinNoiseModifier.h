// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TerrainModifier.h"
#include "PerlinNoiseModifier.generated.h"

/**
 * 
 */
UCLASS()
class TERRAINGENERATOR_API UPerlinNoiseModifier : public UTerrainModifier
{
	GENERATED_BODY()
public:
	
	UPROPERTY(EditAnywhere, meta=(UIMin = 0.1, UIMax = 12.0))
	float Frequency = 1.0;
	
	UPROPERTY(EditAnywhere, meta=(UIMin = 0.1, UIMax = 8.0, DisplayName = "Lacunarity (1 / Persistence)"))
	float Persistence = 2;

	UPROPERTY(EditAnywhere, meta=(UIMin = 1, UIMax = 16))
	uint16 Octaves = 1;
	
	virtual float GetTerrainHeightValue_Implementation(float X, float Y) override;
};
