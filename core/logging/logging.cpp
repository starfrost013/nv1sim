// 
// VOLT
// Copyright © 2023-2025 starfrost
//
// Logging.cpp: Handles logging for the Volt project
//

#include "logging.hpp"

namespace NV1Sim
{
	
	// Defines 
	#define	LOGGING_MAX_LENGTH_TEXT		1024												// Maximum length of text being logged via the logger.
	#define LOGGING_MAX_LENGTH_DATE		32													// Length of the string containing the current date.
	#define LOGGING_MAX_LENGTH_TOTAL	LOGGING_MAX_LENGTH_TEXT + LOGGING_MAX_LENGTH_DATE	// Total length of the logging text buffer.

	#define ITS_OVER					0x43524150											// 'CRAP' Exit code used on failure

	// Prototypes for "internal" functions only used in this source file
	void Logging_Log(const char* text, LogChannel channel, va_list args);

	// various buffer size demands
	#define CONSOLE_ESC_CHAR				"\x1B"	//Some commands don't use it
	#define CONSOLE_TERMINAL_COMMAND_PREFIX "\x1B["
	#define CONSOLE_COLOR_BUFFER_SIZE		24		// maded a bit bigger for safety

	void Util_ConsoleSetForegroundColor(ConsoleColor color)
	{
		int32_t color_final = 30 + color;

		if (color >= ConsoleColor::FIRST_BRIGHT) 
			color_final = 90 + (color & ConsoleColor::FIRST_BRIGHT - 1);

		// 10 (max) + 1 + 4 + 1 for safety
		char final_string[CONSOLE_COLOR_BUFFER_SIZE] = { 0 };

		snprintf(final_string, CONSOLE_COLOR_BUFFER_SIZE, "%s%dm", CONSOLE_TERMINAL_COMMAND_PREFIX, color_final);

		printf("%s", final_string);
	}

	void Util_ConsoleSetBackgroundColor(ConsoleColor color)
	{
		int32_t color_final = 40 + color;

		if (color >= ConsoleColor::FIRST_BRIGHT) 
			color_final = 100 + (color & ConsoleColor::FIRST_BRIGHT - 1);

		// 10 (max) + 1 + 4 + 1 for safety
		char final_string[CONSOLE_COLOR_BUFFER_SIZE] = { 0 };

		snprintf(final_string, CONSOLE_COLOR_BUFFER_SIZE, "%s%dm", CONSOLE_TERMINAL_COMMAND_PREFIX, color_final);
		
		printf("%s", final_string);
	}

	void Util_ConsoleResetForegroundColor()
	{
		printf(CONSOLE_TERMINAL_COMMAND_PREFIX "39m");
	}

	void Util_ConsoleResetBackgroundColor()
	{
		printf(CONSOLE_TERMINAL_COMMAND_PREFIX "49m");
	}

	// not threadsafe :O
	Logger logger = { 0 };

	// Implementation
	bool Logging_Init()
	{
		//todo: allow this to be configured in settings
		if (!logger.settings.changed)
		{
			logger.settings.channels = (LogChannel)(LogChannel::Debug | LogChannel::Message | LogChannel::Warning | LogChannel::Error | LogChannel::Fatal | LogChannel::SuperFatal);
			logger.settings.source = (LogDestination)(LogDestination::Printf | LogDestination::File);
			logger.settings.keep_old_logs = false; 
		}

		if (!logger.settings.file_name) 
			logger.settings.file_name = "latest.log";
			

		if (logger.settings.source & LogDestination::File)
		{
			logger.handle = fopen(logger.settings.file_name, "w+");

			if (!logger.handle)
			{
				printf("Log failed: 0x0002DEAD Error opening logfile %s: errno %d\n", logger.settings.file_name, errno);
				return false;
			}
		}

		logger.initialised = true;

		Logging_LogChannel("%s initialised", LogChannel::Debug, STARFROSTLOG_VERSION);

		return true;
	}

	void Logging_Shutdown()
	{
		fclose(logger.handle);
	}

	void Logging_LogChannel(const char* text, LogChannel channel, ...)
	{
		va_list	args;

		va_start(args, channel);

		Logging_Log(text, channel, args);
	}

	// Logs to all channels except the Fatal Error log channel.
	void Logging_LogAll(const char* text, ...)
	{
		va_list args;

		va_start(args, text);

		Logging_Log(text, (LogChannel)(LogChannel::Debug | LogChannel::Message | LogChannel::Warning | LogChannel::Error), args);
	}

	// yes this is required
	void Logging_Log(const char* text, LogChannel channel, va_list args)
	{
		// don't print messages on closed log channels
		if (!(logger.settings.channels & channel))
			return;

	// if solely printing debug channel, return on release build
	#ifdef RELEASE
		if (channel == LogChannel::Debug)
			return;
	#endif

		if (strlen(text) > LOGGING_MAX_LENGTH_TEXT)
		{
			printf("Log failed: cannot log string of length 0 or above %d bytes!", LOGGING_MAX_LENGTH_TEXT);
			return;
		}

		char date_buffer[LOGGING_MAX_LENGTH_DATE] = {0};
		char log_string_buffer[LOGGING_MAX_LENGTH_TOTAL] = {0};

		auto now = std::chrono::system_clock::now();
		std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
		snprintf(date_buffer, LOGGING_MAX_LENGTH_DATE, "%s", std::ctime(&now_time_t));

		size_t date_buffer_length = strlen(date_buffer);

		// avoids a crash by trying to decrement 1 from this value...whoops
		if (date_buffer_length <= 0)
			return;

		// lop off the last character so it doesn't have a new line

		date_buffer[date_buffer_length - 1] = '\0';

		const char* prefix = "[";
		const char* suffix_date = "]: ";
		const char* suffix = " \n";

		// print separate colours
		switch (channel)
		{
			case LogChannel::Message:	//shutup gcc
				break;
			case LogChannel::Debug:
				prefix = "[DEBUG] [";
				break;		
			case LogChannel::Warning:
				prefix = "[WARNING] [";
				break;
			case LogChannel::Error:
				prefix = "[ERROR] [";
				break;
			case LogChannel::Fatal:
				prefix = "[FATAL] [";
				break;
			case LogChannel::SuperFatal:
				prefix = "[SUPER FATAL] [";
				break;
		}

		snprintf(log_string_buffer, sizeof(log_string_buffer), 
		"%s%s%s %s %s", prefix, date_buffer, suffix_date, text, suffix);

		if (logger.settings.source & LogDestination::Printf)
		{
			switch (channel)
			{
				case LogChannel::Message: //shutup GCC
					break;
				case LogChannel::Debug:
					Util_ConsoleSetForegroundColor(ConsoleColor::BrightBlue);
					break;
				case LogChannel::Warning:
					Util_ConsoleSetForegroundColor(ConsoleColor::BrightYellow);
					break;
				case LogChannel::Error:
				case LogChannel::Fatal:
				case LogChannel::SuperFatal:
					Util_ConsoleSetForegroundColor(ConsoleColor::BrightRed);
					break;
			}

			vprintf(log_string_buffer, args);

			Util_ConsoleResetForegroundColor();

			// If a fatal error message is logged, we're going down,
			// so call the fatal error function if it is safe to do so
			if (channel & LogChannel::Fatal
			&& !(channel & LogChannel::SuperFatal)
			&& logger.settings.fatal_function)
			{
				logger.settings.fatal_function();
			}
				
		}

		if (logger.settings.source & LogDestination::File)
			vfprintf(logger.handle, log_string_buffer, args);

		if (channel & LogChannel::SuperFatal)
		{
			std::cout << "MEMORY COMPLETELY HOSED. NOT SAFE TO CLEANLY SHUT DOWN - LET'S GET THE HELL OUT OF HERE!!!" << std::endl;
			exit(ITS_OVER);
		}

		va_end(args);
	}
}
