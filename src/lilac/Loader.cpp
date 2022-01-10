#include <Hook.hpp>
#include <Mod.hpp>
#include <Log.hpp>
#include <Loader.hpp>
#include <utils.hpp>
#include <Internal.hpp>
#include <InternalMod.hpp>

USE_LILAC_NAMESPACE();

Loader* Loader::get() {
    static auto g_loader = new Loader;
    return g_loader;
}

void Loader::createDirectories() {
    file_utils::createDirectory(const_join_path_c_str<lilac_directory>);
    file_utils::createDirectory(const_join_path_c_str<lilac_directory, lilac_resource_directory>);
    file_utils::createDirectory(const_join_path_c_str<lilac_directory, lilac_mod_directory>);
}

size_t Loader::updateMods() {
    InternalMod::get()->log()
        << Severity::Debug
        << "Loading mods..."
        << lilac::endl;

    size_t loaded = 0;
    this->createDirectories();
    for (auto const& entry : std::filesystem::directory_iterator(
        std::filesystem::absolute(lilac_directory) / lilac_mod_directory
    )) {
        if (
            std::filesystem::is_regular_file(entry) &&
            entry.path().extension() == lilac_mod_extension
        ) {
            InternalMod::get()->log()
                << Severity::Debug
                << "Loading " << entry.path().string()
                << lilac::endl;
            if (!vector_utils::contains<Mod*>(
                this->m_mods,
                [entry](Mod* p) -> bool {
                    return p->m_info.m_path == entry.path().string();
                }
            )) {
                auto res = this->loadModFromFile(entry.path().string());
                if (res) {
                    loaded++;
                } else {
                    InternalMod::get()->throwError(res.error(), Severity::Error);
                }
            }
        }
    }
    return loaded;
}

bool Loader::isModLoaded(std::string_view const& id) const {
    return vector_utils::contains<Mod*>(
        this->m_mods,
        [id](Mod* p) -> bool {
            return p->m_info.m_id == id;
        }
    );
}

Mod* Loader::getLoadedMod(std::string_view const& id) const {
    return vector_utils::select<Mod*>(
        this->m_mods,
        [id](Mod* p) -> bool {
            return p->m_info.m_id == id;
        }
    );
}

UnresolvedMod* Loader::getUnresolvedMod(std::string_view const& id) const {
    return vector_utils::select<UnresolvedMod*>(
        this->m_unresolvedMods,
        [id](UnresolvedMod* p) -> bool {
            return p->m_info.m_id == id;
        }
    );
}

std::vector<Mod*> Loader::getLoadedMods() const {
    return this->m_mods;
}

std::vector<UnresolvedMod*> Loader::getUnresolvedMods() const {
    return this->m_unresolvedMods;
}

void Loader::updateAllDependencies() {
    for (auto const& mod : this->m_mods) {
        mod->m_info.updateDependencyStates();
    }
    for (auto const& mod : this->m_unresolvedMods) {
        mod->m_info.updateDependencyStates();
    }
}

void Loader::unloadMod(Mod* mod) {
    vector_utils::erase(this->m_mods, mod);
    // ~Mod will call FreeLibrary 
    // automatically
    delete mod;
}

bool Loader::setup() {
    if (this->m_isSetup)
        return true;

    InternalMod::get()->log()
        << Severity::Debug
        << "Setting up Loader..."
        << lilac::endl;

    this->createDirectories();
    this->updateMods();

    this->m_isSetup = true;

    return true;
}

Loader::Loader() {
    this->m_logStream = new LogStream;
}

Loader::~Loader() {
    for (auto const& Mod : this->m_mods) {
        delete Mod;
    }
    for (auto const& log : this->m_logs) {
        delete log;
    }
    delete this->m_logStream;
}

LogStream& Loader::logStream() {
    return *this->m_logStream;
}

void Loader::log(LogMessage* log) {
    this->m_logs.push_back(log);
}

void Loader::deleteLog(LogMessage* log) {
    vector_utils::erase(this->m_logs, log);
    delete log;
}

std::vector<LogMessage*> const& Loader::getLogs() const {
    return this->m_logs;
}

std::vector<LogMessage*> Loader::getLogs(
    std::initializer_list<Severity> severityFilter
) {
    if (!severityFilter.size()) {
        return this->m_logs;
    }

    std::vector<LogMessage*> logs;

    for (auto const& log : this->m_logs) {
        if (vector_utils::contains<Severity>(severityFilter, log->getSeverity())) {
            logs.push_back(log);
        }
    }

    return logs;
}

