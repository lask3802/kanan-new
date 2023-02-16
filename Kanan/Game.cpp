#include <Scan.hpp>
#include <String.hpp>

#include "Log.hpp"
#include "Game.hpp"

using namespace std;

namespace kanan {
    Game::Game()
        : m_rendererPtr{ nullptr },
        m_entityListPtr{ nullptr }, 
        m_worldPtr{nullptr}, 
        m_accountPtr{nullptr}
    {
        log("Entering Game constructor.");

        // Find the games global renderer pointer.
        const auto x64renderAddressPattern = "4C 8D 4D 48 4C 8D 45 40 BA ? ? ? ? 48 8B 0D ? ? ? ?"s;
        auto rendererAddress = scan("client.exe", x64renderAddressPattern);

        if (rendererAddress) {
            //address = offset + rip
            auto offset = *(uint32_t*)(*rendererAddress + 16);
            auto rip = (*rendererAddress + 20);
            m_rendererPtr = (CRendererPtr*)(offset + rip);

            log("Got CRendererPtr %p, offset %p, rip %p", m_rendererPtr, offset, rip);
        }
        else {
            error("Failed to find address of CRendererPtr.");
        }

        // Find the games global entity list pointer.
        const auto x64EntityListPattern = "48 8B DA 4C 8B F9 48 8B 0D ? ? ? ? E8 ? ? ? ? 48 85 C0"s;
        auto entityListAddress = scan("client.exe", x64EntityListPattern);

        if (entityListAddress) {                                                                
            m_entityListPtr = (CEntityListPtr*)(*(uint32_t*)(*entityListAddress + 9) + (*entityListAddress + 13));

            log("Got CEntityListPtr %p", m_entityListPtr);
        }
        else {
            error("Failed to find CEntityListPtr.");
        }

        // Find the games global world pointer.
        const auto x64WorldPattern = "48 8B 05 ? ? ? ? 48 8B 48 38 E8 ? ? ? ?"s;
        auto worldAddress = scan("client.exe", x64WorldPattern);

        if (worldAddress) {
            m_worldPtr = (CWorldPtr*)(*(uint32_t*)(*worldAddress + 3) + (*worldAddress + 7));

            log("Got CWorldPtr %p", m_worldPtr);
        }
        else {
            error("Failed to find CWorldPtr.");
        }

         // Find the games global account pointer.
        const auto x64AccountPattern = "48 8B 0D ? ? ? ? 48 8D 97 18 02 00 00 45 33 C9 45 33 C0 E8 ? ? ? ?";
        auto accountAddress = scan("client.exe", x64AccountPattern);

        if (accountAddress) {
            m_accountPtr = (CAccountPtr*)(*(uint32_t*)(*accountAddress + 3) + (*accountAddress + 7));

            log("Got CAccountPtr %p", m_accountPtr);
        }
        else {
            error("Failed to find CAccountPtr.");
        }

        log("Leaving Game constructor.");
    }

    KCharacter* Game::getCharacterByID(uint64_t id) {
        auto entityList = getEntityList();

        if (entityList == nullptr) {
            return nullptr;
        }

        // Go through the entity list looking for a character with the matching
        // ID.
        auto& characters = entityList->characters;
        auto highestIndex = characters.count;
        auto node = characters.root;

        for (uint32_t i = 0; i <= highestIndex && node != nullptr; ++i, node = node->next) {
            auto character = (KCharacter*)node->entry->character;

            if (character == nullptr) {
                continue;
            }

            auto characterID = character->getID();

            if (!characterID || *characterID != id) {
                continue;
            }

            return character;
        }

        // If we get here then no character with the specified ID is in the list.
        return nullptr;
    }

    KCharacter* Game::getLocalCharacter() {
        auto world = getWorld();

        if (world == nullptr) {
            return nullptr;
        }

        return getCharacterByID(world->localPlayerID);
    }

    void Game::changeChannel(int channel) {
        static auto cc = (char(__thiscall*)(void*, CString**, char, char))scan("client.exe", "55 8B EC 6A ? 68 ? ? ? ? 64 A1 ? ? ? ? 50 83 EC ? 53 56 57 A1 ? ? ? ? 33 C5 50 8D 45 F4 64 A3 ? ? ? ? 8B D9 A1 ? ? ? ? 83 78 20 ?").value_or(0);
        static auto logged = false;

        if (!logged) {
            log("Change channel function = %p", cc);
            logged = true;
        }
        //patching to allow to work on nao
        if (channel < 1 || channel > 10) {
            return;
        }

        auto account = getAccount();

        if (!account) {
            return;
        }

        log("Account = %p", account);

        string str{ "Ch" };

        str += to_string(channel);

        log("Channel = %s", str.c_str());

        CString val{};

        // CMessage::WriteString only needs these members.
        val.capacity = sizeof(val.buffer) / 2;
        val.length = str.length();
        val.referenceCount = 1;

        wcscpy_s((wchar_t*)val.buffer, val.capacity, widen(str).c_str());

        auto val2 = &val;

        cc(account, &val2, 0, 0);
    }

    CRenderer* Game::getRenderer() const {
        if (m_rendererPtr == nullptr || m_rendererPtr->renderer == nullptr) {
            return nullptr;
        }

        return m_rendererPtr->renderer;
    }

    CEntityList* Game::getEntityList() const {
        if (m_entityListPtr == nullptr || m_entityListPtr->entityList == nullptr) {
            return nullptr;
        }

        return m_entityListPtr->entityList;
    }

    CWorld* Game::getWorld() const {
        if (m_worldPtr == nullptr || m_worldPtr->world == nullptr) {
            return nullptr;
        }

        return m_worldPtr->world;
    }

    CAccount* Game::getAccount() const {
		if (m_accountPtr == nullptr || m_accountPtr->account == nullptr) {
			return nullptr;
		}

		return m_accountPtr->account;
    }
    }
