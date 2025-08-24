#include "BoundingBoxSimple.h"


BoundingBoxSimple::BoundingBoxSimple(){
    bottomLeftNear = FVector(0, 0, 0);
    topRightLeft = FVector(0, 0, 0);
}

BoundingBoxSimple::BoundingBoxSimple(TArray<FVector> &vertecies){
    bottomLeftNear = FVector(0, 0, 0);
    topRightLeft = FVector(0, 0, 0);
    updateBoundsIfNeeded(vertecies);
}

BoundingBoxSimple &BoundingBoxSimple::operator=(const BoundingBoxSimple &other){
    if(this != &other){
        bottomLeftNear = other.bottomLeftNear;
        topRightLeft = other.topRightLeft;
    }
    return *this;
}

BoundingBoxSimple::BoundingBoxSimple(const BoundingBoxSimple &other){
    if(this != &other){
        *this = other;
    }
}



BoundingBoxSimple::~BoundingBoxSimple(){

}

void BoundingBoxSimple::updateBoundsIfNeeded(TArray<FVector> &vertecies){
    for (int i = 0; i < vertecies.Num(); i++){
        updateBoundsIfNeeded(vertecies[i]);
    }
}

void BoundingBoxSimple::updateBoundsIfNeeded(FVector &other){
    bottomLeftNear.X = std::min(bottomLeftNear.X, other.X);
    bottomLeftNear.Y = std::min(bottomLeftNear.Y, other.Y);
    bottomLeftNear.Z = std::min(bottomLeftNear.Z, other.Z);

    topRightLeft.X = std::max(topRightLeft.X, other.X);
    topRightLeft.Y = std::max(topRightLeft.Y, other.Y);
    topRightLeft.Z = std::max(topRightLeft.Z, other.Z);
}

bool BoundingBoxSimple::isInsideBoundingbox(FVector &other){
    return other.X > bottomLeftNear.X &&
           other.Y > bottomLeftNear.Y &&
           other.Z > bottomLeftNear.Z &&
           other.X < topRightLeft.X &&
           other.Y < topRightLeft.Y &&
           other.Z < topRightLeft.Z;
}



FVector BoundingBoxSimple::bottomLeftNearVertex(){
    return bottomLeftNear;
}

FVector BoundingBoxSimple::topRightLeftVertex(){
    return topRightLeft;
}