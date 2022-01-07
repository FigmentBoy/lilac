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

namespace lilac {
    #pragma warning(disable: 4251) // I will use unordered_map and
                                   // no amount of compiler warnings
                                   // can stop me >:)
                                   
    struct PlatformInfo;

    class Hook;
    class Loader;
    class LogStream;

    struct Dependency {
        std::string_view m_id;
        // todo: Dynamic versions (1.*.*)
        VersionInfo m_version { 1, 0, 0 };
        ModResolveState m_state = ModResolveState::Unloaded;
        bool m_required = false;
        Mod* m_loaded = nullptr;
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
        std::string_view m_path;
        std::string_view m_id;
        VersionInfo m_version { 1, 0, 0 };
        std::vector<Dependency> m_dependencies;

        bool hasUnresolvedDependencies() const;
        Result<> resolve();
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
             * Path of the loaded file on a
             * given platform
             */
            std::string_view m_path;
            /**
             * Platform-specific info
             */
            PlatformInfo* m_platformInfo;
            /**
             * Hooks owned by this mod
             */
            std::vector<Hook*> m_hooks;
            /**
             * Whether the mod is enabled or not
             */
            bool m_enabled;
            /**
             * Dependencies
             */
            std::vector<Dependency> m_dependencies;

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

        protected:
            /**
             * Mod Version. Should follow semver.
             */
            VersionInfo m_version;
            /**
             * Human-readable ID of the Mod.
             * Recommended to be in the format
             * "com.developer.mod". Not
             * guaranteed to be either case-
             * nor space-sensitive. Should
             * be restricted to the ASCII
             * character set.
             */
            std::string_view m_id;
            /**
             * Name of the mod. May contain
             * spaces & punctuation, but should
             * be restricted to the ASCII
             * character set.
             */
            std::string_view m_name;
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
            std::string_view m_developer;
            /**
             * Short description between 1 and
             * 60 characters.
             */
            std::string_view m_description = "";
            /**
             * Free-form detailed description
             * of the mod. Do not write credits
             * here; use `m_credits` instead.
             */
            std::string_view m_details = "";
            /**
             * Free-form list of credits.
             */
            std::string_view m_credits = "";

            /**
             * Mod-specific setup function.
             * Initialize `m_id`, `m_name`,
             * `m_developer` and other members
             * here. Do not call this yourself,
             * lilac will call it for you.
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
            /* @region getters */
            std::string_view getID()         const;
            std::string_view getName()       const;
            std::string_view getDeveloper()  const;
            std::string_view getDescription()const;
            std::string_view getDetails()    const;
            std::string_view getCredits()    const;
            std::string_view getPath()       const;
            VersionInfo      getVersion()    const;
            bool             isEnabled()     const;

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
