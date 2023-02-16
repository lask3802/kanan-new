#pragma once
class CItem;
class CCharacter;
#pragma pack(push, 1)
class CEntityList {
public:
    class CItemList {
    public:
        class CCItemListNode {
        }; // Size: 0x0

        class CItemListNode {
        public:
            class CItemListNodeEntry {
            public:
                char pad_0[0x18];
                CItem* item; // 0x18
            }; // Size: 0x20

            CItemListNodeEntry* entry; // 0x0
            CEntityList::CItemList::CItemListNode* next; // 0x8
        }; // Size: 0x10

        float float_0; // 0x0
        char pad_4[0x4];
        CItemListNode* root; // 0x8
        uint32_t count; // 0x10
    }; // Size: 0x14

    class CCharacterList {
    public:
        class CCharacterListNode {
        public:
            class CCharacterListNodeEntry {
            public:
                char pad_0[0x18];
                CCharacter* character; // 0x18
            }; // Size: 0x20

            CCharacterListNodeEntry* entry; // 0x0
            CEntityList::CCharacterList::CCharacterListNode* next; // 0x8
        }; // Size: 0x10

        float float_1; // 0x0
        char pad_4[0x4];
        CCharacterListNode* root; // 0x8
        uint32_t count; // 0x10
    }; // Size: 0x14

    char pad_0[0x10];
    CItemList items; // 0x10
    char pad_24[0x2c];
    CCharacterList characters; // 0x50
}; // Size: 0x64
#pragma pack(pop)
