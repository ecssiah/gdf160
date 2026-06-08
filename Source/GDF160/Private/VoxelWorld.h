// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IPropertyTable.h"
#include "Sector.h"
#include "SectorComponent.h"
#include "GameFramework/Actor.h"
#include "VoxelWorld.generated.h"

constexpr uint32 WorldSizeInSectorsXLog2 = 1;
constexpr uint32 WorldSizeInSectorsX = 1 << WorldSizeInSectorsXLog2;
constexpr uint32 WorldSizeInSectorsYLog2 = 1;
constexpr uint32 WorldSizeInSectorsY = 1 << WorldSizeInSectorsYLog2;

constexpr uint32 WorldAreaInSectors = WorldSizeInSectorsX * WorldSizeInSectorsY;

FORCEINLINE bool
GridCoordinateIsValid(const FIntVector3 GridCoordinate)
{
	return (
		GridCoordinate.X >= 0 && GridCoordinate.X < WorldSizeInSectorsX * SectorSizeInCellsX &&
		GridCoordinate.Y >= 0 && GridCoordinate.Y < WorldSizeInSectorsY * SectorSizeInCellsY &&
		GridCoordinate.Z >= 0 && GridCoordinate.Z < SectorSizeInCellsZ
	);
}

FORCEINLINE FIntVector2
SectorIndexToSectorCoordinate(const uint32 SectorIndex)
{
	return {
		static_cast<int32>(SectorIndex % WorldSizeInSectorsX),
		static_cast<int32>(SectorIndex / WorldSizeInSectorsY),
	};
}

FORCEINLINE uint32
SectorCoordinateToSectorIndex(const FIntVector2 SectorCoordinate)
{
	return SectorCoordinate.X + SectorCoordinate.Y * WorldSizeInSectorsX;
}

FORCEINLINE uint32
GridCoordinateToSectorIndex(const FIntVector3 GridCoordinate)
{
	const FIntVector2 SectorCoordinate = {
		GridCoordinate.X >> SectorSizeInCellsXLog2,
		GridCoordinate.Y >> SectorSizeInCellsYLog2,
	};
	
	return SectorCoordinate.X + SectorCoordinate.Y * WorldSizeInSectorsX;
}

FORCEINLINE uint32
GridCoordinateToCellIndex(const FIntVector3 GridCoordinate)
{
	const uint32 WorldStrideX = 1;
	const uint32 WorldStrideY = WorldSizeInSectorsX * SectorSizeInCellsX;
	const uint32 WorldStrideZ = WorldStrideY * (WorldSizeInSectorsY * SectorSizeInCellsY);
	
	return GridCoordinate.X * WorldStrideX + GridCoordinate.Y * WorldStrideY + GridCoordinate.Z * WorldStrideZ;
}

FORCEINLINE FIntVector3 
SectorCoordinateToGridCoordinate(const FIntVector2 SectorCoordinate)
{
	return {
		SectorCoordinate.X * SectorSizeInCellsX,
		SectorCoordinate.Y * SectorSizeInCellsY,
		0,
	};
}

FORCEINLINE FIntVector3 
IndicesToGridCoordinate(const int32 SectorIndex, const int32 CellIndex)
{
	const FIntVector2 SectorCoordinate = SectorIndexToSectorCoordinate(SectorIndex);
	const FIntVector3 CellCoordinate = CellIndexToCellCoordinate(CellIndex);
	
	return {
		SectorCoordinate.X * SectorSizeInCellsX + CellCoordinate.X,
		SectorCoordinate.Y * SectorSizeInCellsY + CellCoordinate.Y,
		CellCoordinate.Z,
	};
}


UCLASS()
class GDF160_API AVoxelWorld : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AVoxelWorld();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void OnConstruction(const FTransform& Transform) override;
	
	virtual void Tick(float DeltaTime) override;
	
	UPROPERTY(EditAnywhere)
	UMaterialInterface* BlockMaterial;

private:
	TArray<FSector> SectorArray;
	
	UPROPERTY()
	TMap<FIntVector2, USectorComponent*> SectorComponentCache;
	
	void GenerateWorld();
	void GenerateSector(const int32 SectorIndex);
	
	TBitArray<> CalculateNeighborSet(const FCell& Block);
	
	void BuildSectorMeshes();
	void BuildSectorMesh(const int32 SectorIndex);
	
	void BuildSectorComponents();
	void BuildSectorComponent(const FSector& Sector);
	
	FCell& GetCell(const FIntVector3 GridCoordinate);
};
