﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PerlinNoiseModifier.h"
#include "CurveModifier.generated.h"

/**
 * 
 */
UCLASS()
class TERRAINGENERATOR_API UCurveModifier : public UPerlinNoiseModifier
{
	GENERATED_BODY()
public:
	
	UPROPERTY(EditAnywhere)
	UCurveFloat* TerrainControlCurve;

	virtual float GetTerrainHeightValue_Implementation(float X, float Y) override;
};