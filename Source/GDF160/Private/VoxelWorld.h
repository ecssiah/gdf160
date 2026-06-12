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
	
	UPROPERTY()
	APawn* PlayerPawn;
	
	UPROPERTY(EditAnywhere)
	UMaterialInterface* BlockMaterial;
	
	UFUNCTION(BlueprintPure, Category="Voxel")
	FIntVector GetPlayerCellCoordinate() const;
	
	UFUNCTION(BlueprintPure, Category="Voxel")
	FIntVector2 GetPlayerSectorCoordinate() const;

protected:

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:
	
	FIntVector PlayerCellCoordinate;
	FIntVector2 PlayerSectorCoordinate;
	
	TArray<FCell> CellArray;
	TArray<FSectorMesh> SectorMeshArray;
	TArray<TObjectPtr<USectorComponent>> FreeSectorComponentArray;
	TMap<FIntVector2, TObjectPtr<USectorComponent>> SectorComponentMap;
	
	static bool CellCoordinateIsValid(const FIntVector& CellCoordinate);
	static bool SectorCoordinateIsValid(const FIntVector2& SectorCoordinate);
	
	static FIntVector2 SectorIndexToSectorCoordinate(uint32 SectorIndex);
	
	static int32 SectorCoordinateToSectorIndex(const FIntVector2& SectorCoordinate);
	static FIntVector SectorCoordinateToCellCoordinate(const FIntVector2& SectorCoordinate);
	
	static FIntVector CellIndexToCellCoordinate(int32 CellIndex);
	static int32 CellCoordinateToCellIndex(const FIntVector& CellCoordinate);
	static int32 CellCoordinateToSectorIndex(const FIntVector& CellCoordinate);
	
	static FIntVector WorldLocationToCellCoordinate(const FVector& WorldLocation);
	static FIntVector2 WorldLocationToSectorCoordinate(const FVector& WorldLocation);
	
	FCell& GetCell(const FIntVector& CellCoordinate);

	void InitPlayer();
	void InitSectorCache();	
	
	void GenerateWorld();
	
	void BuildSectorMeshes();
	FSectorMesh BuildSectorMesh(int32 SectorIndex);
	
	uint8 CalculateNeighborSet(const FCell& Cell);
	
	void UpdateSectorComponents();
	
	void AddSectorComponent(const FIntVector2& SectorCoordinate);
	void RemoveSectorComponent(const FIntVector2& SectorCoordinate);
};
