#include "VoxelWorld.h"

#include "BlockKind.h"
#include "CartesianDirection.h"
#include "Constants.h"
#include "SectorMesh.h"
#include "Kismet/GameplayStatics.h"

AVoxelWorld::AVoxelWorld()
{
	PrimaryActorTick.bCanEverTick = true;
	
	USceneComponent* RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	
	SetRootComponent(RootComponent);
}

void 
AVoxelWorld::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
}

void 
AVoxelWorld::BeginPlay()
{
	Super::BeginPlay();
	
	const FVector StartLocation = FVector(
		WorldSizeInCellsX / 2.0f * CellSizeInCentimeters,
		WorldSizeInCellsY / 2.0f * CellSizeInCentimeters,
		2000.0
	);
	
	PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	
	PlayerPawn->SetActorLocation(StartLocation);

	PlayerSectorCoordinate = { -1, -1 };
	
	SetActorLocation(FVector::ZeroVector);
	
	GenerateWorld();
	
	BuildSectorMeshes();
	
	const int32 SectorCacheSize = FMath::Pow(2.0f * SectorViewRange + 1, 2);
	
	for (int32 CacheIndex = 0; CacheIndex < SectorCacheSize; CacheIndex++)
	{
		const FString ComponentName = FString::Printf(TEXT("SectorMesh_%d"), FreeSectorComponentArray.Num() + 1);
			
		USectorComponent* SectorComponent = NewObject<USectorComponent>(this, *ComponentName);
		
		SectorComponent->AttachToComponent(
			RootComponent,
			FAttachmentTransformRules::KeepRelativeTransform
		);
		
		FreeSectorComponentArray.Add(SectorComponent);
	}
	
	BuildSectorComponents();
}

void 
AVoxelWorld::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (PlayerPawn)
	{
		const FVector PlayerLocation = PlayerPawn->GetActorLocation();
		
		const FIntVector2 SectorCoordinate = WorldLocationToSectorCoordinate(PlayerLocation);
		
		if (PlayerSectorCoordinate != SectorCoordinate)
		{
			PlayerSectorCoordinate = SectorCoordinate;
			
			BuildSectorComponents();
		}
	}
}

bool
AVoxelWorld::CellCoordinateIsValid(FIntVector3 CellCoordinate)
{
	return (
		CellCoordinate.X >= 0 && CellCoordinate.X < WorldSizeInCellsX &&
		CellCoordinate.Y >= 0 && CellCoordinate.Y < WorldSizeInCellsY &&
		CellCoordinate.Z >= 0 && CellCoordinate.Z < WorldSizeInCellsZ
	);
}

bool 
AVoxelWorld::SectorCoordinateIsValid(FIntVector2 SectorCoordinate)
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
AVoxelWorld::SectorCoordinateToSectorIndex(const FIntVector2 SectorCoordinate)
{
	return SectorCoordinate.X + SectorCoordinate.Y * WorldSizeInSectorsX;
}

int32
AVoxelWorld::CellCoordinateToSectorIndex(FIntVector3 CellCoordinate)
{
	const FIntVector2 SectorCoordinate = {
		CellCoordinate.X >> SectorSizeInCellsXLog2,
		CellCoordinate.Y >> SectorSizeInCellsYLog2,
	};
	
	return SectorCoordinate.X + SectorCoordinate.Y * WorldSizeInSectorsX;
}

int32
AVoxelWorld::CellCoordinateToCellIndex(FIntVector3 CellCoordinate)
{
	const uint32 WorldStrideX = 1;
	const uint32 WorldStrideY = WorldSizeInSectorsX * SectorSizeInCellsX;
	const uint32 WorldStrideZ = WorldStrideY * (WorldSizeInSectorsY * SectorSizeInCellsY);
	
	return CellCoordinate.X * WorldStrideX + CellCoordinate.Y * WorldStrideY + CellCoordinate.Z * WorldStrideZ;
}

FIntVector3 
AVoxelWorld::SectorCoordinateToCellCoordinate(FIntVector2 SectorCoordinate)
{
	return {
		SectorCoordinate.X * SectorSizeInCellsX,
		SectorCoordinate.Y * SectorSizeInCellsY,
		0,
	};
}

FIntVector3 
AVoxelWorld::CellIndexToCellCoordinate(int32 CellIndex)
{
	const int32 WorldStrideY = WorldSizeInSectorsX * SectorSizeInCellsX;
	const int32 WorldStrideZ = WorldSizeInSectorsX * SectorSizeInCellsX * WorldSizeInSectorsY * SectorSizeInCellsY;
	
	const int32 CellZ = CellIndex / WorldStrideZ;

	CellIndex -= CellZ * WorldStrideZ;

	const int32 CellY = CellIndex / WorldStrideY;

	CellIndex -= CellY * WorldStrideY;

	const int32 CellX = CellIndex;

	return { CellX, CellY, CellZ };
}

FIntVector3 
AVoxelWorld::WorldLocationToCellCoordinate(FVector WorldLocation)
{
	return {
		FMath::FloorToInt32(WorldLocation.X / CellSizeInCentimeters),
		FMath::FloorToInt32(WorldLocation.Y / CellSizeInCentimeters),
		FMath::FloorToInt32(WorldLocation.Z / CellSizeInCentimeters),
	};
}

FIntVector2
AVoxelWorld::WorldLocationToSectorCoordinate(FVector WorldLocation)
{
	const FIntVector CellCoordinate = WorldLocationToCellCoordinate(WorldLocation);

	return {
		CellCoordinate.X >> SectorSizeInCellsXLog2,
		CellCoordinate.Y >> SectorSizeInCellsYLog2,
	};
}


FCell& 
AVoxelWorld::GetCell(FIntVector3 CellCoordinate)
{
	const int32 CellIndex = CellCoordinateToCellIndex(CellCoordinate);
	
	return CellArray[CellIndex];
}

void 
AVoxelWorld::GenerateWorld()
{
	CellArray.SetNumUninitialized(WorldVolumeInCells);
	
	const int32 BlockKindCount = static_cast<int32>(EBlockKind::Count);
	
	for (int32 CellIndex = 0; CellIndex < CellArray.Num(); CellIndex++)
	{
		const EBlockKind BlockKind = static_cast<EBlockKind>(FMath::RandRange(1, BlockKindCount - 1));
	
		FCell& Cell = CellArray[CellIndex];
	
		Cell.CellIndex = CellIndex;
		Cell.BlockKind = BlockKind;
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
		const FIntVector3 DirectionOffset = DirectionOffsets[DirectionIndex];
		
		const FIntVector3 TestCellPosition = CellIndexToCellCoordinate(Cell.CellIndex) + DirectionOffset;
		
		if (CellCoordinateIsValid(TestCellPosition))
		{
			const FCell& TestBlock = GetCell(TestCellPosition);
			
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
	const FIntVector3 SectorCellCoordinate = SectorCoordinateToCellCoordinate(SectorCoordinate);
	
	for (int32 CellZ = 0; CellZ < SectorSizeInCellsZ; CellZ++)
	{
		for (int32 CellY = SectorCellCoordinate.Y; CellY < SectorCellCoordinate.Y + SectorSizeInCellsY; CellY++)
		{
			for (int32 CellX = SectorCellCoordinate.X; CellX < SectorCellCoordinate.X + SectorSizeInCellsX; CellX++)
			{
				const FIntVector3 CellCoordinate = { CellX, CellY, CellZ };
				
				if (CellCoordinateIsValid(CellCoordinate))
				{
					const FCell& Cell = GetCell(CellCoordinate);

					if (Cell.BlockKind == EBlockKind::None)
					{
						continue;
					}
					
					for (int32 DirectionIndex = 0; DirectionIndex < static_cast<int32>(ECartesianDirection::Count); DirectionIndex++)
					{
						if ((Cell.NeighborSet & (1 << DirectionIndex)) == 0)
						{
							FSectorFace SectorFace = {
								.BlockKind = Cell.BlockKind,
								.Direction = static_cast<ECartesianDirection>(DirectionIndex),
								.CellCoordinate = CellCoordinate,
							};
							
							SectorMesh.SectorFaceArray.Add(SectorFace);
						}
					}
				}
			}
		}
	}
	
	return SectorMesh;
}

void 
AVoxelWorld::BuildSectorComponents()
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
		TObjectPtr<USectorComponent>* FoundComponent = SectorComponentMap.Find(SectorCoordinate);
		
		if (FoundComponent && *FoundComponent)
		{
			USectorComponent* SectorComponent = *FoundComponent;
			
			SectorComponent->SetVisibility(false);
			SectorComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			
			SectorComponent->UnregisterComponent();
			
			FreeSectorComponentArray.Add(SectorComponent);
			SectorComponentMap.Remove(SectorCoordinate);
		}
	}
	
	for (const FIntVector2 SectorCoordinate : SectorsToAdd)
	{
		const int32 SectorIndex = SectorCoordinateToSectorIndex(SectorCoordinate);
		const FSectorMesh& SectorMesh = SectorMeshArray[SectorIndex];
		
		USectorComponent* SectorComponent = FreeSectorComponentArray.Pop();
		
		SectorComponent->RegisterComponent();

		SectorComponent->SetVisibility(true);
		SectorComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		SectorComponent->SetCollisionObjectType(ECC_WorldStatic);
		SectorComponent->SetCollisionResponseToAllChannels(ECR_Block);

		SectorComponent->SetComplexAsSimpleCollisionEnabled(true, true);
			
		SectorComponent->SetMaterial(0, BlockMaterial);
		
		FDynamicMesh3 DynamicMesh = USectorComponent::BuildDynamicMesh(SectorMesh);
		SectorComponent->GetDynamicMesh()->SetMesh(MoveTemp(DynamicMesh));
		SectorComponent->NotifyMeshUpdated();
		
		SectorComponentMap.Add(SectorCoordinate, SectorComponent);
	}
	

}
