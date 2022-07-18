// Fill out your copyright notice in the Description page of Project Settings.


#include "TerrainComponent.h"

#include "Landscape.h"
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

		UE_LOG(LogTemp, Warning, TEXT("Min: %d, %d    || Max: %d, %d"), MinX, MinY, MaxX, MaxY);

		FIntRect SampleRect = FIntRect(MinX, MinY, 1 + MaxX - MinX, 1 + MaxY - MinY);

		TArray<uint16> HeightData;
		TArray<uint8> Pixels;

		HeightData.Reserve(SampleRect.Width() * SampleRect.Height());
		Pixels.Reserve(SampleRect.Width() * SampleRect.Height() * 4);

		UE_LOG(LogTemp, Warning, TEXT("Width: %d    || Height: %d"), SampleRect.Width(), SampleRect.Height());

		for (float i = 0; i < SampleRect.Height(); i++)
		{
			for (float j = 0; j < SampleRect.Width(); j++)
			{
				float X = j / ((float)SampleRect.Width());
				float Y = i / ((float)SampleRect.Height());

				float PerlinValue = 1;
				float OctavesScale = 1;
				float TempFrequency = Frequency;

				for (int k = 1; k <= Octaves; k++)
				{
					float NoiseValue = FMath::PerlinNoise2D(FVector2D(TempFrequency * X, TempFrequency * Y));
					//UE_LOG(LogTemp, Warning, TEXT("X: %f  |   Y: %f       | NoiseValue: %f"), X, Y, NoiseValue);
					PerlinValue += NoiseValue * OctavesScale;

					TempFrequency *= 2;
					OctavesScale /= 2;
				}

				uint16 HeightValue = PerlinValue * Elevation;

				HeightData.Add(HeightValue);

				FColor Color = FLinearColor::LerpUsingHSV(FLinearColor(FColor::Black),
					FLinearColor(0.6,0.6,0.6), PerlinValue).ToFColor(true);
				//UE_LOG(LogTemp, Warning, TEXT("Color R: %d, G: %d, B: %d, A: %d"), Color.R, Color.G, Color.B, Color.A);
				//UE_LOG(LogTemp, Warning, TEXT("PerlinValue: %f"), PerlinValue);
				Pixels.Add(Color.R);
				Pixels.Add(Color.G);
				Pixels.Add(Color.B);
				Pixels.Add(Color.A);
			}
		}

		bool bSaved = WriteHeightDataToTexture(Pixels, SampleRect);
		UE_LOG(LogTemp, Warning, TEXT("Writing data to texture %s"), (bSaved ? TEXT("finished with success") : TEXT("failed")));

		FHeightmapAccessor<false> HeightmapAccessor(LandscapeInfo);
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
	NewTexture->GetPlatformData()->PixelFormat = PF_B8G8R8A8;

	FTexture2DMipMap* Mip = new FTexture2DMipMap();
	NewTexture->GetPlatformData()->Mips.Add(Mip);
	Mip->SizeX = SampleRect.Width();
	Mip->SizeY = SampleRect.Height();

	// Lock the texture so it can be modified
	Mip->BulkData.Lock(LOCK_READ_WRITE);
	uint8* TextureData = (uint8*)Mip->BulkData.Realloc(HeightData.Num());
	FMemory::Memcpy(TextureData, HeightData.GetData(), sizeof(uint8) * HeightData.Num());
	Mip->BulkData.Unlock();

	NewTexture->Source.Init(512, 512, 1, 1, TSF_RGBA8, HeightData.GetData());

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
		if (PropertyChangedEvent.Property->GetFName() == FName("Frequency") ||
			PropertyChangedEvent.Property->GetFName() == FName("Octaves") ||
			PropertyChangedEvent.Property->GetFName() == FName("Elevation"))
		{
			GenerateLandmass();
		}
	}
}
#endif

/*
void UTerrainComponent::GenerateLandmass()
{
	if (GetWorld())
	{
		ALandscape* Landscape = Cast<ALandscape>(UGameplayStatics::GetActorOfClass(GetWorld(), ALandscape::StaticClass()));

		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, Landscape->GetName());

		int32 MinX, MinY, MaxX, MaxY;
		ULandscapeInfo* LandscapeInfo = Landscape->GetLandscapeInfo();

		LandscapeInfo->GetLandscapeExtent(MinX, MinY, MaxX, MaxY);

		FIntRect SampleRect = FIntRect(0, 0, 1 + MaxX - MinX, 1 + MaxY - MinY);

		TArray<uint16> HeightData;


		for (float i = SampleRect.Min.Y; i < SampleRect.Max.Y; i++)
		{
			for (float j = SampleRect.Min.X; j < SampleRect.Max.X; j++)
			{
				float X = j / ((float)SampleRect.Width());
				float Y = i / ((float)SampleRect.Height());

				float PerlinValue = 1;
				float OctavesScale = 1;
				float TempFrequency = Frequency;

				for (int k = 1; k <= Octaves; k++)
				{
					float NoiseValue = FMath::PerlinNoise2D(FVector2D(TempFrequency * X, TempFrequency * Y));

					PerlinValue += NoiseValue * OctavesScale;

					TempFrequency *= 2;
					OctavesScale /= 2;
				}
				
				HeightData.Add(FMath::Abs(PerlinValue * MaxHeight));
			}
		}

		FHeightmapAccessor<false> HeightmapAccessor(LandscapeInfo);
		HeightmapAccessor.SetData(MinX, MinY, SampleRect.Width() - 1, SampleRect.Height() - 1, HeightData.GetData());
	}
}
*/
