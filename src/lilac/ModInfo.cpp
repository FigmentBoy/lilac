#include <Mod.hpp>
#include <Loader.hpp>

USE_LILAC_NAMESPACE();

void ModInfo::updateDependencyStates() {
	for (auto & dep : this->m_dependencies) {
		if (!dep.m_unresolved && !dep.m_loaded) {
			dep.m_unresolved = Loader::get()->getUnresolvedMod(dep.m_id);
			dep.m_loaded = Loader::get()->getLoadedMod(dep.m_id);
		}
		if (dep.m_unresolved) {
			dep.m_unresolved->m_info.updateDependencyStates();
			if (dep.m_unresolved->m_info.hasUnresolvedDependencies()) {
				dep.m_state = ModResolveState::Unresolved;
			} else {
				dep.m_state = ModResolveState::Resolved;
				Loader::get()->loadResolvedMod(dep.m_id);
			}
		} else if (dep.m_loaded) {
			if (dep.m_loaded->isEnabled()) {
				dep.m_state = ModResolveState::Loaded;
			} else {
				dep.m_state = ModResolveState::Disabled;
			}
		} else {
			dep.m_state = ModResolveState::Unresolved;
		}
	}
}

bool ModInfo::hasUnresolvedDependencies() const {
	for (auto const& dep : this->m_dependencies) {
		if (
			dep.m_state == ModResolveState::Unloaded ||
			dep.m_state == ModResolveState::Unresolved
		) {
			return false;
		}
	}
	return true;
}
