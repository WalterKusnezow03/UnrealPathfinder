#pragma once

#include "CoreMinimal.h"
#include <set>
#include "KismetProceduralMeshLibrary.h"
#include "AssetPlugin/gamestart/assetEnums/materialEnum.h"
#include "CoreMath/Matrix/MMatrix.h"
#include "GameCore/MeshGenBase/MeshData/aeroDynamic/Util/ThreeKeyMap.h"
#include "GameCore/MeshGenBase/MeshData/MeshData.h"

class GAMECORE_API AeroMeshData : public MeshData {

public:
    AeroMeshData();
    virtual ~AeroMeshData() override;

    AeroMeshData(TArray<FVector> &&verteciesIn, TArray<int> &&trianglesIn);
	AeroMeshData(const AeroMeshData &other);
	AeroMeshData &operator=(const AeroMeshData &other); //copy constructor

protected:



/**
 * AERO DYNAMIC SECTION
 */
public:

	/// @brief calculates the sum of off all normals
	/// and triangle scales with SUM(pressure * normal * areaOfTriangle)
	/// @param pressure 
	/// @return 
	FVector forceFrom(FVector &windDirectionAndScaledSpeed);
    //FVector forceFrom(FVector &windDir, float airDensity);

    FVector torqueVector(
        FVector &force,
		FVector &offsetFromAirPlaneCenter
    );

	//new
	void forceAndTorqueFrom(
		FVector &windDirectionScaled,
		FVector &offsetFromAirPlaneCenter,
		FVector &outForce,
		FVector &outTorque
	);



protected:
	FVector normalFor(int v0, int v1, int v2);
	float areaOfTriangle(int v0, int v1, int v2);
	float angleBetween(
		FVector &a,
		FVector &b
	);

	FVector forceVector(
		FVector &flowDir,
		int v0,
		int v1,
		int v2,
		float windSpeed
	);

	FVector forceVector(
		FVector &flowDir, // is also drag dir --->  flow <=> drag
		FVector &normalDir,
		float areaOfTriangle,
		float windSpeed
	);

    FVector torqueVector(
        FVector &force,
        FVector &centerOfMass,
        int v0,
        int v1,
        int v2
    );

	float LiftCoefficent(float angle);
	float DragCoefficent(float angle);

    virtual void removeVertex(int index, std::vector<int> &connectedvertecies) override;

    ThreeKeyMap triangleAreas;
};