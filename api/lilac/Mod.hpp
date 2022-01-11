#pragma once

#include "../keybinds/Keybind.hpp"
#include "../keybinds/KeybindAction.hpp"
#include "macros.hpp"
#include "types.hpp"
#include <utils/Result.hpp>
#include <utils/VersionInfo.hpp>
#include <string_view>
#include <vector>
#include <unordered_map>

class Lilac;
class InternalMod;

namespace lilac {                  
    struct PlatformInfo;

    class Hook;
    class Patch;
    class Loader;
    class LogStream;
    class Mod;
    struct UnresolvedMod;

    struct Dependency {
        std::string m_id;
        // todo: Dynamic versions (1.*.*)
        VersionInfo m_version { 1, 0, 0 };
        ModResolveState m_state = ModResolveState::Unloaded;
        bool m_required = false;
        Mod* m_loaded = nullptr;
        UnresolvedMod* m_unresolved = nullptr;
    };

    struct LILAC_DLL ModInfo {
        /**
         * Path to the mod file
         */
        std::string m_path;
        /**
         * Name of the platform binary within 
         * the mod zip
         */
        std::string m_binaryName = LILAC_WINDOWS("mod.dll")
                                   LILAC_MACOS("mod.dylib")
                                   LILAC_ANDROID("mod.so");
        /**
         * Mod Version. Should follow semver.
         */
        VersionInfo m_version { 1, 0, 0 };
        /**
         * Human-readable ID of the Mod.
         * Recommended to be in the format
         * "com.developer.mod". Not
         * guaranteed to be either case-
         * nor space-sensitive. Should
         * be restricted to the ASCII
         * character set.
         */
        std::string m_id;
        /**
         * Name of the mod. May contain
         * spaces & punctuation, but should
         * be restricted to the ASCII
         * character set.
         */
        std::string m_name;
        /**
         * The name of the head developer.
         * Should be a single name, like
         * "HJfod" or "The Lilac Team".
         * If the mod has multiple
         * developers, this field should
         * be one of their name or a team
         * name, and the rest of the credits
         * should be named in `m_credits`
         * instead.
         */
        std::string m_developer;
        /**
         * Short & concise description of the 
         * mod.
         */
        std::string m_description;
        /**
         * Free-form detailed description
         * of the mod. Do not write credits
         * here; use `m_credits` instead.
         */
        std::string m_details;
        /**
         * Free-form list of credits.
         */
        std::string m_credits;
        /**
         * Dependencies
         */
        std::vector<Dependency> m_dependencies;
        bool hasUnresolvedDependencies() const;
        void updateDependencyStates();
    };

    /**
     * A mod that has been picked up 
     * by lilac, but which has unresolved 
     * dependencies and thus its code 
     * and Mod interface have not been 
     * loaded yet.
     * 
     * Once all dependencies are loaded, 
     * the Mod interface will be loaded.
     * @struct UnresolvedMod
     */
    struct LILAC_DLL UnresolvedMod {
        ModInfo m_info;
    };

    /**
     * Base for the Mod class.
     * Contains internal members that
     * are managed by lilac. Do not
     * try to call / access properties
     * on here that don't have public
     * getters / setters in the Mod
     * class.
     * @class ModBase
     */
    class LILAC_DLL ModBase {
    protected:
        /**
         * Platform-specific info
         */
        PlatformInfo* m_platformInfo;
        /**
         * Hooks owned by this mod
         */
        std::vector<Hook*> m_hooks;
        /**
         * Patches owned by this mod
         */
        std::vector<Patch*> m_patches;
        /**
         * Whether the mod is enabled or not
         */
        bool m_enabled;
        /**
         * Mod info
         */
        ModInfo m_info;
        /**
         * Pointers to mods that depend on 
         * this Mod. Makes it possible to 
         * enable / disable them automatically, 
         * when their dependency is disabled.
         */
        std::vector<Mod*> m_parentDependencies;

        /**
         * Cleanup platform-related info
         */
        void platformCleanup();

        /**
         * Check whether or not this Mod
         * depends on another mod
         */
        bool depends(std::string_view const& id) const;

        /**
         * Check whether all the required 
         * dependencies for this mod have 
         * been loaded or not
         */
        bool hasUnresolvedDependencies() const;

        friend class InternalMod;
        
        /**
         * Low-level add hook
         */
        Result<Hook*> addHookBase(
            void* addr,
            void* detour,
            Hook* hook = nullptr
        );
        Result<Hook*> addHookBase(Hook* hook);
    };

    /**
     * @class Mod
     * Represents a Mod ingame. Inherit
     * from this class to create your own
     * mod interfaces.
     * @abstract
     */
    class LILAC_DLL Mod : ModBase {
    private:
        void disableBase();
        void enableBase();

        void setupInternal();

        friend class InternalMod;

    protected:
        /**
         * Mod-specific setup function.
         * Initialize any managers, hooks, 
         * etc. here. Do not call this 
         * yourself, lilac will call it 
         * for you.
         */
        virtual void setup() = 0;
        
        /**
         * Override to provide mod-specific
         * enabling code, such as re-initializing
         * managers. Do not manually re-create
         * hooks, patches, keybinds nor
         * anything else created through the
         * Mod handle; lilac will handle those.
         * Do not call this yourself,
         * lilac will call it for you.
         */
        virtual void enable();
        
        /**
         * Override to provide mod-specific
         * disabling code, such as de-initializing
         * managers. Do not manually disable
         * hooks, patches, keybinds nor
         * anything else created through the
         * Mod handle; lilac will handle those.
         * Do not call this yourself,
         * lilac will call it for you.
         */
        virtual void disable();

        // no copying
        Mod(Mod const&)           = delete;
        Mod operator=(Mod const&) = delete;
        
        /**
         * Protected constructor/destructor
         */
        Mod();
        virtual ~Mod();

        friend class Loader;
        friend class Lilac;

    public:
        std::string getID()         const;
        std::string getName()       const;
        std::string getDeveloper()  const;
        std::string getDescription()const;
        std::string getDetails()    const;
        std::string getCredits()    const;
        std::string getPath()       const;
        VersionInfo getVersion()    const;
        bool        isEnabled()     const;

        /**
         * Log to lilac's integrated console / 
         * the platform debug console.
         * @returns Reference to log stream. Make sure 
         * to end your logging with lilac::endl.
         */
        LogStream& log();

        /**
         * Throw an error. Equivalent to 
         * ```
         * Mod::log() << Severity::severity << info << lilac::endl.
         * ```
         * @param info Error infomration
         * @param severity Error severity
         */
        void throwError(
            std::string_view const& info,
            Severity severity
        );

        /**
         * Get all hooks owned by this Mod
         * @returns Vector of hooks
         */
        std::vector<Hook*> getHooks() const;

        /**
         * Create a hook at an address. Call the original 
         * function by calling the original function – 
         * no trampoline needed
         * @param address The absolute address of 
         * the function to hook, i.e. gd_base + 0xXXXX
         * @param detour Pointer to your detour function
         * @returns Successful result containing the 
         * Hook handle, errorful result with info on 
         * error
         */
        Result<Hook*> addHook(void* address, void* detour);

        /**
         * Create a hook at an address with a detour
         * and trampoline
         * @param address The absolute address of 
         * the function to hook, i.e. gd_base + 0xXXXX
         * @param detour Pointer to your detour function
         * @param trampoline Pointer to a function pointer 
         * used to call the original
         * @returns Successful result containing the 
         * Hook handle, errorful result with info on 
         * error
         */
        Result<Hook*> addHook(void* address, void* detour, void** trampoline);

        /**
         * Enable a hook owned by this Mod
         * @returns Successful result on success, 
         * errorful result with info on error
         */
        Result<> enableHook(Hook* hook);

        /**
         * Disable a hook owned by this Mod
         * @returns Successful result on success, 
         * errorful result with info on error
         */
        Result<> disableHook(Hook* hook);

        /**
         * Remove a hook owned by this Mod
         * @returns Successful result on success, 
         * errorful result with info on error
         */
        Result<> removeHook(Hook* hook);

        /**
         * Write a patch at an address
         * @param address The address to write into
         * @param data The data to write there
         * @returns Successful result on success, 
         * errorful result with info on error
         */
        Result<Patch*> patch(void* address, byte_array data);

        /**
         * Remove a patch owned by this Mod
         * @returns Successful result on success, 
         * errorful result with info on error
         */
        Result<> unpatch(Patch* patch);

        /**
         * Check whether or not this Mod
         * depends on another mod
         */
        bool depends(std::string_view const& id) const;

        /**
         * Add a new keybind action, i.e. a 
         * function that can be bound to a keybind.
         * @param action A KeybindAction; either 
         * TriggerableAction, ModifierAction or 
         * RepeatableAction.
         * @param defaults Default keybinds for 
         * this action.
         * @param insertAfter Where to insert 
         * this action in the in-game list. 
         * `nullptr` means to insert at the end.
         * @returns True if the action was added, 
         * false if not. If the function returns 
         * false, it's probably the action's ID 
         * being invalid / colliding with another 
         * action's ID.
         */
        bool addKeybindAction(
            KeybindAction     const& action,
            KeybindList       const& defaults,
            keybind_action_id const& insertAfter = nullptr
        );
        /**
         * Remove a keybind action.
         * @param id ID of the action.
         * @returns True if the action was 
         * removed, false if not.
         */
        bool removeKeybindAction(keybind_action_id const& id);
    };
}
