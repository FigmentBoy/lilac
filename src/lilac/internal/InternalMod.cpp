#include "InternalMod.hpp"

void Mod::setupInternal() {
    this->m_info.m_id          = "com.lilac.lilac";
    this->m_info.m_name        = "Lilac";
    this->m_info.m_developer   = "Lilac Team";
    this->m_info.m_description = "Internal representation";
    this->m_info.m_details     = "Internal representation of Lilac.";
    this->m_info.m_credits     = "";
    this->m_info.m_version     = { 1, 0, 0 };
}

void InternalMod::setup() {}

InternalMod::InternalMod() {
    this->setupInternal();
}

InternalMod* InternalMod::get() {
    static auto g_mod = new InternalMod;
    return g_mod;
}
