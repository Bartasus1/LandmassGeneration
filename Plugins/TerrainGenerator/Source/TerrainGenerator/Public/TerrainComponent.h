// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TerrainModifiers/TerrainModifier.h"
#include "TerrainComponent.generated.h"

class UMaterialInstance;

USTRUCT()
struct FWeightedTerrainModifier
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
	None,
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

	UFUNCTION(CallInEditor)
	void RegenerateLandscapeInfo();

	UPROPERTY(EditAnywhere)
	bool bGenerateAdditionalHeightmap = false;

	UPROPERTY(EditAnywhere)
	TEnumAsByte<EUpdateBehaviour> UpdateBehaviour = ValueSet;
	
	UPROPERTY(EditAnywhere)
	TArray<FWeightedTerrainModifier> TerrainModifiers;

	UPROPERTY(EditAnywhere);
	TObjectPtr<UMaterialParameterCollection> ParameterCollection = nullptr;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	bool WriteHeightDataToTexture(TArray<uint8>& Data, FIntRect Rect, FString LandscapeActorName);


	TArray<uint16> HeightData;
	TArray<uint8> Pixels;
	FIntRect SampleRect;
	
	int32 MaxHeight = 0;

	
	virtual void OnRegister() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};