#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class GAMECORE_API baryCentricInterpolator
{
public:
    baryCentricInterpolator();
    ~baryCentricInterpolator();

    void setup(FVector &a, FVector &b, FVector &c);
    void interpolateAll(int stepdistance, std::vector<FVector> &output);

    void interpolateAllAsMatrixTransformToVertexAndLookOfNormal(
        int stepdistance,
        std::vector<MMatrix> &output
    );

private:
    FVector v0;
    FVector v1;
    FVector v2;

    FVector normal;
    MMatrix rotator;

    void interpolateLinear(
        FVector &start,
        FVector &end,
        std::vector<FVector> &output,
        int stepDistance
    );
};