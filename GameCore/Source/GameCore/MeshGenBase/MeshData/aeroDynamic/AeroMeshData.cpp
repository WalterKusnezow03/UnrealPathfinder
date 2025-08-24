
#include "AeroMeshData.h"

#include "CoreMinimal.h"
#include "GameCore/DebugHelper.h"




AeroMeshData::AeroMeshData(){

}


/// @brief creates the mesh from triangles and vertecies AND calculates the normals!
/// @param verteciesIn 
/// @param trianglesIn 
AeroMeshData::AeroMeshData(TArray<FVector> &&verteciesIn, TArray<int> &&trianglesIn){
    setVertecies(MoveTemp(verteciesIn));
    setTriangles(MoveTemp(trianglesIn));
    calculateNormals();
}


/// @brief copy constructor gets called if passing by value
/// @param other 
AeroMeshData::AeroMeshData(const AeroMeshData &other){
    *this = other;
}

/// @brief abusing the assign operator to create a deepcopy of the data
/// @param other 
/// @return 
AeroMeshData& AeroMeshData::operator=(const AeroMeshData &other){
    if(&other != this){
        clearMesh(); //clear mesh data before adding any data!

        for (int i = 0; i < other.vertecies.Num(); i++){
            vertecies.Add(other.vertecies[i]);
        }
        for (int i = 0; i < other.triangles.Num(); i++){
            triangles.Add(other.triangles[i]);
        }
        for (int i = 0; i < other.normals.Num(); i++){
            normals.Add(other.normals[i]);
        }
        for (int i = 0; i < other.UV0.Num(); i++){
            UV0.Add(other.UV0[i]);
        }
        for (int i = 0; i < other.Tangents.Num(); i++){
            Tangents.Add(other.Tangents[i]);
        }
        for (int i = 0; i < other.VertexColors.Num(); i++){
            VertexColors.Add(other.VertexColors[i]);
        }

        materialPreferred = other.materialPreferred;
        triangleAreas = other.triangleAreas;
    }
    return *this;
}


AeroMeshData::~AeroMeshData(){
    clearMesh();

    //clear triangles saved

}







/**
 * 
 * ---------------------------------------------------------------------------------------
 * 
 * AERO DYNAMIC SECTION
 * 
 * ---------------------------------------------------------------------------------------
 * 
 */

void AeroMeshData::forceAndTorqueFrom(
    FVector &windDirectionScaled,
    FVector &offsetFromAirPlaneCenter,
    FVector &outForce,
    FVector &outTorque
){
    FVector flow = windDirectionScaled * -1.0f; //auftreff punkt wind ist aus entgegengesetzer richtung, wie normale wegschaut.
    float windSpeedCms = flow.Size();
    FVector flowDir = flow.GetSafeNormal();



    //FVector totalForce;
    for (int i = 2; i < triangles.Num(); i += 3){
        int v0 = triangles[i - 2];
        int v1 = triangles[i - 1];
        int v2 = triangles[i];
        FVector currentForce = forceVector(flowDir, v0, v1, v2, windSpeedCms);
        FVector currentTorque = torqueVector(
            currentForce, 
            offsetFromAirPlaneCenter,
            v0,
            v1,
            v2
        );
        
        outForce += currentForce;
        outTorque += currentTorque;
    }

}










/**
 * OLD
 */




/// @brief wind direction must be relative to the object AND its movement!
/// @param windDirectionAndScaledSpeed 
/// @return force in CM 
FVector AeroMeshData::forceFrom(FVector &windDirectionAndScaledSpeed){
    FVector flow = windDirectionAndScaledSpeed * -1.0f; //auftreff punkt wind ist aus entgegengesetzer richtung, wie normale wegschaut.
    float windSpeedCms = flow.Size();



    FVector flowDir = flow.GetSafeNormal();



    //total force is sum of all triangle forces
    FVector totalForce;
    for (int i = 2; i < triangles.Num(); i += 3){
        int v0 = triangles[i - 2];
        int v1 = triangles[i - 1];
        int v2 = triangles[i];

        totalForce += forceVector(flowDir, v0, v1, v2, windSpeedCms);
    }

    return totalForce;
}



FVector AeroMeshData::forceVector(
    FVector &flowDir,
    int v0, 
    int v1, 
    int v2,
    float windSpeed
){
    FVector normal = normalFor(v0, v1, v2);
    float areaOfTriangleFound = areaOfTriangle(v0, v1, v2);
    //DebugHelper::logMessage("areaOftriangle ", areaOfTriangleFound); //ok

    return forceVector(flowDir, normal, areaOfTriangleFound, windSpeed);
}

/// @brief calculates the force vector for a given triangle, air speed, resulting air pressure,
/// and the angle to the triangle 
/// @param flowDir 
/// @param normalDir 
/// @param areaOfTriangle 
/// @param windSpeed needed to calculate the airpressure
/// @return 
FVector AeroMeshData::forceVector(
    FVector &flowDir, //is also drag dir --->  flow <=> drag
    FVector &normalDir,
    float areaOfTriangle,
    float windSpeed 
){


    
    FVector spannVektor = FVector::CrossProduct(flowDir, normalDir); //seiten vektor von flow und normal dir

    FVector liftDir = FVector::CrossProduct(spannVektor, flowDir).GetSafeNormal(); //nach oben
    FVector dragDir = flowDir;

    //pressure = 1/2 * air * v^2 nach bernoullie bro
    float airDensityKGM3 = 1.225f; //Standard wert auf meeres höhe in kubik METER
    float airDensityCM3 = airDensityKGM3 / (100 * 100 * 100);//von SI-Meter zu CM
    float q_pressure = 0.5f * airDensityCM3 * windSpeed * windSpeed; //ekin = 0.5 m v^2

    DebugHelper::showScreenMessage("air pressure ", q_pressure);


    
    //für pressure notwendig weil wenn = 0: kein pressure,
    //wenn = -1 full pressure, anhand normale und flow dir bestimmt, 
    //ganz einfach! 
    //Wie normale und flow dir zu einander stehen, das bestimmt die pressure
    //und somit den auftrieb, wenn paralell nichts (dot = 0), wenn blockade (dot bis 1): auftrieb!
    float angle = angleBetween(flowDir, normalDir);

    //lift kooeffizienten cappen bei 45 grad (2*alpha doppelt so schnell bei 90, sin(pi/2) = 1)
    //maximaler lift bei 45 grad, rest abwärts zu 0
    float liftCoefficent = LiftCoefficent(angle);//std::sin(2 * angle); // lift coefficent
    float dragCoefficent = DragCoefficent(angle);



    FVector lift = liftDir * (liftCoefficent * q_pressure * areaOfTriangle); //lift force
    FVector drag = dragDir * (dragCoefficent * q_pressure * areaOfTriangle); //drag force
    FVector totalForce = lift + drag;

    DebugHelper::showScreenMessage("lift ", lift);
    DebugHelper::showScreenMessage("drag ", drag);

    return totalForce;
}


/// @brief returns the angle between two vectors in radians
/// @param a 
/// @param b 
/// @return 
float AeroMeshData::angleBetween(
    FVector &a,
    FVector &b
){
    //cos(theta) = a dot b wenn a und b normalisiert sind
    float cosAlpha = FVector::DotProduct(a.GetSafeNormal(), b.GetSafeNormal());
    float alpha = std::acosf(cosAlpha);
    return alpha;
}




float AeroMeshData::LiftCoefficent(float angle){
    //old
    //lift kooeffizienten cappen bei 45 grad (2*alpha doppelt so schnell bei 90, sin(pi/2) = 1)
    //maximaler lift bei 45 grad, rest abwärts zu 0
    
    float liftCoefficent = std::sin(2 * angle); // lift coefficent
    return liftCoefficent;
    
    
    //new
    const float MaxAngle = FMath::DegreesToRadians(15.0f); // Stall ab 15°
    const float StallDropoffRate = 2.0f; // Wie schnell der Lift nach Stall abfällt
    
    if (FMath::Abs(angle) <= MaxAngle)
    {
        // Bis Stall linear steigen
        return angle / MaxAngle;
    }
    else
    {
        // Nach Stall exponentiell oder linear sinken
        float overshoot = FMath::Abs(angle) - MaxAngle;
        return FMath::Clamp(1.0f - StallDropoffRate * overshoot, 0.0f, 1.0f);
    }
}


float AeroMeshData::DragCoefficent(float angle){
    //float sinAlpha = std::sin(angle); //umkehren der skalarprodukt logik
    //float sin_a_pow2 = sinAlpha * sinAlpha;
    
    /*
    idee: wenn skalarprodukt = 1, maximaler drag, flugzeug pendelt sich bei 390 kmh ein.
    */
    if(true){
        float dragCoefficent = std::cos(std::abs(angle));
        return dragCoefficent;
    }else{
        //angepasset funktion:
        float degSlowDown = std::sin(M_PI / 6.0f); //45 grad bei pi / 4
        float dragCoefficent = (std::cos(std::abs(angle)))- degSlowDown;


        return dragCoefficent;
    }
    


    
}







/**
 * -- dreh moment (torque extraction) --
 */
FVector AeroMeshData::torqueVector(
    FVector &force,
    FVector &offsetFromAirPlaneCenter
){
    FVector totalDrehMoment(0, 0, 0);

    // zu jedem auftreff punkt der kraft wird ein moment berechnet
    // aus M = r x F
    // wobei r = targetHit - centerOfMass; ist 

    for (int i = 2; i < triangles.Num(); i+=3){
        //der total dreh moment ist die summe aus allen momentums
        totalDrehMoment += torqueVector(
            force,
            offsetFromAirPlaneCenter,
            triangles[i - 2],
            triangles[i - 1],
            triangles[i]
        );
    }
    return totalDrehMoment;
}

FVector AeroMeshData::torqueVector(
    FVector &force,
    FVector &relativeLocationMesh,
    int v0,
    int v1,
    int v2
){
    if(isValidVertexIndex(v0, v1, v2)){
        FVector centerOfTriangle = vertecies[v0] + vertecies[v1] + vertecies[v2];
        centerOfTriangle /= 3.0f;

        //M is torque momentum
        //M = r x F ----> r vector kann auch in tabelle gespeichert werden
        FVector centerOfMassToHittedPoint = relativeLocationMesh + centerOfTriangle; //verbidungs vektor zum vertex insgesamt
        FVector torque = FVector::CrossProduct(centerOfMassToHittedPoint, force);

        return torque;
    }
    return FVector(0, 0, 0);
}


/// @brief finds the normal for a given triangle
/// @param v0 
/// @param v1 
/// @param v2 
/// @return normal for the given triangle
FVector AeroMeshData::normalFor(int v0, int v1, int v2){
    FVector normal(0,0,1);
    if(isValidNormalIndex(v0,v1,v2)){
        FVector sumNormal = normals[v0] + normals[v1] + normals[v2];
        normal = sumNormal.GetSafeNormal();
    }
    if(isValidVertexIndex(v0,v1,v2)){
        FVector &a = vertecies[v0];
        FVector &b = vertecies[v1];
        FVector &c = vertecies[v2];

        FVector ab = b - a;
        FVector ac = c - a;
        normal = FVector::CrossProduct(ab, ac).GetSafeNormal();
    }
    return normal;
}

/// @brief finds the area A for a given triangle
/// @param v0 
/// @param v1 
/// @param v2 
/// @return 
float AeroMeshData::areaOfTriangle(int v0, int v1, int v2){
    if(isValidVertexIndex(v0,v1,v2)){

        float areaOut = 0.0f;
        if(triangleAreas.findValue(v0,v1,v2, areaOut)){
            return areaOut;
        }

        //create instead
        FVector &a = vertecies[v0];
        FVector &b = vertecies[v1];
        FVector &c = vertecies[v2];

        FVector ab = b - a;
        FVector ac = c - a;

        float area = 0.5f * FVector::CrossProduct(ab, ac).Size();//länge der paralellogramm fäche / 2 = dreieck

        triangleAreas.replaceValue(v0, v1, v2, area); //replace key intern

        return area;
    }
    return 0.0f;
}



/**
 * 
 * --- overriden ---
 * 
 */
void AeroMeshData::removeVertex(int index, std::vector<int> &connectedvertecies){
    MeshData::removeVertex(index, connectedvertecies); //super

    //remove from area map
    triangleAreas.removeValue(index);
}