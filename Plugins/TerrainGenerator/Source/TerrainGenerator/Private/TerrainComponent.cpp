// Fill out your copyright notice in the Description page of Project Settings.


#include "TerrainComponent.h"

#include "Landscape.h"
#include "LandscapeDataAccess.h"
#include "LandscapeEdit.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Kismet/GameplayStatics.h"
#include "Rendering/Texture2DResource.h"
#include "UObject/SavePackage.h"

// Sets default values for this component's properties
UTerrainComponent::UTerrainComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}

void UTerrainComponent::GenerateLandmass()
{
	check(GetWorld());

	if (ALandscape* Landscape = Cast<ALandscape>(GetOwner()))
	{
		int32 MinX, MinY, MaxX, MaxY;
		ULandscapeInfo* LandscapeInfo = Landscape->GetLandscapeInfo();

		LandscapeInfo->GetLandscapeExtent(MinX, MinY, MaxX, MaxY);

		FIntRect SampleRect = FIntRect(MinX, MinY, 1 + MaxX - MinX, 1 + MaxY - MinY);

		TArray<uint16> HeightData;
		TArray<uint8> Pixels;

		HeightData.Init(0, SampleRect.Width() * SampleRect.Height());
		Pixels.Init(0, SampleRect.Width() * SampleRect.Height() * 4);

		
		ParallelFor(SampleRect.Width() * SampleRect.Height(), [&](int32 F)
		{
			int32 i = F % SampleRect.Width() + Seed;
			int32 j = F / SampleRect.Height() + Seed;
		
			float X = i / static_cast<float>(SampleRect.Width());
			float Y = j / static_cast<float>(SampleRect.Height());

			float NoiseValue = 0;
			float AmplitudeSum = 0;
			
			
			for (int k = 0; k < Octaves; k++)
			{
				const float Multiplier =  FMath::Pow(Persistence, k);

				const float PerlinValue = FMath::GetMappedRangeValueClamped(FFloatRange(-1, 1), FFloatRange(0, 1),FMath::PerlinNoise2D(FVector2D((Frequency * Multiplier) * X, (Frequency * Multiplier) * Y)));

				NoiseValue += PerlinValue * (1 / Multiplier);
				AmplitudeSum += (1 / Multiplier);
				
			}
			
			if(bWriteToLog)
				UE_LOG(LogTemp, Warning, TEXT("X: %f  |   Y: %f       | NoiseValue: %f	"), X, Y, (NoiseValue / AmplitudeSum));
			
			HeightData[F] = (bUseCurves && TerrainControlCurve) ? (TerrainControlCurve->GetFloatValue((NoiseValue / AmplitudeSum)) / 100.f) * (Elevation + SHRT_MAX) : FMath::Clamp((Elevation + SHRT_MAX) * (NoiseValue / AmplitudeSum), 0, UINT16_MAX);

			FColor Color = FLinearColor::LerpUsingHSV(FLinearColor(FColor::Black),
				FLinearColor(FColor::White), (NoiseValue / AmplitudeSum)).ToFColor(false);

			Pixels[(F * 4) + 0] = Color.R;
			Pixels[(F * 4) + 1] = Color.G;
			Pixels[(F * 4) + 2] = Color.B;
			Pixels[(F * 4) + 3] = 255;
		});


		//Save height to the texture
		WriteHeightDataToTexture(Pixels, SampleRect);
		
		//Apply height to the landscape
		FHeightmapAccessor<true> HeightmapAccessor(LandscapeInfo);
		HeightmapAccessor.SetData(MinX, MinY, MaxX, MaxY, HeightData.GetData());
	}
}

// Called when the game starts
void UTerrainComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
}

bool UTerrainComponent::WriteHeightDataToTexture(TArray<uint8> HeightData, FIntRect SampleRect)
{
	FString Path = "/TerrainGenerator/";
	FString TextureName = "HeightMap";
	Path += TextureName;

	UPackage* Package = CreatePackage(*Path);
	Package->FullyLoad();

	UTexture2D* NewTexture = NewObject<UTexture2D>(Package, *TextureName, RF_Public | RF_Standalone | RF_MarkAsRootSet);
	NewTexture->AddToRoot(); // This line prevents garbage collection of the texture
	NewTexture->SetPlatformData(new FTexturePlatformData()); // Then we initialize the PlatformData
	NewTexture->GetPlatformData()->SizeX = SampleRect.Width();
	NewTexture->GetPlatformData()->SizeY = SampleRect.Height();
	NewTexture->GetPlatformData()->SetNumSlices(1);
	NewTexture->GetPlatformData()->PixelFormat = PF_R8G8B8A8;

	FTexture2DMipMap* Mip = new FTexture2DMipMap();
	NewTexture->GetPlatformData()->Mips.Add(Mip);
	Mip->SizeX = SampleRect.Width();
	Mip->SizeY = SampleRect.Height();

	// Lock the texture so it can be modified
	Mip->BulkData.Lock(LOCK_READ_WRITE);
	uint8* TextureData = (uint8*)Mip->BulkData.Realloc(HeightData.Num());
	FMemory::Memcpy(TextureData, HeightData.GetData(), sizeof(uint8) * HeightData.Num());
	Mip->BulkData.Unlock();

	NewTexture->Source.Init(SampleRect.Width(), SampleRect.Height(), 1, 1, TSF_RGBA8, HeightData.GetData());

	NewTexture->UpdateResource();
	Package->MarkPackageDirty();
	FAssetRegistryModule::AssetCreated(NewTexture);

	FString PackageFileName = FPackageName::LongPackageNameToFilename(Path, FPackageName::GetAssetPackageExtension());
	FSavePackageArgs SavePackageArgs;
	SavePackageArgs.SaveFlags = RF_Public | RF_Standalone;
	SavePackageArgs.Error = GError;
	SavePackageArgs.bForceByteSwapping = true;
	SavePackageArgs.bWarnOfLongFilename = SAVE_NoError;

	return UPackage::SavePackage(Package, NewTexture, *PackageFileName, SavePackageArgs);
}

#if WITH_EDITOR
void UTerrainComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property && bUpdateOnValuesChanged)
	{
		if (PropertyChangedEvent.Property->GetFName() == FName("Seed")			||
			PropertyChangedEvent.Property->GetFName() == FName("Frequency")		||
			PropertyChangedEvent.Property->GetFName() == FName("Octaves")		||
			PropertyChangedEvent.Property->GetFName() == FName("Persistence")	||
			PropertyChangedEvent.Property->GetFName() == FName("Elevation")		)
		{
			GenerateLandmass();
		}
	}
}
#endif
