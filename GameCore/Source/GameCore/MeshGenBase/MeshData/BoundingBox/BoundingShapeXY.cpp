#include "BoundingShapeXY.h"

BoundingShapeXY::BoundingShapeXY(){

}

BoundingShapeXY::~BoundingShapeXY(){

}

void BoundingShapeXY::initWithSortedQuad(FVector &a, FVector &b, FVector &c, FVector &d){
    TArray<FVector> array = {a, b, c, d};
    initWithSortedShape(array);
}

void BoundingShapeXY::initWithSortedShape(
    TArray<FVector> &array
){
    shape = array;
}


bool BoundingShapeXY::isInsideShape(FVector &test){
    bool flagPrev = true;
    for (int i = 1; i < shape.Num(); i++)
    {
        FVector &prev = shape[i - 1];
        FVector &current = shape[i];
        
        //wenn sich die seite ändert ist es nicht im shape
        //(?)
        bool flag = rightOff(prev, current, test);
        if(i == 1){
            flagPrev = flag;
        }else{
            if(flagPrev != flag){
                return false;
            }
        }
    }
    return true;
}

bool BoundingShapeXY::rightOff(FVector &a, FVector &b, FVector &target){
    FVector edge = b - a; //AB = B - A
    FVector relativeTarget = target - a;
    FVector normal = FVector(edge.Y, -1.0f * edge.X, 0.0f); // n = (a.y, -a.x)
    relativeTarget.Z = 0.0f;

    return FVector::DotProduct(normal, relativeTarget) > 0.0f;
}