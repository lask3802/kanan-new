#pragma once
class CString;
#pragma pack(push, 1)
class CItem {
public:
    struct SItemDBDesc {
        uint64_t* vftable; // 0x0
        CString* type; // 0x8
        char pad_10[0x18];
        CString* str_28; // 0x28
        char pad_30[0x20];
        CString* displayName; // 0x50
        char pad_58[0x8];
        CString* name; // 0x60
        CString* description; // 0x68
        char pad_70[0x4c];
        uint16_t maxStackCount; // 0xbc
    }; // Size: 0xbe

    uint64_t* vftable; // 0x0
    uint64_t* entityID; // 0x8
    uint32_t inventoryID; // 0x10
    uint32_t itemID; // 0x14
    uint32_t color1; // 0x18
    uint32_t color2; // 0x1c
    uint32_t color3; // 0x20
    char pad_24[0x4];
    uint32_t color4; // 0x28
    uint32_t color5; // 0x2c
    uint32_t color6; // 0x30
    uint32_t stackCount; // 0x34
    char pad_38[0x4];
    uint32_t positionX; // 0x3c
    uint32_t positionY; // 0x40
    char pad_44[0x30];
    uint32_t price; // 0x74
    uint32_t sellPrice; // 0x78
    char pad_7c[0x4];
    uint32_t durability; // 0x80
    uint32_t maxDurability; // 0x84
    uint32_t originalDurability; // 0x88
    char pad_8c[0x74];
    SItemDBDesc* dbDesc; // 0x100
    char pad_108[0x28];
    uint64_t ownerID; // 0x130
    char pad_138[0x38];
    CString* name; // 0x170
}; // Size: 0x178
#pragma pack(pop)
