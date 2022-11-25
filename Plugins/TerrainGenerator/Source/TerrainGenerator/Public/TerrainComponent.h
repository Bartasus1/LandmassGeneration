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

	UPROPERTY(EditAnywhere, AdvancedDisplay, meta=(EditCondition="!bUpdateOnValuesChanged"))
	bool bWriteToLog = false;

	UPROPERTY(EditAnywhere, meta=(UIMin = 1, UIMax = 65536))
	uint32 Seed = 1;

	UPROPERTY(EditAnywhere, meta=(UIMin = 0.1, UIMax = 12.0))
	float Frequency = 1.0;
	
	UPROPERTY(EditAnywhere, meta=(UIMin = 0.1, UIMax = 8.0, DisplayName = "Lacunarity (1 / Persistence)"))
	float Persistence = 2;

	UPROPERTY(EditAnywhere, meta=(UIMin = 1, UIMax = 16))
	uint16 Octaves = 1;

	UPROPERTY(EditAnywhere, meta=(UIMin = 1, UIMax = 65536))
	uint32 Elevation = SHRT_MAX;

	
	UPROPERTY(EditAnywhere, meta=(InlineEditConditionToggle))
	bool bUseCurves = false;

	UPROPERTY(EditAnywhere, meta=(EditCondition="bUseCurves"))
	UCurveFloat* TerrainControlCurve;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	bool WriteHeightDataToTexture(TArray<uint8> HeightData, FIntRect SampleRect);

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
