#pragma once

#include "macros.hpp"
#include <string>

namespace lilac {
	/**
	 * Describes the severity of the log 
	 * message.
	 * @enum Severity
	 */
	struct Severity {
		enum {
			/**
			 * The message contains information 
			 * only relevant to the developer / 
			 * other mod developers.
			 */
			Debug,

			/**
			 * The message contains general 
			 * information about that logger's 
			 * state. The logger is still 
			 * working correctly.
			 */
			Info,

			/**
			 * The message contains information 
			 * about the logger's state that 
			 * may require special attention.
			 */
			Notice,

			/**
			 * The message contains information 
			 * about the logger's state that 
			 * is abnormal and may result in 
			 * errors if not handled properly.
			 */
			Warning,

			/**
			 * The message contains information 
			 * about a general error, such as 
			 * inability to read files.
			 */
			Error,
			
			/**
			 * The message contains information 
			 * about a severe error that may 
			 * cause the logger to be partly or 
			 * fully practically unusable.
			 */
			Critical,

			/**
			 * The message contains information 
			 * about a very severe error that 
			 * requires immediate attention,
			 * and renders the logger completely 
			 * unusable.
			 */
			Alert,
			
			/**
			 * The logger has encountered an 
			 * error so severe it is utterly 
			 * unusable and can not proceed 
			 * its execution. This error level 
			 * should never be seen in-game, 
			 * as an error this severe will 
			 * most likely cause an immediate 
			 * unrequested exit of the 
			 * application, also known as a 
			 * crash.
			 */
			Emergency,
		};
	   
		using type = decltype(Debug);

		type m_value;

		Severity(type t) { m_value = t; }
		Severity& operator=(type t) { m_value = t; return *this; }
		bool operator==(int other) const { return m_value == other; }
		bool operator==(Severity const& other) const { return m_value == other.m_value; }
		operator int() { return m_value; }

		template<class T>
		static type cast(T t) {
			return static_cast<type>(t);
		}

		static constexpr const char* toString(type lp) {
			switch (lp) {
				case Debug:     return "Debug";
				case Info:      return "Info";
				case Notice:    return "Notice";
				case Warning:   return "Warning";
				case Error:     return "Error";
				case Critical:  return "Critical";
				case Alert:     return "Alert";
				case Emergency: return "Emergency";
			}
			return "Undefined";
		}
	};

	class Mod;

	/**
	 * Struct for providing mod information for 
	 * mods written in languages other than 
	 * C++, who aren't able to use `lilac_load`. 
	 * @struct CModInfo
	 */
	struct CModInfo {
		/**
		 * Mod Version. Should follow semver. 
		 * i.e. `v1.0.0`
		 */
		const char* version;
		/**
		 * Human-readable ID of the Mod.
		 * Recommended to be in the format
		 * "com.developer.mod". Not
		 * guaranteed to be either case-
		 * nor space-sensitive. Should
		 * be restricted to the ASCII
		 * character set.
		 */
		const char* id;
		/**
		 * Name of the mod. May contain
		 * spaces & punctuation, but should
		 * be restricted to the ASCII
		 * character set.
		 */
		const char* name;
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
		const char* developer;
		/**
		 * Short description between 1 and
		 * 60 characters.
		 */
		const char* description;
		/**
		 * Free-form detailed description
		 * of the mod. Do not write credits
		 * here; use `m_credits` instead.
		 */
		const char* details;
		/**
		 * Free-form list of credits.
		 */
		const char* credits;
	};

    /**
     * Represents if a mod has been loaded & 
     * its dependencies resolved
     */
    enum class ModResolveState {
        // Mod has not been loaded at all
        Unloaded,
        // Mod has unresolved dependencies
        Unresolved,
        // Mod has all dependencies resolved, 
        // but is not loaded yet
        Resolved,
        // Mod is loaded
        Loaded,
		// Mod is loaded, however it is also 
		// disabled and therefore can't be used
		Disabled,
    };

	/**
	 * Default Lilac load method for C++ 
	 * mods: The mod creates an instance 
	 * of its own inherited Mod interface 
	 * and returns a pointer to the created 
	 * instance.
	 */
	typedef Mod* (__stdcall* lilac_load)();

	/**
	 * If you're using a civilized language 
	 * and are unable to interface with 
	 * barbaric C++ classes, you may create 
	 * bindings for another language using 
	 * these C-style functions. Make sure to 
	 * dllexport them.
	 * 
	 * `lilac_c_load` provides you a pointer 
	 * to a pre-allocated CModInfo struct, 
	 * which you fill your mod info into.
	 * Return a nullptr if load was succesful, 
	 * and a pointer to a char array that 
	 * describes why the mod couldn't be loaded. 
	 * The char array will be freed by lilac 
	 * afterwards.
	 */
	typedef char*(__stdcall* lilac_c_load)(CModInfo*);
	typedef void (__stdcall* lilac_c_unload)();
	typedef void (__stdcall* lilac_c_enable)();
	typedef void (__stdcall* lilac_c_disable)();
}
