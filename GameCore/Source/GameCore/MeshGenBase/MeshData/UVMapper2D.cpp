#include "UVMapper2D.h"
#include "CoreMinimal.h"
#include "GameCore/MeshGenBase/MeshData/BoundingBox/BoundingBoxSimple.h"

void UVMapper2D::generateUVBuffer(
    const TArray<FVector> &vertecies,
    FVector normal,
    TArray<FVector2D> &outUV
){
    //remove rotation from the vertecies
    normal = normal.GetSafeNormal();
    FRotator rotation = normal.Rotation();

    rotation.Pitch += -90.0f; //sodass Z = 1.0f wird, xy pane only

    FMatrix RotationMatrix = FRotationMatrix(rotation);
    FMatrix InverseMatrix = RotationMatrix.Inverse();

    TArray<FVector> bufferOnXYPane;
    bufferOnXYPane.SetNum(vertecies.Num());
    for (int i = 0; i < vertecies.Num(); i++){
        FVector copy = vertecies[i];
        bufferOnXYPane[i] = InverseMatrix.TransformPosition(copy);
    }

    BoundingBoxSimple bounds(bufferOnXYPane);

    outUV.Empty();
    outUV.SetNum(vertecies.Num());
    for (int i = 0; i < outUV.Num(); i++){
        outUV[i] = generateUVCoordinate(bufferOnXYPane[i], bounds);
    }
}


FVector2D UVMapper2D::generateUVCoordinate(
    FVector &vertex,
    BoundingBoxSimple &boundingBox
){
    FVector bl = boundingBox.bottomLeftNearVertex();
    FVector tr = boundingBox.topRightLeftVertex();

    float xMin = bl.X;
    float yMin = bl.Y;

    float xAll = std::abs(tr.X - bl.X);
    float yAll = std::abs(tr.Y - bl.Y);

    if(xAll <= 0.0f || yAll <= 0.0f){
        return FVector2D(0, 0);
    }

    float distTargetX = std::abs(vertex.X - xMin); //AB = B - A
    float distTargetY = std::abs(vertex.Y - yMin);

    //skalar = distTraget / distAll
    //UV ranges from 0 to 1! remember!
    FVector2D uvOut(
        (distTargetX / xAll),
        (distTargetY / yAll)
    );
    return uvOut;
}