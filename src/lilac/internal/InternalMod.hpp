#pragma once

#include <Mod.hpp>

USE_LILAC_NAMESPACE();

class Lilac;

class InternalMod : public Mod {
    protected:
        void setup() override;

        friend class Lilac;

        InternalMod();

    public:
        static InternalMod* get();
};
