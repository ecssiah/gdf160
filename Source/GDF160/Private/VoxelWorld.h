#pragma once

#include "CoreMinimal.h"
#include "WorldData.h"
#include "SectorComponent.h"

#include "VoxelWorld.generated.h"

UCLASS()
class GDF160_API AVoxelWorld : public AActor
{
	GENERATED_BODY()

public:
	
	AVoxelWorld();
	
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
	
	static bool CellCoordinateIsValid(const FIntVector& CellCoordinate);
	static bool SectorCoordinateIsValid(const FIntVector2& SectorCoordinate);
	
	static FIntVector2 SectorIndexToSectorCoordinate(uint32 SectorIndex);
	
	static int32 SectorCoordinateToSectorIndex(const FIntVector2& SectorCoordinate);
	static FIntVector SectorCoordinateToCellCoordinate(const FIntVector2& SectorCoordinate);
	
	static int32 CellCoordinateToSectorIndex(const FIntVector& CellCoordinate);
	
	static int32 CellCoordinateToCellIndex(const FIntVector& CellCoordinate);
	static FIntVector CellIndexToCellCoordinate(int32 CellIndex);
	
	UFUNCTION(BlueprintPure, Category="Voxel")
	static FIntVector WorldLocationToCellCoordinate(const FVector& WorldLocation);
	
	UFUNCTION(BlueprintPure, Category="Voxel")
	static FIntVector2 WorldLocationToSectorCoordinate(const FVector& WorldLocation);
	
	FCell& GetCell(const FIntVector& CellCoordinate);
	
	void GenerateWorld();

	void InitPlayer();
	void InitSectorCache();	
	
	void BuildSectorMeshes();
	FSectorMesh BuildSectorMesh(int32 SectorIndex);
	
	void UpdateSectorComponents();
	
	void AddSectorComponent(const FIntVector2& SectorCoordinate);
	void RemoveSectorComponent(const FIntVector2& SectorCoordinate);
	
	uint8 CalculateNeighborSet(const FCell& Cell);
};
