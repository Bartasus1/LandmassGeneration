// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TerrainModifiers/TerrainModifier.h"
#include "TerrainComponent.generated.h"

class UMaterialInstance;

USTRUCT()
struct FTerrainWeights
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, meta=(ClampMin = 0.f, ClampMax = 1.f, Delta = 0.1))
	float Weight = 1.f;

	UPROPERTY(EditAnywhere, Instanced)
	UTerrainModifier* TerrainModifier;
	
};

UENUM()
enum EUpdateBehaviour
{
	Interactive,
	ValueSet
};


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

	UPROPERTY(EditAnywhere, meta=(EditCondition="bUpdateOnValuesChanged"))
	TEnumAsByte<EUpdateBehaviour> UpdateBehaviour = ValueSet;
	
	UPROPERTY(EditAnywhere,  meta=(ForceInlineRow))
	TArray<FTerrainWeights> TerrainModifierWeights;

	UPROPERTY(EditAnywhere);
	TObjectPtr<UMaterialParameterCollection> ParameterCollection;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	bool WriteHeightDataToTexture(TArray<uint8>& Data, FIntRect Rect, FString LandscapeActorName);


	TArray<uint16> HeightData;
	TArray<uint8> Pixels;
	FIntRect SampleRect;
	
	uint32 MaxHeight;

	
	virtual void OnRegister() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};