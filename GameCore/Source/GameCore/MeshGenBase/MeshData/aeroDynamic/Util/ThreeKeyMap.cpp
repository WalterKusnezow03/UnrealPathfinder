#include "ThreeKeyMap.h"
#include "CoreMinimal.h"


ThreeKeyMap::ThreeKeyMap(){

}

ThreeKeyMap::~ThreeKeyMap(){

}

/// @brief caution: key is independant of order of params, one key for 3 values, no matte the order
/// @param v0 
/// @param v1 
/// @param v2 
/// @param outValue 
/// @return 
bool ThreeKeyMap::findValue(int v0, int v1, int v2, float &outValue){

    std::vector<FString> allKeys = createAllKeys(v0, v1, v2);
    for (int i = 0; i < allKeys.size(); i++){
        FString &key = allKeys[i];
        if(stringKeyMap.find(key) != stringKeyMap.end()){
            outValue = stringKeyMap[key];
            return true;
        }
    }

    //none found:
    return false;
}

/// @brief caution: key is independant of order of params, one key for 3 values, no matte the order
/// @param v0 
/// @param v1 
/// @param v2 
/// @param value 
void ThreeKeyMap::replaceValue(int v0, int v1, int v2, float value){

    std::vector<FString> allKeys = createAllKeys(v0, v1, v2);
    for (int i = 0; i < allKeys.size(); i++){
        FString &key = allKeys[i];
        if(stringKeyMap.find(key) != stringKeyMap.end()){
            stringKeyMap[key] = value;
            return;
        }
    }

    //none found: create
    stringKeyMap[allKeys[0]] = value;
}




std::vector<FString> ThreeKeyMap::createAllKeys(int v0, int v1, int v2){
    FString key0 = createKey(v0, v1, v2);
    FString key1 = createKey(v0, v2, v1);
    FString key2 = createKey(v1, v0, v2);
    FString key3 = createKey(v1, v2, v0);
    FString key4 = createKey(v2, v0, v1);
    FString key5 = createKey(v2, v1, v0);

    //3! = 6
    std::vector<FString> output = {
        key0, key1, key2, key3, key4, key5
    };
    return output;
}

FString ThreeKeyMap::createKey(int v0, int v1, int v2){
    FString key = FString::Printf(TEXT("_%d_%d_%d_"), v0, v1, v2);
    return key;
}


/// @brief removes all values having the passed key inside the complete key
/// @param v0 key having
void ThreeKeyMap::removeValue(int v0){
    FString subKey = FString::Printf(TEXT("_%d_"), v0);

    for(auto &pair : stringKeyMap){
        FString key = pair.first;
        if(key.Contains(subKey)){
            pair.second = 0.0f;
        }
    }
}

void ThreeKeyMap::removeValue(int v0, int v1, int v2){
    std::vector<FString> keys = createAllKeys(v0, v1, v2);
    for(auto &pair : stringKeyMap){
        FString key = pair.first;

        for (int i = 0; i < keys.size(); i++){
            FString checkKey = keys[i];
            if(checkKey == key){
                pair.second = 0.0f;
            }
        }
    }
}