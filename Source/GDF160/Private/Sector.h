#pragma once

#include "Cell.h"

constexpr int32 SectorSizeInCellsXLog2 = 2;
constexpr int32 SectorSizeInCellsX = 1 << SectorSizeInCellsXLog2;
constexpr int32 SectorSizeInCellsYLog2 = 2;
constexpr int32 SectorSizeInCellsY = 1 << SectorSizeInCellsYLog2;
constexpr int32 SectorSizeInCellsZLog2 = 3;
constexpr int32 SectorSizeInCellsZ = 1 << SectorSizeInCellsZLog2;

constexpr int32 SectorAreaInCells = SectorSizeInCellsX * SectorSizeInCellsY;
constexpr int32 SectorVolumeInCells = SectorSizeInCellsX * SectorSizeInCellsY * SectorSizeInCellsZ;

struct FSector
{
	int32 SectorIndex;
	FIntVector2 SectorCoordinate;
	
	TArray<FCell> CellArray;
};
