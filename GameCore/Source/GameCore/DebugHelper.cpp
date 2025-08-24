// Fill out your copyright notice in the Description page of Project Settings.

#include "GameCore/util/TVector.h"
#include "DebugHelper.h"

DebugHelper::DebugHelper()
{
}

DebugHelper::~DebugHelper()
{
}

/**
 * 
 * 
 * TESTING 
 * 
 * 
 */
void DebugHelper::Debugtest(UWorld *world){


}

/**
 * 
 *  --- SCREEN MESSAGES ---
 * 
 */

/// @brief showsa screen message and a color
/// @param s 
/// @param color 
void DebugHelper::showScreenMessage(FString s, FColor color){

	//debug
	

	bool check = false;
	if(check){
		FString filterString = "filter string";
		if(s.Contains(filterString) == false ){ 
			return;
		}
	}
	
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, color, s);
	}
}




/// @brief shows a scnreen message in green color
/// @param s string to show
void DebugHelper::showScreenMessage(FString s){
	showScreenMessage(s, FColor::Green);
}



void DebugHelper::showScreenMessage(FString s, int argument, FColor color){
	FString res = s;
	res.Append(FString::Printf(TEXT("%d"), argument));
	showScreenMessage(res, color);
}


/// @brief shows a screen message and a number of choice
/// @param s string
/// @param argument number to concatenate 
void DebugHelper::showScreenMessage(FString s, int argument){
	showScreenMessage(s, argument, FColor::Green);
}


void DebugHelper::showScreenMessage(FString s, float argument){
	FString res = s;
	res.Append(FString::Printf(TEXT("%.2f"), argument));
	showScreenMessage(res, FColor::Green);
}


void DebugHelper::showScreenMessage(FString s, FVector2D a, FVector2D b, FColor color){
	FString sOut = s;
	FString xPart = connect(a.X, b.X);
	FString yPart = connect(a.Y, b.Y);

	sOut = sOut + TEXT("\n x:") +
		   xPart + TEXT("\n y:") +
		   yPart + TEXT("\n");

	showScreenMessage(sOut, color);
}


void DebugHelper::showScreenMessage(FString s, FVector a, FColor color){
	FString aX = FString::Printf(TEXT("( %.2f )"), a.X);
	FString aY = FString::Printf(TEXT("( %.2f )"), a.Y);
	FString aZ = FString::Printf(TEXT("( %.2f )"), a.Z);

	FString sOut = s + TEXT("\n") +
				   aX + TEXT("\n") +
				   aY + TEXT("\n ") +
				   aZ + TEXT("\n");

	showScreenMessage(sOut, color);
}

void DebugHelper::showScreenMessageCompare(FString message, FVector a, FVector b, float maxDistance){
	
	FString aX = FString::Printf(TEXT("( %.2f ) ( %.2f)"), a.X, b.X);
	FString aY = FString::Printf(TEXT("( %.2f ) ( %.2f)"), a.Y, b.Y);
	FString aZ = FString::Printf(TEXT("( %.2f ) ( %.2f)"), a.Z, b.Z);

	FString sOut = message + TEXT("\n") +
				   aX + TEXT("\n") +
				   aY + TEXT("\n ") +
				   aZ + TEXT("\n");

	float distance = FVector::Dist(a, b);
	FColor color = FColor::Red;
	if (distance < maxDistance)
	{
		color = FColor::Green;
		sOut = "OK: " + sOut;
	}else{
		sOut = "ISSUE: " + sOut;
	}
	showScreenMessage(sOut, color);
}





void DebugHelper::showScreenMessage(FString s, FVector a, FVector b, FColor color){

	FString sOut = s;
	FString xPart = connect(a.X, b.X);
	FString yPart = connect(a.Y, b.Y);
	FString zPart = connect(a.Z, b.Z);

	sOut = sOut + TEXT("\n") +
				   xPart + TEXT("\n") +
				   yPart + TEXT("\n") +
				   zPart + TEXT("\n");

	showScreenMessage(sOut, color);
}

FString DebugHelper::connect(int a, int b){
	FString out = FString::Printf(TEXT("( %d"), a);
	out += TEXT(" )( ");
	out += FString::Printf(TEXT("%d )"), b);
	return out;
}

void DebugHelper::showScreenMessage(FVector vec){
	showScreenMessage("vector: ", vec);
}

void DebugHelper::showScreenMessage(FString sOut, FVector vec){
	
	FString xPart = FString::Printf(TEXT("%.2f"), vec.X);
	FString yPart = FString::Printf(TEXT("%.2f"), vec.Y);
	FString zPart = FString::Printf(TEXT("%.2f"), vec.Z);

	sOut = sOut + TEXT("(") +
				   xPart + TEXT("; ") +
				   yPart + TEXT("; ") +
				   zPart + TEXT(" )");

	showScreenMessage(sOut, FColor::Purple);
}




/**
 * 
 * -- log messages --
 * 
 */
void DebugHelper::logMessage(FString printing){
	UE_LOG(LogTemp, Log, TEXT("%s"), *printing); //for whatever reason the string must be dereferenced
}

void DebugHelper::logMessage(FString message, FRotator rot){
	message += FString::Printf(
		TEXT("roll: %.2f; pitch: %.2f, yaw: %.2f"), rot.Roll, rot.Pitch, rot.Yaw
	);
	logMessage(message);
}

void DebugHelper::logMessage(FString message, FVector a, FVector b){
	FString message1 = FString::Printf(
		TEXT("(%.2f %.2f %.2f) -> (%.2f %.2f %.2f)"),
		a.X,
		a.Y,
		a.Z,
		b.X,
		b.Y,
		b.Z
	);
	message += message1;
	logMessage(message);
}




double DebugHelper::timeSum = 0.0;
void DebugHelper::addTime(double time)
{
	timeSum += time;
}
void DebugHelper::logTime(FString message){
	UE_LOG(LogTemp, Log, TEXT("message: %s , %f seconds"), *message, timeSum);
	timeSum = 0.0;
}

void DebugHelper::logMessage(FString s, int a){
	s += TEXT(" ");
	FString iTostring = FString::Printf(TEXT("%d"), a);
	s += iTostring;

	logMessage(s);
}

void DebugHelper::logMessageFloat(FString s, float a){
	s += TEXT(" ");
	FString iTostring = FString::Printf(TEXT("%.2f"), a);
	s += iTostring;

	logMessage(s);
}



void DebugHelper::logMessage(FString s, FVector a){
	s += TEXT(" ");
	s += FVectorToString(a);
	logMessage(s);
}

void DebugHelper::logMessage(FString s, FString t){
	s += t;
	logMessage(s);
}

FString DebugHelper::FVectorToString(FVector vec){
	FString xPart = FString::Printf(TEXT("%.2f"), vec.X);
	FString yPart = FString::Printf(TEXT("%.2f"), vec.Y);
	FString zPart = FString::Printf(TEXT("%.2f"), vec.Z);
	FString sOut = xPart + TEXT(", ") + yPart + TEXT(", ") + zPart;
    return sOut;
}

/**
 * 
 *  --- DRAW LINES ---
 *
 */



void DebugHelper::showLineBetween(UWorld *world, FVector Start, FVector End, FColor color){
	if(world != nullptr){
		DrawDebugLine(world, Start, End, color, false, 100.0f, 0, 1.0f);
	}
}


void DebugHelper::showLineBetween(UWorld *worldin, FVector Start, FVector End, FColor color, float time){
	if(worldin != nullptr){
		DrawDebugLine(worldin, Start, End, color, false, time, 0, 1.0f);
	}
}




/// @brief draws a line between 2 given points
/// @param world world to draw in
/// @param Start start point
/// @param End end point
void DebugHelper::showLineBetween(UWorld *world, FVector Start, FVector End){
	showLineBetween(world, Start, End, FColor::Blue);
}




void DebugHelper::showLine(UWorld *world, std::vector<FVector> &vec){
	showLine(world, vec, FColor::Green);
}

/// @brief draw line for a complete set of points
/// @param world world to draw in
/// @param vec all points to connect in order
void DebugHelper::showLine(UWorld *world, std::vector<FVector> &vec, FColor color){
	if(world != nullptr){
		for(int i = 1; i < vec.size(); i++){
			showLineBetween(world, vec.at(i - 1), vec.at(i), color);
		}
	}
}




void DebugHelper::showLine(UWorld *world, TArray<FVector> &array){
	showLine(world, array, FColor::Green);
}


void DebugHelper::showLine(UWorld *world, TArray<FVector> &array, FColor color){
	if(world != nullptr){
		for(int i = 1; i < array.Num(); i++){
			showLineBetween(world, array[i-1], array[i], color);
		}
	}
}




void DebugHelper::showLine(UWorld *world, std::vector<FVector> &vec, FColor color, int scale){
	if(world != nullptr){
		for(int i = 1; i < vec.size(); i++){
			showLineBetween(world, vec[i-1] * scale, vec[i] * scale, color);
		}
	}
}

/// @brief shows a line with a certain display time
/// @param world 
/// @param vec 
/// @param color 
/// @param scale 
/// @param time 
void DebugHelper::showLine(UWorld *world, std::vector<FVector> &vec, FColor color, float time){
	if(world != nullptr){
		for(int i = 1; i < vec.size(); i++){
			showLineBetween(world, vec[i-1], vec[i], color, time); //with display time
		}
	}
}





/**
 * 
 * 2D draw
 * 
 */

void DebugHelper::showLineBetween(UWorld *world, FVector2D Start, FVector2D End){
	showLineBetween(world, Start, End, 100);
}

void DebugHelper::showLineBetween(UWorld *world, FVector2D Start, FVector2D End, int zOffset){
	showLineBetween(world, Start, End, zOffset, FColor::Blue);
}

void DebugHelper::showLineBetween(UWorld *world, FVector2D Start, FVector2D End, int zOffset, FColor color){
	FVector sA(
		Start.X,
		Start.Y,
		zOffset);
	FVector eB(
		End.X,
		End.Y,
		zOffset);
	showLineBetween(world, sA, eB, color);
}
