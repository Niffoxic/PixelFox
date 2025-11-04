// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

/*
 *  -----------------------------------------------------------------------------
 *  Project   : PixelFox (WMG Warwick - Module 1)
 *  Author    : Niffoxic (a.k.a Harsh Dubey)
 *  License   : MIT
 *  -----------------------------------------------------------------------------
 */

#pragma once
#include "PixelFoxEngineAPI.h"

#include <cstdint>
#include <format>
#include <source_location>
#include <string>
#include <string_view>
#include <sal.h>

namespace pixel_engine
{
	namespace logger_config
	{
		//~ simple rgb color
		struct rgb
		{
			std::uint8_t r = 255;
			std::uint8_t g = 255;
			std::uint8_t b = 255;
		};

		//~ how loud the log is
		enum class LogLevel : std::uint8_t
		{
			Trace = 0,
			Debug,
			Info,
			Warn,
			Error,
			Fatal
		};

		//~ where the log belongs
		enum class LogCategory : std::uint8_t
		{
			General = 0,
			System,
			Render,
			Physics,
			Audio,
			AI,
			Network,
			IO,
			Asset,
			Scripting,
			Editor,
			Gameplay
		};

		//~ helper: number of categories
		inline constexpr std::size_t kCategoryCount =
			static_cast<std::size_t>(LogCategory::Gameplay) + 1;

		//~ terminal theme
		struct LoggerTheme
		{
			rgb background	 { 255,   100,   100 }; // TODO: Fix it not working
			rgb trace		 { 128, 128, 128 };
			rgb debug		 { 100, 170, 255 };
			rgb info		 { 180, 180, 180 };
			rgb success		 { 90,  200, 90 };
			rgb warn		 { 255, 220, 50 };
			rgb error		 { 255, 100, 100 };
			rgb fatal		 { 255, 64,  64 };
			rgb timestamp	 { 120, 120, 120 };
			rgb levelBadge	 { 140, 140, 140 };
			rgb categoryBadge{ 160, 160, 220 };
			rgb threadId	 { 120, 140, 180 };
			rgb fileLine	 { 110, 110, 110 };
			rgb frameIndex	 { 110, 160, 110 };
			rgb scopeGlyph	 { 90,  90,  90 };

			//~ per category beauty
			rgb categoryColor[kCategoryCount] =
			{
				{160,160,220}, // General
				{160,200,200}, // System
				{140,180,255}, // Render
				{255,160, 90}, // Physics
				{200,160,255}, // Audio
				{255,140,200}, // AI
				{160,220,160}, // Network
				{200,200,140}, // IO
				{220,180,120}, // Asset
				{180,140,255}, // Scripting
				{255,180,140}, // Editor
				{160,220,180}  // Gameplay
			};
		};
	} // namespace logger_config

	typedef struct _LOGGER_CREATE_DESC
	{
		std::string TerminalName		  = "PixelFox Logger";
		bool        EnableTerminal		  = true;
		bool        EnableAnsiTrueColor   = true;
		bool        DuplicateToDebugger   = true;
		bool        ShowTimestamps		  = true;
		bool        ShowThreadId		  = true;
		bool        ShowFileAndLine		  = true;
		bool        ShowFunction		  = true;
		bool        UseUtcTimestamps	  = false;
		bool        UseRelativeTimestamps = false;

		std::string   TimeFormat			= "%H:%M:%S.%e";
		std::uint16_t IndentSpacesPerScope	= 2;
		std::uint16_t MaxPrefixWidth		= 64;

		logger_config::LogLevel  MinimumLevel = logger_config::LogLevel::Trace;
		logger_config::LoggerTheme Theme{};
	} LOGGER_CREATE_DESC;

	/// <summary>
	/// A Static Logger
	/// </summary>
	class PFE_API logger final
	{
	public:
		logger() = delete;

		//~ Life Cycle
		static void init (_In_ const LOGGER_CREATE_DESC& desc);
		static void close();

		//~ Theme for the terminal
		static void set_level	   (_In_ logger_config::LogLevel level)			  noexcept;
		static void set_theme	   (_In_ const logger_config::LoggerTheme& theme) noexcept;
		static void set_time_format(_In_ std::string_view fmt);

		static void set_show_timestamps(_In_ bool v) noexcept;
		static void set_show_thread_id (_In_ bool v) noexcept;
		static void set_show_file_line (_In_ bool v) noexcept;
		static void set_show_function  (_In_ bool v) noexcept;
		static void set_use_utc		   (_In_ bool v) noexcept;

		static void set_use_relative_timestamps(_In_ bool v)			  noexcept;
		static void set_indent_spaces		   (_In_ std::uint16_t n)	  noexcept;
		static void set_frame_index			   (_In_ std::uint64_t frame) noexcept;

		//~ scopes for indent pretty like (main then something inside it)
		static void push_scope(_In_ std::string_view scopeName);
		static void pop_scope ();

		//~ progress bars 
		static void progress_begin(
			_In_ std::uint32_t id,
			_In_ std::string_view title,
			_In_ std::uint64_t total);

		static void progress_update(
			_In_ std::uint32_t id,
			_In_ std::uint64_t current,
			_In_ std::string_view note = {});

		static void progress_end(_In_ std::uint32_t id, _In_ bool ok);

		//~ just prints it on the logger
		template<class... Args>
		static void trace(
			_In_ const std::format_string<Args...> fmt,
			_In_opt_ Args&&... args)
		{
			logv
			(
				logger_config::LogLevel::Trace,
				logger_config::LogCategory::General,
				std::format(fmt, std::forward<Args>(args)...)
			);
		}

		//~ =============== Log Debug ====================

		template<class... Args>
		static void debug(
			_In_ const std::format_string<Args...> fmt,
			_In_opt_ Args&&... args)
		{
			logv
			(
				logger_config::LogLevel::Debug,
				logger_config::LogCategory::General,
				std::format(fmt, std::forward<Args>(args)...)
			);
		}

		template<class... Args>
		static void debug(
			_In_ logger_config::LogCategory cat,
			_In_ const std::format_string<Args...> fmt,
			_In_opt_ Args&&... args)
		{
			logv
			(
				logger_config::LogLevel::Debug,
				cat,
				std::format(fmt, std::forward<Args>(args)...)
			);
		}

		//~ =============== Log Info ====================

		template<class... Args>
		static void info(
			_In_ const std::format_string<Args...> fmt,
			_In_opt_ Args&&... args)
		{
			logv
			(
				logger_config::LogLevel::Info,
				logger_config::LogCategory::General,
				std::format(fmt, std::forward<Args>(args)...)
			);
		}

		template<class... Args>
		static void info(
			_In_ logger_config::LogCategory cat,
			_In_ const std::format_string<Args...> fmt,
			_In_opt_ Args&&... args)
		{
			logv
			(
				logger_config::LogLevel::Info,
				cat,
				std::format(fmt, std::forward<Args>(args)...)
			);
		}

		//~ =============== Log Warning ====================

		template<class... Args>
		static void warning(
			_In_ const std::format_string<Args...> fmt,
			_In_opt_ Args&&... args)
		{
			logv
			(
				logger_config::LogLevel::Warn,
				logger_config::LogCategory::General,
				std::format(fmt, std::forward<Args>(args)...)
			);
		}

		template<class... Args>
		static void warning(
			_In_ logger_config::LogCategory cat,
			_In_ const std::format_string<Args...> fmt,
			_In_opt_ Args&&... args)
		{
			logv
			(
				logger_config::LogLevel::Warn,
				cat,
				std::format(fmt, std::forward<Args>(args)...)
			);
		}

		//~ =============== Log Success ====================

		template<class... Args>
		static void success(
			_In_ const std::format_string<Args...> fmt,
			_In_opt_ Args&&... args)
		{
			logv
			(
				logger_config::LogLevel::Info,
				logger_config::LogCategory::General,
				std::format(fmt, std::forward<Args>(args)...),
				true
			);
		}

		template<class... Args>
		static void success(
			_In_ logger_config::LogCategory cat,
			_In_ const std::format_string<Args...> fmt,
			_In_opt_ Args&&... args)
		{
			logv
			(
				logger_config::LogLevel::Info,
				cat,
				std::format(fmt, std::forward<Args>(args)...),
				true
			);
		}

		//~ =============== Log Error ====================

		template<class... Args>
		static void error(
			_In_ const std::format_string<Args...> fmt,
			_In_opt_ Args&&... args)
		{
			logv
			(
				logger_config::LogLevel::Error,
				logger_config::LogCategory::General,
				std::format(fmt, std::forward<Args>(args)...),
				true
			);
		}

		template<class... Args>
		static void error(
			_In_ logger_config::LogCategory cat,
			_In_ const std::format_string<Args...> fmt,
			_In_opt_ Args&&... args)
		{
			logv
			(
				logger_config::LogLevel::Error,
				cat,
				std::format(fmt, std::forward<Args>(args)...),
				true
			);
		}

		static std::uint16_t& tls_depth(); //~ later for imgui and level editor

	private:
		static void enable_terminal();

		//~ core writing logic
		static _Check_return_ bool logv(
			_In_ logger_config::LogLevel level,
			_In_ logger_config::LogCategory category,
			_In_ std::string&& message,
			_In_ bool isSuccess = false,
			_In_opt_ const std::source_location* loc = nullptr);

		//~ ANSI write path (unicode is disabled and must be disable for this project!)
		static bool write_line_ansi(_In_ std::string_view line);

		static std::uint64_t& frame_index_storage(); //~ global index frame
	};
}
