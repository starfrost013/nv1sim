#pragma once
#include <chrono>
#include <cstdarg>
#include <cstdbool>
#include <cstdint>
#include <cstring>
#include <iostream>

//
// Logging system (Version 4.3) - everything before 4.3 is retroactive
// 
// Oct 2023 (1.0): 				Wrote it
// Jan 2024 (2.0): 				Fixed it...and added log_channel::Debug.
// Aug 2024 (3.0): 				Fixed it even more, and also made it C++.
// Nov 2024 (3.1): 				Fixed bright colours, simplified interface to use a singleton, other minor tweaks.
// Feb 2025 (3.2): 				Correctly namespaced it, use C++ enum names.
// June 8, 2025	(4.0):			Use C++-style declarations, remove Logger_SetOptions and add fatal_function, a function pointer to call if a fatal log message occurs.
// 								Move from 5 strncat calls to one snprintf call
// June 28, 2025 (4.1):			Add superfatals for when memory corruption makes it not safe to continue
// October 22, 2025 (4.2):		Completely use STD for everything and do not depend on any other functions, use C++ includes
// October 25, 2025 (4.3):		NV1Sim: Made a self-contained version not dependent on any other part of the app with the rewritten console stuff from Volt
//								Removed some extraneous memset (they were already initialised to zero)
//								Allow the user to specify LogSettings::changed boolean to not override log settings during init

// This should be 
#define STARFROSTLOG_VERSION 	"Starfrost Shared Logging System 4.3 (October 25, 2025)"

namespace NV1Sim
{
	// Enumerate console colours
	enum ConsoleColor
	{
		Black = 0,
		Red = 1,
		Green = 2,
		Yellow = 3,
		Blue = 4,
		Magenta = 5,
		Cyan = 6,
		White = 7,
		// this is just used to indicate the first "bright" console colour so other code is easier to read
		FIRST_BRIGHT = 8,
		BrightBlack = 8,
		BrightRed = 9,
		BrightGreen = 10,
		BrightYellow = 11,
		BrightBlue = 12,
		BrightMagenta = 13,
		BrightCyan = 14,
		BrightWhite = 15,
	};

	// Enumerates logging channels. This is a set of flags, so the user can enable anything they want.
	enum LogChannel
	{
		// Prints only on debug builds.
		Debug = 1,

		// Prints advisory messages.
		Message = 2,

		// Prints warnings. The user needs to know about them, but they don't impair program operation.
		Warning = 4,

		// Prints errors that impair program operation.
		Error = 8,

		// Prints fatal errors that require the program to exit.
		Fatal = 16,

		// Memory corruption detected. Even calling Common_Shutdown isn't safe. 
		SuperFatal = 32,

	};

	// Enumerates available log output sources.
	enum LogDestination
	{
		// Logs to standardout using vprintf.
		Printf = 1,

		// Logs to a file. (currently "latest.log")
		File = 2,
	};

	// Struct storing log settings.
	struct LogSettings
	{
		const char* file_name;
		LogChannel channels;
		LogDestination source;
		bool keep_old_logs;
		bool changed; 
		void (*fatal_function)();
	};

	// Struct storing the actual logger itelf.
	struct Logger
	{
		LogSettings settings;
		FILE* handle;
		bool initialised;
	};

	bool Logging_Init();
	void Logging_LogChannel(const char* text, LogChannel channel, ...);
	void Logging_LogAll(const char* text, ...);
	void Logging_Shutdown();

	// Logger global object
	extern Logger logger;
}

