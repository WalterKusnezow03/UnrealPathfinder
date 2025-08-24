// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EntityManagerGeneric.h"
#include <map>


/**
 * enity manager based on a type (aactor) and a plotted enum class, each type will be tracked 
 * in the manager
 */

template <typename E, typename T>
class P2_API EntityManagerGenericMap
{
	//static_assert(std::is_enum<E>::value, "must be an enum");
	//static_assert(std::is_base_of<AActor, T>::value, "must be an aactor");

public:
	EntityManagerGenericMap(){
		
	}

	~EntityManagerGenericMap(){
		for(auto &pair : map){
			EntityManagerGeneric<T> *val = pair.second;
			if(val != nullptr){
				delete val; //delete the created entity manager which wont be accessed anyway from outside. 
			}
		}
	}


	void add(E type, T *actor){
		if(map.find(type) != map.end()){
			EntityManagerGeneric<T> *fromMap = map[type];
			fromMap->add(actor);
		}else{
			EntityManagerGeneric<T> *newManager = new EntityManagerGeneric<T>();
			newManager->add(actor);
			map[type] = newManager;
		}
	}

	T *getFirstActor(E type){
		//DebugHelper::showScreenMessage("GENERIC MAP REQUEST", FColor::Blue);
		if (map.find(type) != map.end())
		{
			EntityManagerGeneric<T> *fromMap = map[type];
			if(fromMap != nullptr && fromMap->hasActorsLeft()){
				//DebugHelper::showScreenMessage("GENERIC MAP REQUEST SUCCESS", FColor::Blue);
				return fromMap->getFirstActor();
			}
		}
	
		return nullptr;
	}

	bool hasActorsLeft(E type){
		if(map.find(type) != map.end()){
			EntityManagerGeneric<T> *fromMap = map[type];
			if(fromMap != nullptr && fromMap->hasActorsLeft()){
				return true;
			}
		}
		return false;
	}

	void erase(E type, T *actor){
		if(map.find(type) != map.end() && actor != nullptr){
			EntityManagerGeneric<T> *pointer = map[type];
			if(pointer != nullptr){
				pointer->erase(actor);
			}
		}
	}

private:
	/// @brief saves one entity manager for each enum value
	std::map<E, EntityManagerGeneric<T> *> map;


};
