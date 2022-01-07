#include <Loader.hpp>
#include <Mod.hpp>
#include <CApiMod.hpp>
#include <InternalMod.hpp>
#include <Log.hpp>
#undef snprintf
#include <json.hpp>
#include <ZipUtils.h>

USE_LILAC_NAMESPACE();

Result<bool> Loader::checkMetaInformation(std::string const& path) {
    // Unzip file
    auto unzip = ZipFile::ZipFile(path);
    if (!unzip.isLoaded()) {
        return Err<>("\"" + path + "\": Unable to unzip");
    }
    // Check if mod.json exists in zip
    if (!unzip.fileExists("mod.json")) {
        return Err<>("\"" + path + "\" is missing mod.json");
    }
    // Read mod.json & parse if possible
    auto read = unzip.getFileData("mod.json", nullptr);
    if (!read) {
        return Err<>("\"" + path + "\": Unable to read mod.json");
    }
    auto json = nlohmann::json::parse(read);
    // Free up memory
    delete[] read;
    if (!json.is_object()) {
        return Err<>(
            "\"" + path + "/mod.json\" does not have an "
            "object at root despite expected"
        );
    }

    // Check mod.json target version
    auto schema = 1;
    if (json.contains("lilac") && json["lilac"].is_number_integer()) {
        schema = json["lilac"];
    }
    if (schema < Loader::s_supportedSchemaMin) {
        return Err<>(
            "\"" + path + "\" has a lower target version (" + 
            std::to_string(schema) + ") than this version of "
            "lilac supports (" + std::to_string(Loader::s_supportedSchemaMin) +
            "). You may need to downdate lilac in order to use "
            "this mod."
        );
    }
    if (schema > Loader::s_supportedSchemaMax) {
        return Err<>(
            "\"" + path + "\" has a higher target version (" + 
            std::to_string(schema) + ") than this version of "
            "lilac supports (" + std::to_string(Loader::s_supportedSchemaMax) +
            "). You may need to update lilac in order to use "
            "this mod."
        );
    }
    
    // Handle mod.json data based on target
    switch (schema) {
        case 1: {
            if (!json.contains("id")) {
                return Err<>("\"" + path + "\" lacks a Mod ID");
            }
            if (json.contains("dependencies")) {
                auto deps = json["dependencies"];
                if (deps.is_array()) {
                    auto mod = new UnresolvedMod;
                    mod->m_id = json["id"];
                    mod->m_path = path;

                    for (auto const& dep : deps) {
                        if (dep.is_object() && dep.contains("id") && dep["id"].is_string()) {
                            auto depobj = Dependency {};
                            depobj.m_id = dep["id"];
                            if (dep.contains("version")) {
                                depobj.m_version = VersionInfo(dep["version"]);
                            }
                            if (dep.contains("required")) {
                                depobj.m_required = dep["required"];
                            }
                            depobj.m_loaded = this->getLoadedMod(depobj.m_id);
                            if (depobj.m_loaded) {
                                auto u = vector_utils::select<UnresolvedMod*>(
                                    this->m_unresolvedMods,
                                    [depobj](UnresolvedMod* t) -> bool {
                                        return t->m_id == depobj.m_id;
                                    }
                                );
                                if (u) {
                                    depobj.m_state =
                                        u->hasUnresolvedDependencies() ?
                                            ModResolveState::Unloaded : 
                                            ModResolveState::Resolved;
                                }
                            }
                        }
                    }

                    this->m_unresolvedMods.push_back(mod);
                }
            }
            return Ok<bool>(true);
        } break;
    }

    // Target version was not handled
    return Err<>(
        "\"" + path + "\" has a version schema (" +
        std::to_string(schema) + ") that isn't "
        "supported by this version of lilac. "
        "This may be a bug, or the given version "
        "schema is invalid."
    );
}
