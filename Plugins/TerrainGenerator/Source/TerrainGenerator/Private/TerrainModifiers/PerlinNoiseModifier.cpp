// Fill out your copyright notice in the Description page of Project Settings.


#include "TerrainModifiers/PerlinNoiseModifier.h"

float UPerlinNoiseModifier::GetTerrainHeightValue_Implementation(float X, float Y)
{
	float NoiseValue = 0;
	float AmplitudeSum = 0;
	float Multiplier = 1;
	
	for (int k = 0; k < Octaves; k++)
	{
		const float PerlinValue = (FMath::PerlinNoise2D(FVector2D((Frequency * Multiplier) * X, (Frequency * Multiplier) * Y)));

		NoiseValue += PerlinValue * (1 / Multiplier);
		AmplitudeSum += (1 / Multiplier);

		Multiplier *= Persistence;
	}
	NoiseValue /= AmplitudeSum;

	return  NoiseValue;
}
