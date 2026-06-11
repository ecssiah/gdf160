constexpr float CellSizeInCentimeters = 100.0f;

constexpr int32 SectorSizeInCellsXLog2 = 4;
constexpr int32 SectorSizeInCellsX = 1 << SectorSizeInCellsXLog2;
constexpr int32 SectorSizeInCellsYLog2 = 4;
constexpr int32 SectorSizeInCellsY = 1 << SectorSizeInCellsYLog2;
constexpr int32 SectorSizeInCellsZLog2 = 4;
constexpr int32 SectorSizeInCellsZ = 1 << SectorSizeInCellsZLog2;

constexpr int32 SectorAreaInCells = SectorSizeInCellsX * SectorSizeInCellsY;
constexpr int32 SectorVolumeInCells = SectorSizeInCellsX * SectorSizeInCellsY * SectorSizeInCellsZ;

constexpr int32 WorldSizeInSectorsXLog2 = 4;
constexpr int32 WorldSizeInSectorsX = 1 << WorldSizeInSectorsXLog2;
constexpr int32 WorldSizeInSectorsYLog2 = 4;
constexpr int32 WorldSizeInSectorsY = 1 << WorldSizeInSectorsYLog2;

constexpr int32 WorldAreaInSectors = WorldSizeInSectorsX * WorldSizeInSectorsY;

constexpr int32 WorldSizeInCellsX = WorldSizeInSectorsX * SectorSizeInCellsX;
constexpr int32 WorldSizeInCellsY = WorldSizeInSectorsY * SectorSizeInCellsY;
constexpr int32 WorldSizeInCellsZ = SectorSizeInCellsX;

constexpr int32 WorldVolumeInCells = WorldSizeInCellsX * WorldSizeInCellsY * WorldSizeInCellsZ; 

const int32 WorldStrideX = 1;
const int32 WorldStrideY = WorldSizeInSectorsX * SectorSizeInCellsX;
const int32 WorldStrideZ = WorldSizeInSectorsX * SectorSizeInCellsX * WorldSizeInSectorsY * SectorSizeInCellsY;

constexpr int32 SectorViewRange = 1;

constexpr int32 TileSizeInPixelsX = 64;
constexpr int32 TileSizeInPixelsY = 64;

constexpr int32 TileAtlasSizeU = 4;
constexpr int32 TileAtlasSizeV = 4;

constexpr float TileSizeU = 1.0f / static_cast<float>(TileAtlasSizeU);
constexpr float TileSizeV = 1.0f / static_cast<float>(TileAtlasSizeV);