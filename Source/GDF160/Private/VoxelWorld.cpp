// Fill out your copyright notice in the Description page of Project Settings.


#include "VoxelWorld.h"

#include "BlockKind.h"
#include "CartesianDirection.h"
#include "SectorMesh.h"
#include "DynamicMesh/DynamicMeshAttributeSet.h"
#include "DynamicMesh/DynamicMeshOverlay.h"

// Sets default values
AVoxelWorld::AVoxelWorld()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	USceneComponent* RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	
	SetRootComponent(RootComponent);
}

void 
AVoxelWorld::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
	GenerateWorld();
	
	BuildSectorMeshes();
	BuildSectorComponents();
}

// Called when the game starts or when spawned
void 
AVoxelWorld::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void 
AVoxelWorld::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void 
AVoxelWorld::GenerateWorld()
{
	CellArray.SetNumUninitialized(WorldVolumeInCells);
	
	const int32 BlockKindCount = static_cast<int32>(EBlockKind::Count);
	
	for (int32 CellIndex = 0; CellIndex < CellArray.Num(); CellIndex++)
	{
		const EBlockKind BlockKind = static_cast<EBlockKind>(FMath::RandRange(0, BlockKindCount - 1));
	
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
		SectorMeshArray.Add(BuildSectorMesh(SectorIndex));
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
					FCell& Cell = GetCell(CellCoordinate);

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
	
	SectorComponentMap.Empty();
	
	for (int32 SectorIndex = 0; SectorIndex < WorldAreaInSectors; SectorIndex++)
	{
		const FIntVector2 SectorCoordinate = SectorIndexToSectorCoordinate(SectorIndex);
		
		SectorComponentMap.Add(SectorCoordinate, BuildSectorComponent(SectorIndex));
	}
}

USectorComponent*
AVoxelWorld::BuildSectorComponent(int32 SectorIndex)
{
	const FSectorMesh& SectorMesh = SectorMeshArray[SectorIndex];
	
	const FString ComponentName = FString::Printf(TEXT("SectorMesh_%d"), SectorIndex);

	USectorComponent* SectorComponent = NewObject<USectorComponent>(this, *ComponentName);
		
	SectorComponent->RegisterComponent();
		
	SectorComponent->AttachToComponent(
		RootComponent,
		FAttachmentTransformRules::KeepRelativeTransform
	);
	
	SectorComponent->SetMaterial(0, BlockMaterial);
	
	SectorComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	SectorComponent->SetCollisionObjectType(ECC_WorldStatic);
	SectorComponent->SetCollisionResponseToAllChannels(ECR_Block);
	
	SectorComponent->SetComplexAsSimpleCollisionEnabled(true, true);
	
	FDynamicMesh3 DynamicMesh = BuildDynamicMesh(SectorMesh);
	
	SectorComponent->GetDynamicMesh()->SetMesh(MoveTemp(DynamicMesh));
	
	return SectorComponent;
}

FDynamicMesh3
AVoxelWorld::BuildDynamicMesh(const FSectorMesh& SectorMesh)
{
	FDynamicMesh3 DynamicMesh;
	
	DynamicMesh.EnableAttributes();
	
	for (const FSectorFace& SectorFace : SectorMesh.SectorFaceArray)
	{
		if (SectorFace.BlockKind == EBlockKind::None)
		{
			continue;
		}
		
		const int32 DirectionIndex = static_cast<int32>(SectorFace.Direction);
		
		const FVector3d CellPosition = { 
			static_cast<double>(SectorFace.CellCoordinate[0]), 
			static_cast<double>(SectorFace.CellCoordinate[1]), 
			static_cast<double>(SectorFace.CellCoordinate[2]),
		};
		
		const FVector3d VertexPosition0 = { 
			VoxelVertexArray[DirectionIndex][0][0], 
			VoxelVertexArray[DirectionIndex][0][1], 
			VoxelVertexArray[DirectionIndex][0][2] 
		};
		
		const FVector3d VertexPosition1 = { 
			VoxelVertexArray[DirectionIndex][1][0], 
			VoxelVertexArray[DirectionIndex][1][1], 
			VoxelVertexArray[DirectionIndex][1][2] 
		};
		
		const FVector3d VertexPosition2 = { 
			VoxelVertexArray[DirectionIndex][2][0], 
			VoxelVertexArray[DirectionIndex][2][1], 
			VoxelVertexArray[DirectionIndex][2][2] 
		};
		
		const FVector3d VertexPosition3 = { 
			VoxelVertexArray[DirectionIndex][3][0], 
			VoxelVertexArray[DirectionIndex][3][1], 
			VoxelVertexArray[DirectionIndex][3][2] 
		};
		
		const int32 VertexIndex0 = DynamicMesh.AppendVertex(100.0 * (CellPosition + VertexPosition0));
		const int32 VertexIndex1 = DynamicMesh.AppendVertex(100.0 * (CellPosition + VertexPosition1));
		const int32 VertexIndex2 = DynamicMesh.AppendVertex(100.0 * (CellPosition + VertexPosition2));
		const int32 VertexIndex3 = DynamicMesh.AppendVertex(100.0 * (CellPosition + VertexPosition3));
		
		const int32 TriangleIndex0 = DynamicMesh.AppendTriangle(VertexIndex0, VertexIndex1, VertexIndex2);
		const int32 TriangleIndex1 = DynamicMesh.AppendTriangle(VertexIndex0, VertexIndex2, VertexIndex3);
		
		UE::Geometry::FDynamicMeshUVOverlay* UVOverlay = DynamicMesh.Attributes()->PrimaryUV();
		
		const FVector2f UVPosition0 = { 
			VoxelUVArray[DirectionIndex][0][0] / 4.0f, 
			VoxelUVArray[DirectionIndex][0][1] / 4.0f, 
		};
		
		const FVector2f UVPosition1 = { 
			VoxelUVArray[DirectionIndex][1][0] / 4.0f, 
			VoxelUVArray[DirectionIndex][1][1] / 4.0f, 
		};
		
		const FVector2f UVPosition2 = { 
			VoxelUVArray[DirectionIndex][2][0] / 4.0f, 
			VoxelUVArray[DirectionIndex][2][1] / 4.0f, 
		};
		
		const FVector2f UVPosition3 = { 
			VoxelUVArray[DirectionIndex][3][0] / 4.0f, 
			VoxelUVArray[DirectionIndex][3][1] / 4.0f, 
		};
		
		const int32 UVIndex0 = UVOverlay->AppendElement(UVPosition0);
		const int32 UVIndex1 = UVOverlay->AppendElement(UVPosition1);
		const int32 UVIndex2 = UVOverlay->AppendElement(UVPosition2);
		const int32 UVIndex3 = UVOverlay->AppendElement(UVPosition3);
	
		UVOverlay->SetTriangle(TriangleIndex0, UE::Geometry::FIndex3i(UVIndex0, UVIndex1, UVIndex2));
		UVOverlay->SetTriangle(TriangleIndex1, UE::Geometry::FIndex3i(UVIndex0, UVIndex2, UVIndex3));
	}

	return DynamicMesh;
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

FCell& 
AVoxelWorld::GetCell(FIntVector3 CellCoordinate)
{
	const int32 CellIndex = CellCoordinateToCellIndex(CellCoordinate);
	
	return CellArray[CellIndex];
}


