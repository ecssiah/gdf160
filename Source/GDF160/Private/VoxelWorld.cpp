// Fill out your copyright notice in the Description page of Project Settings.


#include "VoxelWorld.h"

#include "CartesianDirection.h"
#include "DynamicMesh/DynamicMesh3.h"
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

void AVoxelWorld::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
	GenerateWorld();
	
	BuildSectorMeshes();
	BuildSectorComponents();
}

// Called when the game starts or when spawned
void AVoxelWorld::BeginPlay()
{
	Super::BeginPlay();
	
	UE_LOG(LogTemp, Warning, TEXT("AVoxelWorld::BeginPlay()"));
}

// Called every frame
void AVoxelWorld::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AVoxelWorld::GenerateWorld()
{
	SectorArray.SetNum(WorldAreaInSectors);
	
	for (uint32 SectorIndex = 0; SectorIndex < SectorArray.Num(); SectorIndex++)
	{
		GenerateSector(SectorIndex);
	}
}

void AVoxelWorld::GenerateSector(const int32 SectorIndex)
{
	FSector& Sector = SectorArray[SectorIndex];
	Sector.SectorIndex = SectorIndex;
	Sector.SectorCoordinate = SectorIndexToSectorCoordinate(SectorIndex);
	
	Sector.CellArray.SetNum(SectorVolumeInCells);
	
	const int32 BlockKindCount = static_cast<int32>(EBlockKind::Count);
	
	for (int32 CellIndex = 0; CellIndex < Sector.CellArray.Num(); CellIndex++)
	{
		const EBlockKind BlockKind = static_cast<EBlockKind>(FMath::RandRange(0, BlockKindCount - 1));
		
		FCell& Cell = Sector.CellArray[CellIndex];
		
		Cell.CellIndex = CellIndex;
		Cell.BlockKind = BlockKind;
		Cell.NeighborSet.Empty();
	}
}

TBitArray<> AVoxelWorld::CalculateNeighborSet(const FCell& Block)
{
	TBitArray NeighborSet;
	NeighborSet.Empty();
	
	for (const ECartesianDirection Direction : TEnumRange<ECartesianDirection>())
	{
		const int32 DirectionIndex = static_cast<int32>(Direction);
		const FIntVector3 DirectionOffset = DirectionOffsets[DirectionIndex];
		const FIntVector3 TestGridPosition = Block.CellCoordinate + DirectionOffset;
		
		const FCell& TestBlock = GetCell(TestGridPosition);
		
		if (TestBlock.BlockKind == EBlockKind::None)
		{
			NeighborSet.Insert(true, DirectionIndex);
		}
	}
	
	return NeighborSet;
}

void AVoxelWorld::BuildSectorMeshes()
{
	for (int32 SectorIndex = 0; SectorIndex < SectorArray.Num(); SectorIndex++)
	{
		BuildSectorMesh(SectorIndex);
	}
}

void AVoxelWorld::BuildSectorMesh(const int32 SectorIndex)
{
	const FIntVector2 SectorCoordinate = SectorIndexToSectorCoordinate(SectorIndex);
	const FIntVector3 SectorGridCoordinate = SectorCoordinateToGridCoordinate(SectorCoordinate);
	
	for (int32 GridZ = 0; GridZ < SectorSizeInCellsZ; GridZ++)
	{
		for (int32 GridY = SectorGridCoordinate.Y; GridY < SectorGridCoordinate.Y + WorldSizeInSectorsY; GridY++)
		{
			for (int32 GridX = SectorGridCoordinate.X; GridX < SectorGridCoordinate.X + WorldSizeInSectorsX; GridX++)
			{
				const FIntVector3 GridCoordinate = { GridX, GridY, GridZ };
				
				if (GridCoordinateIsValid(GridCoordinate))
				{
					FCell& Cell = GetCell(GridCoordinate);
					
					Cell.NeighborSet = CalculateNeighborSet(Cell);
				}
			}
		}
	}
}

void AVoxelWorld::BuildSectorComponents()
{
	if (BlockMaterial == nullptr)
	{
		return;
	}
	
	for (int32 SectorX = 0; SectorX < WorldSizeInSectorsX; SectorX++)
	{
		for (int32 SectorY = 0; SectorY < WorldSizeInSectorsY; SectorY++)
		{
			const FIntVector2 SectorCoordinate = FIntVector2(SectorX, SectorY);
			const uint32 SectorIndex = SectorCoordinateToSectorIndex(SectorCoordinate);

			BuildSectorComponent(SectorArray[SectorIndex]);
		}
	}
}

static FDynamicMesh3 BuildDynamicMesh(const FSector& Sector)
{
	FDynamicMesh3 DynamicMesh;
	
	DynamicMesh.EnableAttributes();
	
	for (const FCell& Cell : Sector.CellArray)
	{
		if (Cell.BlockKind == EBlockKind::None)
		{
			continue;
		}
		
		const int32 VertexIndex0 = DynamicMesh.AppendVertex(FVector3d(0, 0, 0));
		const int32 VertexIndex1 = DynamicMesh.AppendVertex(FVector3d(100, 0, 0));
		const int32 VertexIndex2 = DynamicMesh.AppendVertex(FVector3d(0, 100, 0));
		const int32 VertexIndex3 = DynamicMesh.AppendVertex(FVector3d(0, 100, 0));
		
		const int32 TriangleIndex0 = DynamicMesh.AppendTriangle(VertexIndex0, VertexIndex1, VertexIndex2);
		const int32 TriangleIndex1 = DynamicMesh.AppendTriangle(VertexIndex0, VertexIndex2, VertexIndex3);
		
		UE::Geometry::FDynamicMeshUVOverlay* UVOverlay = DynamicMesh.Attributes()->PrimaryUV();
		
		const int32 UV0 = UVOverlay->AppendElement(FVector2f(0.0f, 0.0f));
		const int32 UV1 = UVOverlay->AppendElement(FVector2f(1.0f, 0.0f));
		const int32 UV2 = UVOverlay->AppendElement(FVector2f(0.0f, 1.0f));
		const int32 UV3 = UVOverlay->AppendElement(FVector2f(1.0f, 1.0f));
		
		UVOverlay->SetTriangle(TriangleIndex0, UE::Geometry::FIndex3i(UV0, UV1, UV2));
		UVOverlay->SetTriangle(TriangleIndex1, UE::Geometry::FIndex3i(UV0, UV1, UV2));
	}

	return DynamicMesh;
}

void AVoxelWorld::BuildSectorComponent(const FSector& Sector)
{
	// if (SectorComponentCache.Find(Sector.SectorCoordinate) == nullptr)
	// {
	// 	const FString ComponentName = FString::Printf(TEXT("SectorMesh_%d"), Sector.SectorIndex);
	//
	// 	USectorComponent* SectorComponent = NewObject<USectorComponent>(this, *ComponentName);
	// 		
	// 	SectorComponent->RegisterComponent();
	// 		
	// 	SectorComponent->AttachToComponent(
	// 		RootComponent,
	// 		FAttachmentTransformRules::KeepRelativeTransform
	// 	);
	// 	
	// 	SectorComponent->SetMaterial(0, BlockMaterial);
	// 	
	// 	FDynamicMesh3 DynamicMesh = BuildDynamicMesh(Sector);
	// 	
	// 	SectorComponent->GetDynamicMesh()->SetMesh(MoveTemp(DynamicMesh));
	// 	
	// 	SectorComponentCache.Add(Sector.SectorCoordinate, SectorComponent);
	// }
}

FCell& AVoxelWorld::GetCell(const FIntVector3 GridCoordinate)
{
	const int32 SectorIndex = GridCoordinateToSectorIndex(GridCoordinate);
	
	FSector& Sector = SectorArray[SectorIndex];
	
	const int32 CellIndex = GridCoordinateToCellIndex(GridCoordinate);
	
	return Sector.CellArray[CellIndex];
}

