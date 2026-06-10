// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SectorMesh.h"
#include "Components/DynamicMeshComponent.h"
#include "DynamicMesh/DynamicMesh3.h"
#include "SectorComponent.generated.h"

/**
 * 
 */
UCLASS()
class USectorComponent : public UDynamicMeshComponent
{
	GENERATED_BODY()
	
public:
	
	static FDynamicMesh3 BuildDynamicMesh(const FSectorMesh& SectorMesh);

private:
	
	static FVector2f BlockKindToUVCoordinate(EBlockKind BlockKind);
	
};
