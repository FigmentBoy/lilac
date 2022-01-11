#include <Mod.hpp>
#include <Loader.hpp>
#include <Windows.h>
#include <Internal.hpp>
#include <InternalMod.hpp>
#include <Log.hpp>

USE_LILAC_NAMESPACE();

DWORD WINAPI load_thread(LPVOID hModule) {
    // setup internals
    if (!Lilac::get()->setup()) {
        // if we've made it here, Lilac will 
        // be gettable (otherwise the call to 
        // setup would've immediately crashed)
        Lilac::get()->platformMessageBox(
            "Unable to Load Lilac!",
            "There was an unknown fatal error setting up "
            "internal tools and Lilac can not be loaded."
        );
        return 1;
    }

    InternalMod::get()->log()
        << Severity::Debug
        << "Loaded internal Lilac class"
        << lilac::endl;

    // set up loader, load mods, etc.
    if (!Loader::get()->setup()) {
        Lilac::get()->platformMessageBox(
            "Unable to Load Lilac!",
            "There was an unknown fatal error setting up "
            "the loader and Lilac can not be loaded."
        );
        delete Lilac::get();
        return 1;
    }

    InternalMod::get()->log()
        << Severity::Debug
        << "Set up loader"
        << lilac::endl;

    // debugging console
    #ifdef LILAC_PLATFORM_CONSOLE
    InternalMod::get()->log()
        << Severity::Debug
        << "Loading Console..."
        << lilac::endl;

    Lilac::get()->setupPlatformConsole();
    Lilac::get()->awaitPlatformConsole();
    Lilac::get()->closePlatformConsole();

    InternalMod::get()->log()
        << Severity::Debug
        << "Cleaning up..."
        << lilac::endl;

    delete Lilac::get();
    #endif

    return 0;
}

BOOL WINAPI DllMain(
    HINSTANCE hModule,
    DWORD fdwReason,
    LPVOID lpReserved
) {
    switch (fdwReason) {
        case DLL_PROCESS_ATTACH:
            // Prevents threads from notifying this DLL on creation or destruction.
            // Kind of redundant for a game that isn't multi-threaded but will provide
            // some slight optimizations if a mod frequently creates and deletes threads.
            DisableThreadLibraryCalls(hModule);

            // loading thread
            HANDLE _ = CreateThread(0, 0, load_thread, hModule, 0, nullptr);
            if (_) CloseHandle(_);

            break;
    }
    return TRUE;
}
