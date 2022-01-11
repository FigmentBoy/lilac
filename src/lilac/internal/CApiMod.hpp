#pragma once

#include <Mod.hpp>

USE_LILAC_NAMESPACE();

class CApiMod : public Mod {
    // these are only accessed by internals
    // anyway, so let's just make them public
    // instead of friends
    public:
        lilac_c_load    m_loadFunc    = nullptr;
        lilac_c_unload  m_unloadFunc  = nullptr;
        lilac_c_enable  m_enableFunc  = nullptr;
        lilac_c_disable m_disableFunc = nullptr;

    protected:
        void setup() override;
        void enable() override;
        void disable() override;

        virtual ~CApiMod();

    public:
        CApiMod();
};

