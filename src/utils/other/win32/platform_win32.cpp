#include <utils/other/platform.hpp>

#ifdef LILAC_IS_WIN32

#include <iostream>
#include <sstream>

bool lilac::utils::copyToClipboard(std::string const& data) {
    if (!OpenClipboard(nullptr))
        return false;
    if (!EmptyClipboard()) {
        CloseClipboard();
        return false;
    }

    HGLOBAL hg = GlobalAlloc(GMEM_MOVEABLE, data.size() + 1);
    
	if (!hg) {
		CloseClipboard();
		return false;
	}

	auto dest = GlobalLock(hg);

	if (!dest) {
		CloseClipboard();
		return false;
	}

	memcpy(dest, data.c_str(), data.size() + 1);

	GlobalUnlock(hg);

	SetClipboardData(CF_TEXT, hg);
	CloseClipboard();

	GlobalFree(hg);

    return true;
}

std::string lilac::utils::readClipboard() {
    if (!OpenClipboard(nullptr))
        return "";
    
    HANDLE hData = GetClipboardData(CF_TEXT);
    if (hData == nullptr) {
        CloseClipboard();
        return "";
    }

    char * pszText = static_cast<char*>(GlobalLock(hData));
    if (pszText == nullptr) {
        CloseClipboard();
        return "";
    }

    std::string text(pszText);

    GlobalUnlock(hData);
    CloseClipboard();

    return text;
}

#endif