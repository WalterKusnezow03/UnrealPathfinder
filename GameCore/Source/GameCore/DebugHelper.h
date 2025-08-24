// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameCore/util/TVector.h"

/**
 * 
 */
class GAMECORE_API DebugHelper
{
public:
	DebugHelper();
	~DebugHelper();

	//debugging
	static void Debugtest(UWorld *world);

	//Messages
	static void showScreenMessage(FString s);
	static void showScreenMessage(FString s, FColor color);
	static void showScreenMessage(FString s, int argument);
	static void showScreenMessage(FString s, float argument);
	static void showScreenMessage(FString s, int argument, FColor color);
	static void showScreenMessage(FString s, FVector a, FColor color);
	static void showScreenMessage(FString s, FVector a, FVector b, FColor color);
	static void showScreenMessage(FString s, FVector2D a, FVector2D b, FColor color);
	static void showScreenMessage(FVector vec);
	static void showScreenMessage(FString s, FVector vec);

	static void showScreenMessageCompare(FString message, FVector a, FVector b, float maxDistance);

	//Log Messages
	static void logMessage(FString s);
	static void logMessage(FString s, int a);
	static void logMessageFloat(FString s, float a);
	static void logMessage(FString s, FRotator other);
	static void logMessage(FString s, FVector a, FVector b);
	static void logMessage(FString s, FVector a);
	static void logMessage(FString s, FString t);
	static FString FVectorToString(FVector vec);

	//time
	static double timeSum;
	static void addTime(double time);
	static void logTime(FString message);

	//FVector
	static void showLineBetween(UWorld *world, FVector Start, FVector End);
	static void showLineBetween(UWorld *world, FVector Start, FVector End, FColor color);
	static void showLineBetween(UWorld *worldin, FVector Start, FVector End, FColor color, float time);
	static void showLine(UWorld *world, std::vector<FVector> &vec);
	static void showLine(UWorld *world, std::vector<FVector> &vec, FColor color);
	static void showLine(UWorld *world, std::vector<FVector> &vec, FColor color, int scale);
	static void showLine(UWorld *world, std::vector<FVector> &vec, FColor color, float time);
	static void showLine(UWorld *world, TArray<FVector> &vec);
	static void showLine(UWorld *world, TArray<FVector> &vec, FColor color);



	//Fvector 2D
	static void showLineBetween(UWorld *world, FVector2D Start, FVector2D End);
	static void showLineBetween(UWorld *world, FVector2D Start, FVector2D End, int zOffset);
	static void showLineBetween(UWorld *world, FVector2D Start, FVector2D End, int zOffset, FColor color);

private:
	static FString connect(int a, int b);

};
