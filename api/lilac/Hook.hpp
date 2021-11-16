#pragma once

#include "Macros.hpp"
#include <inttypes.h>

namespace lilac {
    using address_t = uintptr_t;

    class Mod;

    class LILAC_DLL Hook {
        private:
            Mod*  m_owner;
            void* m_address;
            void* m_detour;
            void* m_handle = nullptr;
            bool  m_enabled;

        public:
            address_t getAddress() const { return m_address; }
            bool isEnabled() const { return m_enabled; }
            Mod* getOwner() const { return m_owner; }

            Hook(address_t addr) :
                m_address(reinterpret_cast<void*>(addr)), m_enabled(false) {}
            Hook(void* addr) :
                m_address(addr), m_enabled(false) {}
    };
}