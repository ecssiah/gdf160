#pragma once

#include "BlockKind.h"
#include "CartesianDirection.h"

struct FSectorFace
{
	EBlockKind BlockKind;
	ECartesianDirection Direction;
	
	FIntVector3 CellCoordinate;
};

struct FSectorMesh
{
	int32 SectorIndex;
	TArray<FSectorFace> SectorFaceArray;
};
