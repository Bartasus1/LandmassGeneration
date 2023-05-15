// Fill out your copyright notice in the Description page of Project Settings.


#include "TerrainModifiers/PerlinCurveModifier.h"

float UPerlinCurveModifier::GetTerrainHeightValue_Implementation(float X, float Y)
{
	float Height = Super::GetTerrainHeightValue_Implementation(X, Y); //Perlin Noise (-1, 1) range
	
	return (TerrainControlCurve != nullptr) ? TerrainControlCurve->GetFloatValue(Height) : Height;
}
