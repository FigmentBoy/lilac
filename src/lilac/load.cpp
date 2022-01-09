#include <Loader.hpp>
#include <Mod.hpp>
#include <CApiMod.hpp>
#include <InternalMod.hpp>
#include <Log.hpp>
#undef snprintf
#include <json.hpp>
#include <ZipUtils.h>

USE_LILAC_NAMESPACE();

Result<Loader::MetaCheckResult> Loader::checkMetaInformation(std::string const& path) {
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
        case 1: return this->checkBySchema<1>(path, &json);
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

template<>
Result<Loader::MetaCheckResult> Loader::checkBySchema<1>(std::string const& path, void* jsonData) {
    nlohmann::json json = *reinterpret_cast<nlohmann::json*>(jsonData);
    if (!json.contains("id")) {
        return Err<>("\"" + path + "\" lacks a Mod ID");
    }

    ModInfo info;

    info.m_path          = path;
    info.m_version       = VersionInfo(json["version"]);
    info.m_id            = json["id"];
    info.m_name          = json["name"];
    info.m_developer     = json["developer"];
    info.m_description   = json["description"];
    info.m_details       = json["details"];
    info.m_credits       = json["credits"];

    bool resolved = false;
    if (json.contains("dependencies")) {
        auto deps = json["dependencies"];
        if (deps.is_array()) {
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
                }
            }
        }
    }

    if (!resolved) {
        auto mod = new UnresolvedMod;
        mod->m_info = info;
        this->m_unresolvedMods.push_back(mod);
    }

    return Ok<MetaCheckResult>({ info, resolved });
}
