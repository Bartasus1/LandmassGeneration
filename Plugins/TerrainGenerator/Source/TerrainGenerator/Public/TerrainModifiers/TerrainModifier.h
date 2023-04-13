// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "TerrainModifier.generated.h"

/**
 * 
 */

UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced, CollapseCategories)
class TERRAINGENERATOR_API UTerrainModifier : public UObject
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, meta=(UIMin = 0, UIMax = 65535, Delta = 5.f))
	uint32 Seed = 0;
	
	UPROPERTY(EditAnywhere, meta=(UIMin = 1, UIMax = 65536, Delta = 100.f))
	int32 Elevation = SHRT_MAX;

	UFUNCTION(BlueprintNativeEvent)
	float GetTerrainHeightValue(float X, float Y);
};
