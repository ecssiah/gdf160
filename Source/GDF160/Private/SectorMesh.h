#pragma once

#include "SectorFace.h"

struct FSectorMesh
{
	int32 SectorIndex;
	TArray<FSectorFace> SectorFaceArray;
};
