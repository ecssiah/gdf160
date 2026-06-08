#pragma once

UENUM()
enum class ECartesianDirection : uint8
{
	PosX,
	PosY,
	NegX,
	NegY,
	PosZ,
	NegZ,

	Count
};

ENUM_RANGE_BY_COUNT(ECartesianDirection, ECartesianDirection::Count)

const FIntVector3 DirectionOffsets[] =
{
	{ +1, +0, +0},
	{ +0, +1, +0},
	{ -1, +0, +0},
	{ +0, -1, +0},
	{ +0, +0, +1},
	{ +0, +0, -1},
};