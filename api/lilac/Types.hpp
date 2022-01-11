#pragma once

#include "macros.hpp"
#include <string>
#include "ctypes.h"

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
	 * Return a nullptr from `lilac_c_load` 
	 * if loading was succesful, 
	 * and a pointer to a char array that 
	 * describes why the mod couldn't be loaded. 
	 * The char array will be freed by lilac 
	 * afterwards.
	 */
	typedef char*(__stdcall* lilac_c_load)();
	typedef void (__stdcall* lilac_c_unload)();
	typedef void (__stdcall* lilac_c_enable)();
	typedef void (__stdcall* lilac_c_disable)();
}
