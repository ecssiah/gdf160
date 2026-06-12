#pragma once

constexpr float CellSizeInCentimeters = 100.0f;

constexpr int32 SectorSizeInCellsXLog2 = 4;
constexpr int32 SectorSizeInCellsYLog2 = 4;
constexpr int32 SectorSizeInCellsZLog2 = 6;

constexpr int32 SectorSizeInCellsX = 1 << SectorSizeInCellsXLog2;
constexpr int32 SectorSizeInCellsY = 1 << SectorSizeInCellsYLog2;
constexpr int32 SectorSizeInCellsZ = 1 << SectorSizeInCellsZLog2;

constexpr int32 WorldSizeInSectorsXLog2 = 4;
constexpr int32 WorldSizeInSectorsYLog2 = 4;

constexpr int32 WorldSizeInSectorsX = 1 << WorldSizeInSectorsXLog2;
constexpr int32 WorldSizeInSectorsY = 1 << WorldSizeInSectorsYLog2;

constexpr int32 WorldAreaInSectors = WorldSizeInSectorsX * WorldSizeInSectorsY;

constexpr int32 WorldSizeInCellsX = SectorSizeInCellsX * WorldSizeInSectorsX;
constexpr int32 WorldSizeInCellsY = SectorSizeInCellsY * WorldSizeInSectorsY;
constexpr int32 WorldSizeInCellsZ = SectorSizeInCellsZ;

constexpr int32 WorldVolumeInCells = WorldSizeInCellsX * WorldSizeInCellsY * WorldSizeInCellsZ; 

constexpr int32 WorldStrideX = 1;
constexpr int32 WorldStrideY = WorldSizeInCellsX;
constexpr int32 WorldStrideZ = WorldSizeInCellsX * WorldSizeInCellsY;

constexpr int32 SectorViewRange = 2;

const float TerrainNoisePeriod = 300.0f;

constexpr int32 TerrainHeightMin = 4;
constexpr int32 TerrainHeightMax = WorldSizeInCellsZ - 20;

constexpr int32 TileSizeInPixelsX = 64;
constexpr int32 TileSizeInPixelsY = 64;

constexpr int32 TileAtlasSizeU = 4;
constexpr int32 TileAtlasSizeV = 4;

constexpr float TileSizeU = 1.0f / static_cast<float>(TileAtlasSizeU);
constexpr float TileSizeV = 1.0f / static_cast<float>(TileAtlasSizeV);

UENUM()
enum class EAxisDirection : uint8
{
	PosX,
	NegX,
	PosY,
	NegY,
	PosZ,
	NegZ,

	Count
};

ENUM_RANGE_BY_COUNT(EAxisDirection, EAxisDirection::Count)

const FIntVector AxisDirectionOffsets[] =
{
	{ +1, +0, +0 },
	{ -1, +0, +0 },
	{ +0, +1, +0 },
	{ +0, -1, +0 },
	{ +0, +0, +1 },
	{ +0, +0, -1 },
};

UENUM()
enum class EBlockKind : uint8
{
	None,
	GreenStone,
	BlueStone,
	PurpleStone,
	RedStone,
	
	Count,
};

const float VoxelVertexArray[6][4][3] =
{
	{
		{ 1, 1, 0 }, 
		{ 1, 0, 0 }, 
		{ 1, 0, 1 },
		{ 1, 1, 1 },
	},
	{
        { 0, 0, 0 }, 
		{ 0, 1, 0 }, 
		{ 0, 1, 1 },
		{ 0, 0, 1 },
	},
	{
        { 0, 1, 0 }, 
		{ 1, 1, 0 }, 
		{ 1, 1, 1 },
		{ 0, 1, 1 },
	},
	{
        { 1, 0, 0 }, 
		{ 0, 0, 0 }, 
		{ 0, 0, 1 },
		{ 1, 0, 1 },
	},
	{
        { 1, 0, 1 }, 
		{ 0, 0, 1 }, 
		{ 0, 1, 1 },
		{ 1, 1, 1 },
	},
	{
		{ 0, 0, 0 },
		{ 1, 0, 0 },
		{ 1, 1, 0 }, 
		{ 0, 1, 0 }, 
	},
};

struct FCell
{
	int32 CellIndex;
	
	EBlockKind BlockKind;
	uint8 NeighborSet;
};

struct FSectorFace
{
	EBlockKind BlockKind;
	EAxisDirection Direction;
	
	FIntVector CellCoordinate;
};

struct FSectorMesh
{
	int32 SectorIndex;
	TArray<FSectorFace> SectorFaceArray;
};
