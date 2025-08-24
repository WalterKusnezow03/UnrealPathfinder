#pragma once

/// 4 bytes
#pragma pack(push, 1) // keine Padding-Bytes
struct STORAGEPLUGIN_API FTerrainVertexIndex
{
    int32 index;     //4 bytes
};
#pragma pack(pop)