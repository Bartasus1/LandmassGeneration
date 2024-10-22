// Fill out your copyright notice in the Description page of Project Settings.


#include "TerrainComponent.h"
#include "Landscape.h"
#include "LandscapeDataAccess.h"
#include "LandscapeEdit.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Materials/MaterialParameterCollectionInstance.h"
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
	if(!GetWorld())
	{
		return;
	}
	
	if (ALandscape* Landscape = Cast<ALandscape>(GetOwner()))
	{
		MaxHeight = 0;
		
		ParallelFor(SampleRect.Width() * SampleRect.Height(), [&](int32 F)
		{
			HeightData[F] = SHRT_MAX;
			
			for(const FWeightedTerrainModifier TerrainWeight : TerrainModifiers)
			{
				if(UTerrainModifier* TerrainModifier = TerrainWeight.TerrainModifier)
				{
					int32 i = F % SampleRect.Width();
					int32 j = F / SampleRect.Height();
				
					float X = (i + TerrainModifier->XOffset) / static_cast<float>(SampleRect.Width());
					float Y = (j + TerrainModifier->YOffset) / static_cast<float>(SampleRect.Height());

			
					const float Height = TerrainModifier->GetTerrainHeightValue(X, Y) * TerrainWeight.Weight;
				
					HeightData[F] = FMath::Clamp( HeightData[F] + TerrainModifier->Elevation * Height, 0, UINT16_MAX);

					if(bGenerateAdditionalHeightmap)
					{
						const FColor Color = FLinearColor::LerpUsingHSV(FLinearColor(FColor::Black), FLinearColor(FColor::White), (float)HeightData[F] / UINT16_MAX).ToFColor(false);
				
						Pixels[(F * 4) + 0] = Color.R;
						Pixels[(F * 4) + 1] = Color.G;
						Pixels[(F * 4) + 2] = Color.B;
						Pixels[(F * 4) + 3] = 255;
					}
				
					MaxHeight = FMath::Max(MaxHeight, static_cast<int32>(HeightData[F]));
				}
			}
		});

		MaxHeight = (LANDSCAPE_ZSCALE * Landscape->GetActorScale3D().Z * MaxHeight) / 2;
		
		if(ParameterCollection)
		{
			UMaterialParameterCollectionInstance* ParameterCollectionInstance = GetWorld()->GetParameterCollectionInstance(ParameterCollection);
			ParameterCollectionInstance->SetScalarParameterValue("MaxHeight", MaxHeight);
		}

		if(bGenerateAdditionalHeightmap)
		{
			//Save height to the texture
			WriteHeightDataToTexture(Pixels, SampleRect, Landscape->GetActorNameOrLabel());
		}
	
		//Apply height to the landscape
		FHeightmapAccessor<true> HeightmapAccessor(Landscape->GetLandscapeInfo());
		HeightmapAccessor.SetData(SampleRect.Min.X, SampleRect.Min.Y, SampleRect.Width() - 1, SampleRect.Height() - 1, HeightData.GetData());
	}
	
}

void UTerrainComponent::RegenerateLandscapeInfo()
{
	if (ALandscape* Landscape = Cast<ALandscape>(GetOwner()))
	{
		int32 MinX, MinY, MaxX, MaxY;
		ULandscapeInfo* LandscapeInfo = Landscape->GetLandscapeInfo();

		LandscapeInfo->GetLandscapeExtent(MinX, MinY, MaxX, MaxY);
		SampleRect = FIntRect(MinX, MinY, (MaxX - MinX) + 1, (MaxY - MinY) + 1);
		
		HeightData.Init(0, SampleRect.Width() * SampleRect.Height());
		Pixels.Init(0, SampleRect.Width() * SampleRect.Height() * 4);

		UE_LOG(LogTemp, Warning, TEXT("COMPONENT REGISTERED !!!"));
		UE_LOG(LogTemp, Warning, TEXT("LANSCAPE SIZE: \n\tWIDTH: %d \n\tHEIGHT: %d"), SampleRect.Width(), SampleRect.Height());
	}
}

// Called when the game starts
void UTerrainComponent::BeginPlay()
{
	Super::BeginPlay();


    GenerateLandmass();
	// ...
}

bool UTerrainComponent::WriteHeightDataToTexture(TArray<uint8>& Data, FIntRect Rect, FString LandscapeActorName)
{

	FString Path = "/TerrainGenerator/";
	FString TextureName = "HeightMap_" + LandscapeActorName;

	UPackage* Package = CreatePackage(*(Path + TextureName));
	Package->FullyLoad();

	UTexture2D* NewTexture = NewObject<UTexture2D>(Package, *TextureName, RF_Public | RF_Standalone | RF_MarkAsRootSet);
	NewTexture->AddToRoot(); // This line prevents garbage collection of the texture
	NewTexture->SetPlatformData(new FTexturePlatformData()); // Then we initialize the PlatformData
	NewTexture->GetPlatformData()->SizeX = Rect.Width();
	NewTexture->GetPlatformData()->SizeY = Rect.Height();
	NewTexture->GetPlatformData()->SetNumSlices(1);
	NewTexture->GetPlatformData()->PixelFormat = PF_R8G8B8A8;

	FTexture2DMipMap* Mip = new FTexture2DMipMap();
	NewTexture->GetPlatformData()->Mips.Add(Mip);
	Mip->SizeX = Rect.Width();
	Mip->SizeY = Rect.Height();

	// Lock the texture so it can be modified
	Mip->BulkData.Lock(LOCK_READ_WRITE);
	uint8* TextureData = (uint8*)Mip->BulkData.Realloc(Data.Num());
	FMemory::Memcpy(TextureData, Data.GetData(), sizeof(uint8) * Data.Num());
	Mip->BulkData.Unlock();

	NewTexture->Source.Init(Rect.Width(), Rect.Height(), 1, 1, TSF_RGBA8, Data.GetData());

	NewTexture->UpdateResource();
	Package->MarkPackageDirty();
	FAssetRegistryModule::AssetCreated(NewTexture);

	FString PackageFileName = FPackageName::LongPackageNameToFilename(Path, FPackageName::GetAssetPackageExtension());
	FSavePackageArgs SavePackageArgs;
	SavePackageArgs.SaveFlags = RF_Public | RF_Standalone | RF_NeedLoad;
	SavePackageArgs.Error = GError;
	SavePackageArgs.bForceByteSwapping = true;
	SavePackageArgs.bWarnOfLongFilename = SAVE_NoError;

	return UPackage::SavePackage(Package, NewTexture, *PackageFileName, SavePackageArgs);

}

void UTerrainComponent::OnRegister()
{
	Super::OnRegister();

	RegenerateLandscapeInfo();

	static bool FirstTime = true;
	if(FirstTime) // prevent firing OnRegister() in PostEditChangeProperty() - we only want to call it at engine start
	{
		GenerateLandmass();
		FirstTime = false;
	}

}


#if WITH_EDITOR
void UTerrainComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if(UpdateBehaviour == None)
	{
		return;
	}
	
	EPropertyChangeType::Type ChangeBehaviour = EPropertyChangeType::ValueSet;
	
	if(UpdateBehaviour == Interactive)
	{
		ChangeBehaviour = EPropertyChangeType::Interactive;
	}
	
	if (PropertyChangedEvent.ChangeType == ChangeBehaviour)
	{
		GenerateLandmass();
	}
}
#endif
