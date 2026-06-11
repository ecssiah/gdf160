#include "SectorComponent.h"

FDynamicMesh3
USectorComponent::BuildDynamicMesh(const FSectorMesh& SectorMesh)
{
	FDynamicMesh3 DynamicMesh;
	
	DynamicMesh.EnableAttributes();
	
	for (const FSectorFace& SectorFace : SectorMesh.SectorFaceArray)
	{
		if (SectorFace.BlockKind == EBlockKind::None)
		{
			continue;
		}
		
		const int32 DirectionIndex = static_cast<int32>(SectorFace.Direction);
		
		const FVector CellPosition = FVector(SectorFace.CellCoordinate);
		
		const float (&VertexArray)[4][3] = VoxelVertexArray[DirectionIndex];
		
		const FVector3d VertexPosition0 = { 
			VertexArray[0][0], 
			VertexArray[0][1], 
			VertexArray[0][2] 
		};
		
		const FVector3d VertexPosition1 = { 
			VertexArray[1][0], 
			VertexArray[1][1], 
			VertexArray[1][2] 
		};
		
		const FVector3d VertexPosition2 = { 
			VertexArray[2][0], 
			VertexArray[2][1], 
			VertexArray[2][2] 
		};
		
		const FVector3d VertexPosition3 = { 
			VertexArray[3][0], 
			VertexArray[3][1], 
			VertexArray[3][2] 
		};
		
		const int32 VertexIndex0 = DynamicMesh.AppendVertex(CellSizeInCentimeters * (CellPosition + VertexPosition0));
		const int32 VertexIndex1 = DynamicMesh.AppendVertex(CellSizeInCentimeters * (CellPosition + VertexPosition1));
		const int32 VertexIndex2 = DynamicMesh.AppendVertex(CellSizeInCentimeters * (CellPosition + VertexPosition2));
		const int32 VertexIndex3 = DynamicMesh.AppendVertex(CellSizeInCentimeters * (CellPosition + VertexPosition3));
		
		const int32 TriangleIndex0 = DynamicMesh.AppendTriangle(VertexIndex0, VertexIndex1, VertexIndex2);
		const int32 TriangleIndex1 = DynamicMesh.AppendTriangle(VertexIndex0, VertexIndex2, VertexIndex3);
		
		UE::Geometry::FDynamicMeshUVOverlay* UVOverlay = DynamicMesh.Attributes()->PrimaryUV();
		
		const FVector2f UVCoordinate = BlockKindToUVCoordinate(SectorFace.BlockKind);

		const FVector2f UVPosition0 = { 
			UVCoordinate.X + 0.0f,
			UVCoordinate.Y + TileSizeV,
		};
		
		const FVector2f UVPosition1 = { 
			UVCoordinate.X + TileSizeU,
			UVCoordinate.Y + TileSizeV,
		};
		
		const FVector2f UVPosition2 = { 
			UVCoordinate.X + TileSizeU,
			UVCoordinate.Y + 0.0f,
		};
		
		const FVector2f UVPosition3 = { 
			UVCoordinate.X + 0.0f,
			UVCoordinate.Y + 0.0f,
		};
		
		const int32 UVIndex0 = UVOverlay->AppendElement(UVPosition0);
		const int32 UVIndex1 = UVOverlay->AppendElement(UVPosition1);
		const int32 UVIndex2 = UVOverlay->AppendElement(UVPosition2);
		const int32 UVIndex3 = UVOverlay->AppendElement(UVPosition3);
	
		UVOverlay->SetTriangle(
			TriangleIndex0, 
			UE::Geometry::FIndex3i(UVIndex0, UVIndex1, UVIndex2)
		);
		
		UVOverlay->SetTriangle(
			TriangleIndex1, 
			UE::Geometry::FIndex3i(UVIndex0, UVIndex2, UVIndex3)
		);
	}

	return DynamicMesh;
}

FVector2f 
USectorComponent::BlockKindToUVCoordinate(EBlockKind BlockKind)
{
	checkf(BlockKind != EBlockKind::None, TEXT("BlockKind::None has no UV coordinates"));

	const int32 BlockKindIndex = static_cast<int32>(BlockKind);
		
	return {
		static_cast<float>((BlockKindIndex - 1) % TileAtlasSizeU) / TileAtlasSizeU,
		static_cast<float>((BlockKindIndex - 1) / TileAtlasSizeU) / TileAtlasSizeV,
	};
}
