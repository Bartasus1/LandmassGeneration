// Fill out your copyright notice in the Description page of Project Settings.


#include "TerrainModifiers/CurveModifier.h"

float UCurveModifier::GetTerrainHeightValue_Implementation(float X, float Y)
{
	float Height = Super::GetTerrainHeightValue_Implementation(X, Y); //Perlin Noise
	
	return (TerrainControlCurve != nullptr) ? TerrainControlCurve->GetFloatValue(Height) : Height;
}
