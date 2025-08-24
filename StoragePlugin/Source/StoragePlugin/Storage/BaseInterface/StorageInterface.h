#pragma once

#include "CoreMinimal.h"

class STORAGEPLUGIN_API StorageInterface {

public:
    StorageInterface() {};
    virtual ~StorageInterface() {};

protected:
    bool SaveBinaryData(
        FString path,
        TArray<uint8> &Bytes
    );

    bool LoadBinaryData(
        FString Path,
        TArray<uint8> &Bytes
    );

    /// @brief will create a path with the world string, ends with a slash "/": somepath/-appendyourinnerPath-
    FString BaseDir(FString worldLevelName);
};