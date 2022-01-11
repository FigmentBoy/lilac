#include <Loader.hpp>
#include <Mod.hpp>
#include <CApiMod.hpp>
#include <InternalMod.hpp>
#include <Log.hpp>
#undef snprintf
#include <json.hpp>
#include <ZipUtils.h>

USE_LILAC_NAMESPACE();

template<> Result<Loader::MetaCheckResult> Loader::checkBySchema<1>(std::string const& path, void* jsonData);

Result<Loader::MetaCheckResult> Loader::checkMetaInformation(std::string const& path) {
    InternalMod::get()->log() << "unzipping " << path << lilac::endl;
    // Unzip file
    auto unzip = ZipFile::ZipFile(path);
    if (!unzip.isLoaded()) {
        InternalMod::get()->log() << "not unzipped " << path << lilac::endl;
        return Err<>("\"" + path + "\": Unable to unzip");
    }
    // Check if mod.json exists in zip
    if (!unzip.fileExists("mod.json")) {
        return Err<>("\"" + path + "\" is missing mod.json");
    }
    InternalMod::get()->log() << "reading mod.json for " << path << lilac::endl;
    // Read mod.json & parse if possible
    auto read = unzip.getFileData("mod.json", nullptr);
    if (!read) {
        InternalMod::get()->log() << "actually not reading mod.json for " << path << lilac::endl;
        return Err<>("\"" + path + "\": Unable to read mod.json");
    }
    InternalMod::get()->log() << "parsing mod.json for " << path << lilac::endl;
    nlohmann::json json;
    try { json = nlohmann::json::parse(read); }
    catch(nlohmann::json::parse_error const& e) {
        return Err<>("\"" + path + "\": Unable to parse mod.json - \"" + e.what() + "\"");
    } catch(...) {
        return Err<>("\"" + path + "\": Unable to parse mod.json - Unknown Error");
    }
    // Free up memory
    InternalMod::get()->log() << "freeing memory for " << path << lilac::endl;
    delete[] read;
    if (!json.is_object()) {
        return Err<>(
            "\"" + path + "/mod.json\" does not have an "
            "object at root despite expected"
        );
    }
    InternalMod::get()->log() << "checkig mod json for " << path << lilac::endl;

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
                    depobj.m_unresolved = this->getUnresolvedMod(depobj.m_id);
                    info.m_dependencies.push_back(depobj);
                }
            }
        }
    }
    info.updateDependencyStates();
    bool resolved = info.hasUnresolvedDependencies();

    auto mod = new UnresolvedMod;
    mod->m_info = info;
    this->m_unresolvedMods.push_back(mod);

    return Ok<MetaCheckResult>({ info.m_id, resolved });
}
