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
    static constexpr const std::string_view lilac_mod_extension      = LILAC_WINDOWS(".dll")
                                                                       LILAC_MACOS(".dylib")
                                                                       LILAC_ANDROID(".so");

    class Mod;
    class Hook;
    class LogStream;
    class LogMessage;
    struct UnresolvedMod;

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
            
            Result<Mod*> loadModFromFile(std::string const& file);
            Result<bool> checkMetaInformation(std::string const& file);
            void createDirectories();

            friend class Mod;
            friend class CustomLoader;
            friend class Lilac;
            
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
    
            bool isModLoaded(std::string_view const& id);
            Mod* getLoadedMod(std::string_view const& id);
            std::vector<Mod*> getLoadedMods();
            void unloadMod(Mod* mod);
            bool isCustomLoaderLoaded(std::string_view const& id);
    };

}
