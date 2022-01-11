#pragma once

#include <Loader.hpp>
#include <Mod.hpp>
#include <CApiMod.hpp>
#include <InternalMod.hpp>
#include <Log.hpp>

#ifdef LILAC_IS_WINDOWS

#include <Windows.h>
#include <lilac/windows.hpp>

USE_LILAC_NAMESPACE();

#define TRY_C_AND_MANGLED(_var_, _to_, _c_, _mangled_)                      \
    auto _var_ = reinterpret_cast<_to_>(GetProcAddress(load, _c_));         \
    if (!_var_) {                                                           \
        _var_ = reinterpret_cast<_to_>(GetProcAddress(load, _mangled_));    \
    }

Mod* loadWithCApi(HMODULE load) {
    TRY_C_AND_MANGLED(loadFunc, lilac_c_load, "lilac_c_load", "_lilac_c_load@4");

    if (loadFunc) {
        auto info = new CModInfo;
        auto err = loadFunc(info);
        if (err) {
            // todo: log error in internal plugin
            delete info;
            return nullptr;
        }
        auto mod = new CApiMod(info);

        TRY_C_AND_MANGLED(unloadFunc, lilac_c_unload, "lilac_c_unload", "_lilac_c_unload@0");
        TRY_C_AND_MANGLED(enableFunc, lilac_c_enable, "lilac_c_enable", "_lilac_c_enable@0");
        TRY_C_AND_MANGLED(disableFunc,lilac_c_disable,"lilac_c_disable","_lilac_c_disable@0");

        mod->m_loadFunc     = loadFunc;
        mod->m_unloadFunc   = unloadFunc;
        mod->m_enableFunc   = enableFunc;
        mod->m_disableFunc  = disableFunc;

        delete info;
        return mod;
    }
    return nullptr;
}

Result<Mod*> Loader::loadResolvedMod(std::string const& id) {
    auto ix = this->m_unresolvedMods.begin();
    ModInfo info;
    for (auto const& mod : this->m_unresolvedMods) {
        if (mod->m_info.m_id == id) {
            info = mod->m_info;
        } else { ix++; }
    }
    if (ix == this->m_unresolvedMods.end()) {
        return Err<>("Mod with the ID of " + id + " has not been loaded");
    }
    this->m_unresolvedMods.erase(ix);
    auto load = LoadLibraryA(info.m_path.c_str());
    if (load) {
        Mod* mod = nullptr;

        TRY_C_AND_MANGLED(loadFunc, lilac_load, "lilac_load", "_lilac_load@0");

        if (loadFunc) {
            mod = loadFunc();
        } else {
            mod = loadWithCApi(load);
        }
        if (mod) {
            mod->setup();
            mod->m_enabled = true;
            mod->m_platformInfo = new PlatformInfo { load };
            mod->m_info = info;
            this->m_mods.push_back(mod);
            for (auto const& dep : mod->m_info.m_dependencies) {
                dep.m_loaded->m_parentDependencies.push_back(mod);
            }
            return Ok<Mod*>(mod);
        } else {
            return Err<>("Unable to find load functions for " + info.m_id);
        }
    }
    return Err<>("Unable to load the DLL for \"" + info.m_id + "\"");
}

Result<Mod*> Loader::loadModFromFile(std::string const& path) {
    auto check = this->checkMetaInformation(path);
    if (!check) {
        return check;
    }
    if (!check.value().resolved) {
        return Ok<Mod*>(nullptr);
    }
    return this->loadResolvedMod(check.value().id);
}

#endif
