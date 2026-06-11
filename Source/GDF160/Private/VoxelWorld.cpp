#include "VoxelWorld.h"

#include "Kismet/GameplayStatics.h"
#include "FastNoiseLite/FastNoiseLite.h"

AVoxelWorld::AVoxelWorld()
{
	PrimaryActorTick.bCanEverTick = true;
	
	USceneComponent* RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	
	SetRootComponent(RootComponent);
}

void 
AVoxelWorld::BeginPlay()
{
	Super::BeginPlay();
	
	InitPlayer();
	InitSectorCache();
	
	GenerateWorld();
	BuildSectorMeshes();
	
	UpdateSectorComponents();
}

void 
AVoxelWorld::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (PlayerPawn)
	{
		const FVector PlayerLocation = PlayerPawn->GetActorLocation();
		const FIntVector2 SectorCoordinate = WorldLocationToSectorCoordinate(PlayerLocation);
		
		if (SectorCoordinate != PlayerSectorCoordinate)
		{
			PlayerSectorCoordinate = SectorCoordinate;
			
			UpdateSectorComponents();
		}
	}
}

bool
AVoxelWorld::CellCoordinateIsValid(const FIntVector& CellCoordinate)
{
	return (
		CellCoordinate.X >= 0 && CellCoordinate.X < WorldSizeInCellsX &&
		CellCoordinate.Y >= 0 && CellCoordinate.Y < WorldSizeInCellsY &&
		CellCoordinate.Z >= 0 && CellCoordinate.Z < WorldSizeInCellsZ
	);
}

bool 
AVoxelWorld::SectorCoordinateIsValid(const FIntVector2& SectorCoordinate)
{
	return (
		SectorCoordinate.X >= 0 && SectorCoordinate.X < WorldSizeInSectorsX &&
		SectorCoordinate.Y >= 0 && SectorCoordinate.Y < WorldSizeInSectorsY
	);
}

FIntVector2
AVoxelWorld::SectorIndexToSectorCoordinate(uint32 SectorIndex)
{
	return {
		static_cast<int32>(SectorIndex % WorldSizeInSectorsX),
		static_cast<int32>(SectorIndex / WorldSizeInSectorsX),
	};
}

int32
AVoxelWorld::SectorCoordinateToSectorIndex(const FIntVector2& SectorCoordinate)
{
	return SectorCoordinate.X + SectorCoordinate.Y * WorldSizeInSectorsX;
}

int32
AVoxelWorld::CellCoordinateToSectorIndex(const FIntVector& CellCoordinate)
{
	const FIntVector2 SectorCoordinate = {
		CellCoordinate.X >> SectorSizeInCellsXLog2,
		CellCoordinate.Y >> SectorSizeInCellsYLog2,
	};
	
	return SectorCoordinate.X + SectorCoordinate.Y * WorldSizeInSectorsX;
}

int32
AVoxelWorld::CellCoordinateToCellIndex(const FIntVector& CellCoordinate)
{
	return CellCoordinate.X * WorldStrideX + CellCoordinate.Y * WorldStrideY + CellCoordinate.Z * WorldStrideZ;
}

FIntVector 
AVoxelWorld::SectorCoordinateToCellCoordinate(const FIntVector2& SectorCoordinate)
{
	return {
		SectorCoordinate.X * SectorSizeInCellsX,
		SectorCoordinate.Y * SectorSizeInCellsY,
		0,
	};
}

FIntVector 
AVoxelWorld::CellIndexToCellCoordinate(int32 CellIndex)
{
	const int32 CellZ = CellIndex / WorldStrideZ;
	CellIndex -= CellZ * WorldStrideZ;

	const int32 CellY = CellIndex / WorldStrideY;
	CellIndex -= CellY * WorldStrideY;

	const int32 CellX = CellIndex / WorldStrideX;

	return { CellX, CellY, CellZ };
}

FIntVector 
AVoxelWorld::WorldLocationToCellCoordinate(const FVector& WorldLocation)
{
	return {
		FMath::FloorToInt32(WorldLocation.X / CellSizeInCentimeters),
		FMath::FloorToInt32(WorldLocation.Y / CellSizeInCentimeters),
		FMath::FloorToInt32(WorldLocation.Z / CellSizeInCentimeters),
	};
}

FIntVector2
AVoxelWorld::WorldLocationToSectorCoordinate(const FVector& WorldLocation)
{
	const FIntVector CellCoordinate = WorldLocationToCellCoordinate(WorldLocation);

	return {
		CellCoordinate.X >> SectorSizeInCellsXLog2,
		CellCoordinate.Y >> SectorSizeInCellsYLog2,
	};
}

FCell& 
AVoxelWorld::GetCell(const FIntVector& CellCoordinate)
{
	const int32 CellIndex = CellCoordinateToCellIndex(CellCoordinate);
	
	return CellArray[CellIndex];
}

void
AVoxelWorld::InitPlayer()
{
	const FVector StartLocation = {
		WorldSizeInCellsX / 2.0f * CellSizeInCentimeters,
		WorldSizeInCellsY / 2.0f * CellSizeInCentimeters,
		WorldSizeInCellsZ / 2.0f * CellSizeInCentimeters,
	};
	
	PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	PlayerPawn->SetActorLocation(StartLocation);

	PlayerSectorCoordinate = { -1, -1 };
	
	SetActorLocation(FVector::ZeroVector);
}

void 
AVoxelWorld::InitSectorCache()
{
	const int32 SectorCacheDiameter = 2 * SectorViewRange + 1;
	const int32 SectorCacheSize = SectorCacheDiameter * SectorCacheDiameter;
	
	for (int32 CacheIndex = 0; CacheIndex < SectorCacheSize; CacheIndex++)
	{
		const FString ComponentName = FString::Printf(TEXT("SectorMesh_%d"), FreeSectorComponentArray.Num() + 1);
				
		USectorComponent* SectorComponent = NewObject<USectorComponent>(this, *ComponentName);
			
		SectorComponent->AttachToComponent(
			RootComponent.Get(),
			FAttachmentTransformRules::KeepRelativeTransform
		);
			
		SectorComponent->SetMaterial(0, BlockMaterial);
			
		FreeSectorComponentArray.Add(SectorComponent);
	}
}

void 
AVoxelWorld::GenerateWorld()
{
	CellArray.SetNumUninitialized(WorldVolumeInCells);
	
	FastNoiseLite HeightNoise;
	HeightNoise.SetSeed(1337);
	HeightNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
	HeightNoise.SetFrequency(1.0f / TerrainNoisePeriod);
	HeightNoise.SetFractalType(FastNoiseLite::FractalType_FBm);
	HeightNoise.SetFractalOctaves(3);
	HeightNoise.SetFractalLacunarity(3.0f);
	HeightNoise.SetFractalGain(0.7f);
	
	const int32 BlockKindCount = static_cast<int32>(EBlockKind::Count);
	
	for (int32 CellIndex = 0; CellIndex < CellArray.Num(); CellIndex++)
	{
		const FIntVector CellCoordinate = CellIndexToCellCoordinate(CellIndex);
		
		const float NoiseValue = HeightNoise.GetNoise(
			static_cast<float>(CellCoordinate.X),
			static_cast<float>(CellCoordinate.Y)
		);
		
		const float NoiseValueNormalized = (NoiseValue + 1.0f) * 0.5f;
		
		const int32 TerrainHeight = FMath::RoundToInt32(
			FMath::Lerp(
				static_cast<float>(TerrainHeightMin),
				static_cast<float>(TerrainHeightMax),
				NoiseValueNormalized
			)
		);
		
		FCell& Cell = CellArray[CellIndex];
		
		Cell.CellIndex = CellIndex;
		
		if (CellCoordinate.Z <= TerrainHeight)
		{
			const int32 BlockKindIndex = FMath::RandRange(1, BlockKindCount - 1);
			
			Cell.BlockKind = static_cast<EBlockKind>(BlockKindIndex);
		}
		else
		{
			Cell.BlockKind = EBlockKind::None;
		}
	}
	
	for (FCell& Cell : CellArray)
	{
		Cell.NeighborSet = CalculateNeighborSet(Cell);
	}
}

uint8
AVoxelWorld::CalculateNeighborSet(const FCell& Cell)
{
	uint8 NeighborSet = 0;
	
	for (const ECartesianDirection Direction : TEnumRange<ECartesianDirection>())
	{
		const int32 DirectionIndex = static_cast<int32>(Direction);
		const FIntVector DirectionOffset = CartesianDirectionOffsets[DirectionIndex];
		
		const FIntVector TestCellCoordinate = CellIndexToCellCoordinate(Cell.CellIndex) + DirectionOffset;
		
		if (CellCoordinateIsValid(TestCellCoordinate))
		{
			const FCell& TestBlock = GetCell(TestCellCoordinate);
			
			if (TestBlock.BlockKind != EBlockKind::None)
			{
				NeighborSet |= (1 << DirectionIndex);
			}
		}
	}
	
	return NeighborSet;
}

void 
AVoxelWorld::BuildSectorMeshes()
{
	SectorMeshArray.Empty();
	
	for (int32 SectorIndex = 0; SectorIndex < WorldAreaInSectors; SectorIndex++)
	{
		FSectorMesh SectorMesh = BuildSectorMesh(SectorIndex);
		
		SectorMeshArray.Add(SectorMesh);
	}
}

FSectorMesh
AVoxelWorld::BuildSectorMesh(const int32 SectorIndex)
{
	FSectorMesh SectorMesh = {
		.SectorIndex = SectorIndex,
		.SectorFaceArray = TArray<FSectorFace>(),
	};
	
	const FIntVector2 SectorCoordinate = SectorIndexToSectorCoordinate(SectorIndex);
	const FIntVector SectorCellCoordinate = SectorCoordinateToCellCoordinate(SectorCoordinate);
	
	for (int32 CellZ = 0; CellZ < SectorSizeInCellsZ; CellZ++)
	{
		for (int32 CellY = SectorCellCoordinate.Y; CellY < SectorCellCoordinate.Y + SectorSizeInCellsY; CellY++)
		{
			for (int32 CellX = SectorCellCoordinate.X; CellX < SectorCellCoordinate.X + SectorSizeInCellsX; CellX++)
			{
				const FIntVector CellCoordinate = { CellX, CellY, CellZ };
				
				if (!CellCoordinateIsValid(CellCoordinate))
				{
					continue;
				}
				
				const FCell& Cell = GetCell(CellCoordinate);

				if (Cell.BlockKind == EBlockKind::None)
				{
					continue;
				}
				
				for (const ECartesianDirection& Direction : TEnumRange<ECartesianDirection>())
				{
					const int32 DirectionIndex = static_cast<int32>(Direction);
					
					if ((Cell.NeighborSet & (1 << DirectionIndex)) == 0)
					{
						const FSectorFace SectorFace = {
							.BlockKind = Cell.BlockKind,
							.Direction = Direction,
							.CellCoordinate = CellCoordinate,
						};
						
						SectorMesh.SectorFaceArray.Add(SectorFace);
					}
				}
			}
		}
	}
	
	return SectorMesh;
}

void 
AVoxelWorld::UpdateSectorComponents()
{
	if (BlockMaterial == nullptr)
	{
		return;
	}
	
	TSet<FIntVector2> SectorCoordinateSetCurrent;
	TSet<FIntVector2> SectorCoordinateSetNext;
	
	SectorComponentMap.GetKeys(SectorCoordinateSetCurrent);
	
	for (int32 SectorDeltaY = -SectorViewRange; SectorDeltaY <= SectorViewRange; SectorDeltaY++)
	{
		for (int32 SectorDeltaX = -SectorViewRange; SectorDeltaX <= SectorViewRange; SectorDeltaX++)
		{
			const FIntVector2 SectorDelta = { SectorDeltaX, SectorDeltaY };
			const FIntVector2 SectorCoordinate = PlayerSectorCoordinate + SectorDelta; 
			
			if (SectorCoordinateIsValid(SectorCoordinate))
			{
				SectorCoordinateSetNext.Add(SectorCoordinate);
			}
		}
	}
	
	TSet<FIntVector2> SectorsToRemove = SectorCoordinateSetCurrent.Difference(SectorCoordinateSetNext);
	TSet<FIntVector2> SectorsToAdd = SectorCoordinateSetNext.Difference(SectorCoordinateSetCurrent);
	
	for (const FIntVector2 SectorCoordinate : SectorsToRemove)
	{
		RemoveSectorComponent(SectorCoordinate);
	}
	
	for (const FIntVector2 SectorCoordinate : SectorsToAdd)
	{
		AddSectorComponent(SectorCoordinate);
	}
}

void
AVoxelWorld::RemoveSectorComponent(const FIntVector2& SectorCoordinate)
{
	const TObjectPtr<USectorComponent>* FoundComponent = SectorComponentMap.Find(SectorCoordinate);
		
	if (FoundComponent)
	{
		USectorComponent* SectorComponent = *FoundComponent;
			
		SectorComponent->SetVisibility(false);
		SectorComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		SectorComponent->UnregisterComponent();
			
		FreeSectorComponentArray.Add(SectorComponent);
		SectorComponentMap.Remove(SectorCoordinate);
	}
}

void
AVoxelWorld::AddSectorComponent(const FIntVector2& SectorCoordinate)
{
	const int32 SectorIndex = SectorCoordinateToSectorIndex(SectorCoordinate);
	const FSectorMesh& SectorMesh = SectorMeshArray[SectorIndex];
	
	USectorComponent* SectorComponent = FreeSectorComponentArray.Pop();
	
	FDynamicMesh3 DynamicMesh = USectorComponent::BuildDynamicMesh(SectorMesh);
	
	SectorComponent->RegisterComponent();
	SectorComponent->SetVisibility(true);
	SectorComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	SectorComponent->SetCollisionObjectType(ECC_WorldStatic);
	SectorComponent->SetCollisionResponseToAllChannels(ECR_Block);
	SectorComponent->SetComplexAsSimpleCollisionEnabled(true, true);
	SectorComponent->GetDynamicMesh()->SetMesh(MoveTemp(DynamicMesh));
	SectorComponent->NotifyMeshUpdated();
		
	SectorComponentMap.Add(SectorCoordinate, SectorComponent);
}
