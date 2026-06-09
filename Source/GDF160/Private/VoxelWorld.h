// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Cell.h"
#include "SectorComponent.h"
#include "SectorMesh.h"
#include "GameFramework/Actor.h"
#include "DynamicMesh/DynamicMesh3.h"
#include "VoxelWorld.generated.h"

constexpr int32 WorldSizeInSectorsXLog2 = 1;
constexpr int32 WorldSizeInSectorsX = 1 << WorldSizeInSectorsXLog2;
constexpr int32 WorldSizeInSectorsYLog2 = 1;
constexpr int32 WorldSizeInSectorsY = 1 << WorldSizeInSectorsYLog2;

constexpr int32 WorldAreaInSectors = WorldSizeInSectorsX * WorldSizeInSectorsY;

constexpr int32 SectorSizeInCellsXLog2 = 2;
constexpr int32 SectorSizeInCellsX = 1 << SectorSizeInCellsXLog2;
constexpr int32 SectorSizeInCellsYLog2 = 2;
constexpr int32 SectorSizeInCellsY = 1 << SectorSizeInCellsYLog2;
constexpr int32 SectorSizeInCellsZLog2 = 4;
constexpr int32 SectorSizeInCellsZ = 1 << SectorSizeInCellsZLog2;

constexpr int32 SectorAreaInCells = SectorSizeInCellsX * SectorSizeInCellsY;
constexpr int32 SectorVolumeInCells = SectorSizeInCellsX * SectorSizeInCellsY * SectorSizeInCellsZ;

constexpr int32 WorldSizeInCellsX = WorldSizeInSectorsX * SectorSizeInCellsX;
constexpr int32 WorldSizeInCellsY = WorldSizeInSectorsY * SectorSizeInCellsY;
constexpr int32 WorldSizeInCellsZ = SectorSizeInCellsX;

constexpr int32 WorldVolumeInCells = WorldSizeInCellsX * WorldSizeInCellsY * WorldSizeInCellsZ; 

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
	TArray<FCell> CellArray;
	TArray<FSectorMesh> SectorMeshArray;
	
	UPROPERTY()
	TMap<FIntVector2, USectorComponent*> SectorComponentMap;
	
	void GenerateWorld();
	
	uint8 CalculateNeighborSet(const FCell& Block);
	
	void BuildSectorMeshes();
	FSectorMesh BuildSectorMesh(int32 SectorIndex);
	
	void BuildSectorComponents();
	USectorComponent* BuildSectorComponent(int32 SectorIndex);
	
	FDynamicMesh3 BuildDynamicMesh(const FSectorMesh& SectorMesh);
	
	bool CellCoordinateIsValid(FIntVector3 GridCoordinate);
	
	FIntVector2 SectorIndexToSectorCoordinate(uint32 SectorIndex);
	int32 SectorCoordinateToSectorIndex(FIntVector2 SectorCoordinate);
	FIntVector3 SectorCoordinateToCellCoordinate(FIntVector2 SectorCoordinate);
	
	int32 CellCoordinateToSectorIndex(FIntVector3 CellCoordinate);
	int32 CellCoordinateToCellIndex(FIntVector3 CellCoordinate);
	
	FIntVector3 CellIndexToCellCoordinate(int32 CellIndex);
	
	FCell& GetCell(FIntVector3 CellCoordinate);
};
