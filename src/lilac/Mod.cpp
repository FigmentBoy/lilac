#include <KeybindManager.hpp>
#include <Hook.hpp>
#include <Mod.hpp>
#include <Log.hpp>
#include <Loader.hpp>
#include <utils/utils.hpp>

USE_LILAC_NAMESPACE();

Mod::Mod() {}

Mod::~Mod() {
    this->platformCleanup();
    for (auto const& hook : this->m_hooks) {
        this->removeHook(hook);
    }
    for (auto const& patch : this->m_patches) {
        patch->restore();
        vector_utils::erase<Patch*>(this->m_patches, patch);
        delete patch;
    }
    for (auto const& dep : this->m_info.m_dependencies) {
        vector_utils::erase(dep.m_loaded->m_parentDependencies, this);
    }
}

void Mod::setup() {}
void Mod::enable() {}
void Mod::disable() {}

void Mod::disableBase() {
    this->m_enabled = false;
    for (auto const& hook : this->m_hooks) {
        this->disableHook(hook);
    }
    this->disable();
    Loader::get()->updateAllDependencies();
}

void Mod::enableBase() {
    this->m_enabled = true;
    // note: this will enable hooks that were 
    // disabled prior to disabling the mod. 
    // not good!!!
    for (auto const& hook : this->m_hooks) {
        this->enableHook(hook);
    }
    this->enable();
    Loader::get()->updateAllDependencies();
}

decltype(ModInfo::m_id) Mod::getID() const {
    return this->m_info.m_id;
}

decltype(ModInfo::m_name) Mod::getName() const {
    return this->m_info.m_name;
}

decltype(ModInfo::m_developer) Mod::getDeveloper() const {
    return this->m_info.m_developer;
}

decltype(ModInfo::m_credits) Mod::getCredits() const {
    return this->m_info.m_credits;
}

decltype(ModInfo::m_description) Mod::getDescription() const {
    return this->m_info.m_description;
}

decltype(ModInfo::m_details) Mod::getDetails() const {
    return this->m_info.m_details;
}

decltype(ModInfo::m_path) Mod::getPath() const {
    return this->m_info.m_path;
}

VersionInfo Mod::getVersion() const {
    return this->m_info.m_version;
}

bool Mod::isEnabled() const {
    return this->m_enabled;
}

std::vector<Hook*> Mod::getHooks() const {
    return this->m_hooks;
}

LogStream& Mod::log() {
    return Loader::get()->logStream() << this;
}

void Mod::throwError(
    std::string_view const& info,
    Severity severity
) {
    Loader::get()->log(new LogMessage(
        std::string(info),
        severity,
        this
    ));
}

bool Mod::addKeybindAction(
    KeybindAction     const& action,
    KeybindList       const& defaults,
    keybind_action_id const& insertAfter
) {
    return KeybindManager::get()->addKeybindAction(
        this, action, defaults, insertAfter
    );
}

bool Mod::removeKeybindAction(keybind_action_id const& id) {
    return KeybindManager::get()->removeKeybindAction(this, id);
}

bool Mod::depends(std::string_view const& id) const {
    return this->ModBase::depends(id);
}

bool ModBase::depends(std::string_view const& id) const {
    return vector_utils::contains<Dependency>(
        this->m_info.m_dependencies,
        [id](Dependency t) -> bool { return t.m_id == id; }
    );
}

bool ModBase::hasUnresolvedDependencies() const {
    return vector_utils::contains<Dependency>(
        this->m_info.m_dependencies,
        [](Dependency t) -> bool { return t.m_required && !t.m_loaded; }
    );
}
