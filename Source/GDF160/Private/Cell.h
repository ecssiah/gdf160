#pragma once

#include "BlockKind.h"

constexpr int32 FaceCountPerBlock = 6;
constexpr int32 VertexCountPerFace = 6;

const float VoxelVertexArray[FaceCountPerBlock][VertexCountPerFace][3] =
{
	// +X Face
	{
		{1, 0, 0}, {1, 1, 0}, {1, 1, 1},
		{1, 0, 0}, {1, 1, 1}, {1, 0, 1},
	},
	// -X Face
	{
        {0, 1, 0}, {0, 0, 0}, {0, 0, 1},
		{0, 1, 0}, {0, 0, 1}, {0, 1, 1},
	},
	// +Y Face
	{
        {1, 1, 0}, {0, 1, 0}, {0, 1, 1},
		{1, 1, 0}, {0, 1, 1}, {1, 1, 1},
	},
	// -Y Face
	{
        {0, 0, 0}, {1, 0, 0}, {1, 0, 1},
		{0, 0, 0}, {1, 0, 1}, {0, 0, 1},
	},
	// +Z Face
	{
        {0, 0, 1}, {1, 0, 1}, {1, 1, 1},
		{0, 0, 1}, {1, 1, 1}, {0, 1, 1},
	},
	// -Z Face
	{
        {0, 1, 0}, {1, 1, 0}, {1, 0, 0},
		{0, 1, 0}, {1, 0, 0}, {0, 0, 0},
	},
};

const float VoxelUVArray[FaceCountPerBlock][VertexCountPerFace][2] =
{
	// +X Face
	{
		{0, 0}, {1, 0}, {1, 1},
		{0, 0}, {1, 1}, {0, 1},
	},
	// -X Face
	{
		{0, 0}, {1, 0}, {1, 1},
		{0, 0}, {1, 1}, {0, 1},
	},
	// +Y Face
	{
		{0, 0}, {1, 0}, {1, 1},
		{0, 0}, {1, 1}, {0, 1},
	},
	// -Y Face
	{
		{0, 0}, {1, 0}, {1, 1},
		{0, 0}, {1, 1}, {0, 1},
	},
	// +Z Face
	{
		{0, 0}, {1, 0}, {1, 1},
		{0, 0}, {1, 1}, {0, 1},
	},
	// -Z Face
	{
		{0, 0}, {1, 0}, {1, 1},
		{0, 0}, {1, 1}, {0, 1},
	},
};

struct FCell
{
	int32 CellIndex;
	FIntVector3 CellCoordinate;
	
	EBlockKind BlockKind;
	TBitArray<> NeighborSet;
};
