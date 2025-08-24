#pragma once


#pragma pack(push, 1) // keine Padding-Bytes
struct STORAGEPLUGIN_API FTerrainNormal
{
    double Normal[3];     //24 bytes (3 * 8)
};
#pragma pack(pop)