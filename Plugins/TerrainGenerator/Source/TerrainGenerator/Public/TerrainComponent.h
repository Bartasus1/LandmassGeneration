// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TerrainComponent.generated.h"

class UMaterialInstance;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TERRAINGENERATOR_API UTerrainComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UTerrainComponent();

	UFUNCTION(CallInEditor)
	void GenerateLandmass();

	UPROPERTY(EditAnywhere)
	bool bUpdateOnValuesChanged = false;

	UPROPERTY(EditAnywhere, meta=(UIMin = 0.1, UIMax = 10.0))
	float Frequency = 1.0;

	UPROPERTY(EditAnywhere, meta=(UIMin = 1, UIMax = 10.0))
	float Lacunarity = 2.0;

	UPROPERTY(EditAnywhere, meta=(UIMin = 0, UIMax = 1.0))
	float Persistence = 0.5;


	UPROPERTY(EditAnywhere, meta=(UIMin = 1, UIMax = 8))
	uint16 Octaves = 1;

	UPROPERTY(EditAnywhere, meta=(UIMin = 100, UIMax = 10000))
	uint32 Elevation = 8000;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	bool WriteHeightDataToTexture(TArray<uint8> HeightData, FIntRect SampleRect);

	void ClampHeightValue(float &NoiseValue);

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
