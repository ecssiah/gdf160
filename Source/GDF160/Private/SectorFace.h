#pragma once

#include "BlockKind.h"
#include "CartesianDirection.h"

struct FSectorFace
{
	EBlockKind BlockKind;
	EDirection Direction;
	
	FVector Position;
};
