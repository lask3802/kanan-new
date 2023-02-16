#pragma once
#pragma pack(push, 1)
class CString {
public:
    uint64_t* vftable; // 0x0
    int32_t referenceCount; // 0x8
    uint32_t allocThreadId; // 0xc
    uint32_t length; // 0x10
    uint32_t capacity; // 0x14
    char pad_18[0x8];
    // Metadata: utf16*
    char16_t buffer[70]; // 0x20
}; // Size: 0xac
#pragma pack(pop)
