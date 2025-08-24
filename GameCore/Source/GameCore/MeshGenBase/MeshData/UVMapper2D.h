#pragma once

#include "CoreMinimal.h"
class BoundingBoxSimple;

class GAMECORE_API UVMapper2D{

public:
    UVMapper2D();
    ~UVMapper2D();

    void generateUVBuffer(
        const TArray<FVector> &vertecies,
        FVector normal,
        TArray<FVector2D> &outUV
    );

private:
    FVector2D generateUVCoordinate(
        FVector &vertex,
        BoundingBoxSimple &boundingBox
    );
};