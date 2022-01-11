#include "CApiMod.hpp"

#define ASSIGN_FROM_CMI(member) \
    if (info->member) this->m_info.m_##member = info->member;

void CApiMod::setup() {}

void CApiMod::enable() {
    if (this->m_enableFunc) {
        this->m_disableFunc();
    }
}

void CApiMod::disable() {
    if (this->m_disableFunc) {
        this->m_disableFunc();
    }
}

CApiMod::~CApiMod() {
    if (this->m_unloadFunc) {
        this->m_unloadFunc();
    }
}

CApiMod::CApiMod() {
    this->setup();
}
