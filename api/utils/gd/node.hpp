#pragma once

#include "include_gd.hpp"

namespace lilac::node {
    LILAC_DLL bool nodeIsVisible(cocos2d::CCNode* node);
    LILAC_DLL bool nodeIsHovered(cocos2d::CCNode* node, cocos2d::CCPoint const& gpos);
    LILAC_DLL cocos2d::CCPoint getMousePos();

    template<typename T>
    static T getChild(cocos2d::CCNode* x, int i) {
        if (i < 0)
            i = x->getChildrenCount() + i;
        if (static_cast<int>(x->getChildrenCount()) <= i)
            return nullptr;
        return reinterpret_cast<T>(x->getChildren()->objectAtIndex(i));
    }
}
