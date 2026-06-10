#pragma once

#include "CoreMinimal.h"
#include "Cell.h"
#include "SectorComponent.h"
#include "SectorMesh.h"
#include "GameFramework/Actor.h"
#include "VoxelWorld.generated.h"

UCLASS()
class GDF160_API AVoxelWorld : public AActor
{
	GENERATED_BODY()

public:
	
	AVoxelWorld();
	
	virtual void OnConstruction(const FTransform& Transform) override;
	
	virtual void Tick(float DeltaTime) override;
	
	UPROPERTY()
	APawn* PlayerPawn;
	
	UPROPERTY(EditAnywhere)
	UMaterialInterface* BlockMaterial;

protected:

	virtual void BeginPlay() override;

private:
	
	FIntVector2 PlayerSectorCoordinate;
	
	TArray<FCell> CellArray;
	TArray<FSectorMesh> SectorMeshArray;
	
	UPROPERTY()
	TArray<TObjectPtr<USectorComponent>> FreeSectorComponentArray;
	
	UPROPERTY()
	TMap<FIntVector2, TObjectPtr<USectorComponent>> SectorComponentMap;
	
	bool CellCoordinateIsValid(FIntVector3 CellCoordinate);
	bool SectorCoordinateIsValid(FIntVector2 SectorCoordinate);
	
	FIntVector2 SectorIndexToSectorCoordinate(uint32 SectorIndex);
	
	int32 SectorCoordinateToSectorIndex(FIntVector2 SectorCoordinate);
	FIntVector3 SectorCoordinateToCellCoordinate(FIntVector2 SectorCoordinate);
	
	int32 CellCoordinateToSectorIndex(FIntVector3 CellCoordinate);
	
	int32 CellCoordinateToCellIndex(FIntVector3 CellCoordinate);
	FIntVector3 CellIndexToCellCoordinate(int32 CellIndex);
	
	FIntVector3 WorldLocationToCellCoordinate(FVector WorldLocation);
	FIntVector2 WorldLocationToSectorCoordinate(FVector WorldLocation);
	
	FCell& GetCell(FIntVector3 CellCoordinate);
	
	void GenerateWorld();
	
	void BuildSectorMeshes();
	FSectorMesh BuildSectorMesh(int32 SectorIndex);
	
	void UpdateSectorComponents();
	
	void AddSectorComponent(const FIntVector2& SectorCoordinate);
	void RemoveSectorComponent(const FIntVector2& SectorCoordinate);
	
	uint8 CalculateNeighborSet(const FCell& Block);
};
