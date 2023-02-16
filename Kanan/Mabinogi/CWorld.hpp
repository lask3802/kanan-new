#pragma once
#pragma pack(push, 1)
class CWorld {
public:
    char pad_0[0x160];
    uint64_t localPlayerID; // 0x160
}; // Size: 0x168
#pragma pack(pop)
