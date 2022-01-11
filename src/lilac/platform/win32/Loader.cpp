#pragma once

#include <Loader.hpp>
#include <Mod.hpp>
#include <CApiMod.hpp>
#include <InternalMod.hpp>
#include <Log.hpp>
#include <ZipUtils.h>

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
    TRY_C_AND_MANGLED(loadFunc, lilac_c_load, "lilac_c_load", "_lilac_c_load@0");

    if (loadFunc) {
        auto err = loadFunc();
        if (err) {
            // todo: log error in internal plugin
            return nullptr;
        }
        auto mod = new CApiMod();

        TRY_C_AND_MANGLED(unloadFunc, lilac_c_unload, "lilac_c_unload", "_lilac_c_unload@0");
        TRY_C_AND_MANGLED(enableFunc, lilac_c_enable, "lilac_c_enable", "_lilac_c_enable@0");
        TRY_C_AND_MANGLED(disableFunc,lilac_c_disable,"lilac_c_disable","_lilac_c_disable@0");

        mod->m_loadFunc     = loadFunc;
        mod->m_unloadFunc   = unloadFunc;
        mod->m_enableFunc   = enableFunc;
        mod->m_disableFunc  = disableFunc;

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

    auto unzip = ZipFile::ZipFile(info.m_path);

    if (!unzip.isLoaded()) {
        return Err<>("Unable to re-read zip for \"" + id + "\"");
    }

    if (!unzip.fileExists(info.m_binaryName)) {
        return Err<>(
            "Unable to find platform binary under the name \"" +
            info.m_binaryName + "\" in \"" + id + "\""
        );
    }

    auto tempDir = const_join_path_c_str<lilac_directory, lilac_temp_directory>;
    if (!std::filesystem::exists(tempDir)) {
        auto dir = file_utils::createDirectory(tempDir);
        if (!dir) return Err<>(dir.error());
    }
    
    unsigned long size;
    auto data = unzip.getFileData(info.m_binaryName, &size);
    if (!data || !size) {
        return Err<>("Unable to read \"" + info.m_binaryName + "\" for \"" + id + "\"");
    }
    
    static long long g_tempID = 0;

    auto tempPath = 
        std::filesystem::path(lilac_directory) / 
        lilac_temp_directory / 
        ("mod_" + std::to_string(g_tempID++) + ".dll");
    auto wrt = file_utils::writeBinary(tempPath, byte_array(data, data + size));
    if (!wrt) return Err<>(wrt.error());

    auto load = LoadLibraryA(tempPath.string().c_str());
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
        return Err<>(check.error());
    }
    if (!check.value().resolved) {
        return Ok<Mod*>(nullptr);
    }
    return this->loadResolvedMod(check.value().id);
}

#endif
