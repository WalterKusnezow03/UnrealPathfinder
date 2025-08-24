
#include "baryCentricInterpolator.h"

baryCentricInterpolator::baryCentricInterpolator(){

}
baryCentricInterpolator::~baryCentricInterpolator(){

}

///@brief will setup the interpolator vertecies of the triangle, calculate the normal
///and prepare the rotator which might be wanted for interpolating as MMatrix output data
void baryCentricInterpolator::setup(FVector &a, FVector &b, FVector &c){
    v0 = a;
    v1 = b;
    v2 = c;

    FVector v0v1 = v1 - v0;
    FVector v0v2 = v2 - v0;
    normal = FVector::CrossProduct(v0v1, v0v2);
    normal = normal.GetSafeNormal();

    rotator = MMatrix::createRotatorFrom(normal); //das vertex da auch hinschaut später
}

///@brief interpolates all points in a stepdistance between points, using a Fvector output
void baryCentricInterpolator::interpolateAll(int stepdistance, std::vector<FVector> &output){
    stepdistance = std::abs(stepdistance);
    if (stepdistance < 1){
        return;
    }

    //B(x) = A + beta(B-A) + gamma(C-A)
    //heisst einmal A + beta(B-A) == M1
    //und einmal A + gamma(C-A) interpolieren ==M2
    //dann M1 x M2
    std::vector<FVector> M1;
    interpolateLinear(v0, v1, M1, stepdistance);
    std::vector<FVector> M2;
    interpolateLinear(v0, v2, M2, stepdistance);

    for (int i = 0; i < M1.size(); i++){
        output.push_back(M1[i]);
    }
    for (int i = 0; i < M2.size(); i++){
        output.push_back(M2[i]);
    }
    
    for (int i = 0; i < M1.size(); i++){
        FVector &first = M1[i];
        for (int j = 0; j < M2.size(); j++){
            FVector &second = M2[j];
            FVector combined = (first + second) / 2.0f;
            output.push_back(combined);
        }
    }
}


///@brief interpolates all points in a stepdistance between points, using a MMatrix output
///where the rotation is to the lookdir of the normal from the 3 passed vertecies / triangle
void baryCentricInterpolator::interpolateAllAsMatrixTransformToVertexAndLookOfNormal(
    int stepdistance, 
    std::vector<MMatrix> &output
){
    std::vector<FVector> vertecies;
    interpolateAll(stepdistance, vertecies);

    MMatrix translation;
    for (int i = 0; i < vertecies.size(); i++)
    {
        FVector &current = vertecies[i];
        translation.setTranslation(current);

        MMatrix TR = translation * rotator; //<-- lese richtung --
        output.push_back(TR);
    }
}





void baryCentricInterpolator::interpolateLinear(
    FVector &start, 
    FVector &end, 
    std::vector<FVector> &output,
    int stepDistance
){
    FVector connect = end - start;
    float scale = FVector::Dist(start, end);
    if (scale <= 1.0f)
    {
        return;
    }

    FVector dir = (connect.GetSafeNormal()) * stepDistance;

    /*
    scale = step * x
    scale / x = step
    1 / x = step / scale //kehrwert
    x = scale / step
    */
    float x = scale / stepDistance;

    //jetzt iterativ
    for (int i = 0; i <= x; i++){
        FVector interpolated = start + dir * i;
        output.push_back(interpolated);
    }
}
