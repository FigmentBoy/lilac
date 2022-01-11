#pragma once

#include "macros.hpp"
#include "types.hpp"
#include <string_view>
#include <vector>
#include <string>

class Lilac;

namespace lilac {
    #pragma warning(disable: 4251)

    static constexpr const std::string_view lilac_directory          = "lilac";
    static constexpr const std::string_view lilac_mod_directory      = "mods";
    static constexpr const std::string_view lilac_resource_directory = "resources";
    static constexpr const std::string_view lilac_mod_extension      = ".lilac";

    class Mod;
    class Hook;
    class LogStream;
    class LogMessage;
    struct UnresolvedMod;
    struct ModInfo;

    class LILAC_DLL Loader {
    protected:
        std::vector<Mod*> m_mods;
        std::vector<LogMessage*> m_logs;
        std::unordered_map<std::string, ModResolveState> m_resolveStates;
        std::vector<UnresolvedMod*> m_unresolvedMods;
        LogStream* m_logStream;
        bool m_isSetup = false;

        /**
         * Lowest supported mod version.
         * Any mod targeting a lilac version 
         * lower than this will not be loaded, 
         * as they will be considered out-of-date.
         */
        static constexpr const int s_supportedSchemaMin = 1;
        /**
         * Highest support mod version.
         * Any mod targeting a lilac version 
         * higher than this will not be loaded, 
         * as a higher version means that 
         * the user's lilac is out-of-date, 
         * or that the user is a time traveller 
         * and has downloaded a mod from the 
         * future.
         */
        static constexpr const int s_supportedSchemaMax = 1;

        Loader();
        virtual ~Loader();

        struct MetaCheckResult {
            std::string id;
            bool resolved;
        };
        
        /**
         * This function is to avoid ridiculous 
         * indentation in `checkMetaInformation`
         * 
         * The json parameter is void* because 
         * I don't want to force-include json 
         * for every lilac user and forward-
         * declaring the template-full basic_json 
         * class looks horrifying
         * 
         * This function is only used in one place 
         * anyway, only by me who knows how to 
         * use it, so who cares
         */
        template<int Schema>
        Result<MetaCheckResult> checkBySchema(std::string const& path, void* json);

        Result<MetaCheckResult> checkMetaInformation(std::string const& file);
        Result<Mod*> loadResolvedMod(std::string const& id);
        Result<Mod*> loadModFromFile(std::string const& file);
        void createDirectories();

        void updateAllDependencies();

        friend class Mod;
        friend class CustomLoader;
        friend class Lilac;
        friend struct ModInfo;
        
    public:
        static Loader* get();
        bool setup();
        size_t updateMods();

        LogStream& logStream();
        void log(LogMessage* log);
        void deleteLog(LogMessage* log);
        std::vector<LogMessage*> const& getLogs() const;
        std::vector<LogMessage*> getLogs(
            std::initializer_list<Severity> severityFilter
        );

        bool isModLoaded(std::string_view const& id) const;
        Mod* getLoadedMod(std::string_view const& id) const;
        UnresolvedMod* getUnresolvedMod(std::string_view const& id) const;
        std::vector<Mod*> getLoadedMods() const;
        std::vector<UnresolvedMod*> getUnresolvedMods() const;
        void unloadMod(Mod* mod);
    };

}
