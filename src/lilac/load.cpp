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

#define JSON_ASSIGN_IF_CONTAINS_AND_TYPE_FROM(_name_, _from_, _type_)\
    if (json.contains(#_from_) && !json[#_from_].is_null()) {   \
        if (json[#_from_].is_##_type_()) {                      \
            info.m_##_name_ = json[#_from_];                    \
        }                                                       \
    }

#define JSON_ASSIGN_IF_CONTAINS_AND_TYPE(_name_, _type_)        \
    if (json.contains(#_name_) && !json[#_name_].is_null()) {   \
        if (json[#_name_].is_##_type_()) {                      \
            info.m_##_name_ = json[#_name_];                    \
        }                                                       \
    }

#define JSON_ASSIGN_IF_CONTAINS_AND_TYPE_NO_NULL(_name_, _type_)\
    if (json.contains(#_name_)) {                               \
        if (json[#_name_].is_null()) {                          \
            return Err<>(                                       \
                "\"" + path + "\": \"" #_name_ "\" is not "     \
                "of expected type -- expected \""               \
                #_type_ "\", got " +                            \
                json[#_name_].type_name()                       \
            );                                                  \
        }                                                       \
        if (json[#_name_].is_##_type_()) {                      \
            info.m_##_name_ = json[#_name_];                    \
        }                                                       \
    }

#define JSON_ASSIGN_IF_CONTAINS_AND_TYPE_REQUIRED(_name_, _type_)\
    if (json.contains(#_name_)) {                               \
        if (json[#_name_].is_null()) {                          \
            return Err<>(                                       \
                "\"" + path + "\": \"" #_name_ "\" is not "     \
                "of expected type -- expected \""               \
                #_type_ "\", got " +                            \
                json[#_name_].type_name()                       \
            );                                                  \
        }                                                       \
        if (json[#_name_].is_##_type_()) {                      \
            info.m_##_name_ = json[#_name_];                    \
        }                                                       \
    } else {                                                    \
        return Err<>(                                           \
            "\"" + path + "\": Missing required field \""       \
            #_name_ "\""                                        \
        );                                                      \
    }

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
    unsigned long readSize = 0;
    auto read = unzip.getFileData("mod.json", &readSize);
    if (!read || !readSize) {
        return Err<>("\"" + path + "\": Unable to read mod.json");
    }
    nlohmann::json json;
    try {
        json = nlohmann::json::parse(std::string(read, read + readSize));

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

    } catch(nlohmann::json::exception const& e) {
        return Err<>("\"" + path + "\": Unable to parse mod.json - \"" + e.what() + "\"");
    } catch(...) {
        return Err<>("\"" + path + "\": Unable to parse mod.json - Unknown Error");
    }
}

template<>
Result<Loader::MetaCheckResult> Loader::checkBySchema<1>(std::string const& path, void* jsonData) {
    nlohmann::json json = *reinterpret_cast<nlohmann::json*>(jsonData);
    if (!json.contains("id")) {
        return Err<>("\"" + path + "\" lacks a Mod ID");
    }

    if (
        !json.contains("version") ||
        !json["version"].is_string() ||
        !VersionInfo::validate(json["version"])
    ) {
        return Err<>(
            "\"" + path + "\" is either lacking a version field, "
            "or its value is incorrectly formatted (should be "
            "\"vX.X.X\")"
        );
    }

    ModInfo info;

    info.m_path    = path;
    info.m_version = VersionInfo(json["version"]);
    JSON_ASSIGN_IF_CONTAINS_AND_TYPE_REQUIRED(id, string);
    JSON_ASSIGN_IF_CONTAINS_AND_TYPE_REQUIRED(name, string);
    JSON_ASSIGN_IF_CONTAINS_AND_TYPE_REQUIRED(developer, string);
    JSON_ASSIGN_IF_CONTAINS_AND_TYPE(description, string);
    JSON_ASSIGN_IF_CONTAINS_AND_TYPE(details, string);
    JSON_ASSIGN_IF_CONTAINS_AND_TYPE(credits, string);

    #ifdef LILAC_IS_WINDOWS
    JSON_ASSIGN_IF_CONTAINS_AND_TYPE_FROM(binaryName, windowsBinary, string);
    #elif LILAC_IS_MACOS
    JSON_ASSIGN_IF_CONTAINS_AND_TYPE_FROM(binaryName, macosBinary, string);
    #elif LILAC_IS_ANDROID
    JSON_ASSIGN_IF_CONTAINS_AND_TYPE_FROM(binaryName, androidBinary, string);
    #endif

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
