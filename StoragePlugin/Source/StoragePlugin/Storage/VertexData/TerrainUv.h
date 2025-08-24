#pragma once

/// 12 bytes
#pragma pack(push, 1) // keine Padding-Bytes
struct STORAGEPLUGIN_API FTerrainUV
{
    double Position[2];     //8 * 2 bytes = 16
};
#pragma pack(pop)