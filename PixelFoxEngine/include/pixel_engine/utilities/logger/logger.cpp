// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

/*
 *  -----------------------------------------------------------------------------
 *  Project   : PixelFox (WMG Warwick - Module 1)
 *  Author    : Niffoxic (a.k.a Harsh Dubey)
 *  License   : MIT
 *  -----------------------------------------------------------------------------
 */

#include "pch.h"
#include "logger.h"

#include <chrono>
#include <cstdio>
#include <string>
#include <string_view>
#include <unordered_map>
#include <cassert>
#include <windows.h>

using namespace std::literals;
namespace pe  = pixel_engine;
namespace lec = pixel_engine::logger_config;

namespace
{
	using clock_sys		= std::chrono::system_clock;
	using clock_steady	= std::chrono::steady_clock;

	// ansi helpers
	inline std::string ansi_rgb(const lec::rgb c, const pe::LOGGER_CREATE_DESC& cfg)
	{
		if (!cfg.EnableAnsiTrueColor)
		{
			return {};
		}
		return std::format("\x1b[38;2;{};{};{}m", c.r, c.g, c.b);
	}

	_CONSTEXPR20 std::string_view ANSI_RESET		= "\x1b[0m";
	_CONSTEXPR20 std::string_view ANSI_CLEAR_LINE	= "\x1b[2K";
	_CONSTEXPR20 std::string_view CR				= "\r";
	_CONSTEXPR20 std::string_view CRLF				= "\r\n";

	inline std::string_view level_name(lec::LogLevel lv)
	{
		switch (lv)
		{
		case lec::LogLevel::Trace: return "TRACE";
		case lec::LogLevel::Debug: return "DEBUG";
		case lec::LogLevel::Info:  return "INFO";
		case lec::LogLevel::Warn:  return "WARN";
		case lec::LogLevel::Error: return "ERROR";
		case lec::LogLevel::Fatal: return "FATAL";
		default: return "LOG";
		}
	}

	inline size_t category_index(lec::LogCategory c)
	{
		return static_cast<size_t>(c);
	}

	inline std::string_view category_name(lec::LogCategory c)
	{
		switch (c)
		{
		case lec::LogCategory::General:   return "GEN";
		case lec::LogCategory::System:    return "SYS";
		case lec::LogCategory::Render:    return "RENDER";
		case lec::LogCategory::Physics:   return "PHYS";
		case lec::LogCategory::Audio:     return "AUDIO";
		case lec::LogCategory::AI:        return "AI";
		case lec::LogCategory::Network:   return "NET";
		case lec::LogCategory::IO:        return "IO";
		case lec::LogCategory::Asset:     return "ASSET";
		case lec::LogCategory::Scripting: return "SCRIPT";
		case lec::LogCategory::Editor:    return "EDIT";
		case lec::LogCategory::Gameplay:  return "GAME";
		default: return "CAT";
		}
	}

	struct ProgressState
	{
		std::string   title;
		std::uint64_t total   = 0;
		std::uint64_t current = 0;
	};

	pe::LOGGER_CREATE_DESC s_cfg{};
	HANDLE s_handleTerminal = nullptr;

	std::unordered_map<std::uint32_t, ProgressState> s_progress;

	inline std::string thread_badge()
	{
		return "PixelEngineThread";
	}
}

_Use_decl_annotations_
void pe::logger::init(const LOGGER_CREATE_DESC& desc)
{
	s_cfg = desc;
	enable_terminal();
}

void pe::logger::close()
{
	if (s_handleTerminal)
	{
		CloseHandle(s_handleTerminal);
		s_handleTerminal = nullptr;
	}
}

void pe::logger::enable_terminal()
{
#if defined(DEBUG) || defined(_DEBUG)
	if (!s_cfg.EnableTerminal)
	{
		return;
	}

	// attach/alloc console
	if (!AttachConsole(ATTACH_PARENT_PROCESS))
	{
		AllocConsole();
	}
	s_handleTerminal = GetStdHandle(STD_OUTPUT_HANDLE);

	// enable ANSI VT where available
	DWORD outMode = 0;
	if (s_handleTerminal && s_handleTerminal != INVALID_HANDLE_VALUE 
		&& GetConsoleMode(s_handleTerminal, &outMode))
	{
		outMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
		outMode |= DISABLE_NEWLINE_AUTO_RETURN;
		SetConsoleMode(s_handleTerminal, outMode);
	}

	if (!s_cfg.TerminalName.empty())
	{
		SetConsoleTitleA(s_cfg.TerminalName.c_str());
	}
#endif
}

_Use_decl_annotations_
void pe::logger::set_level(lec::LogLevel level) noexcept
{
	s_cfg.MinimumLevel = level;
}

_Use_decl_annotations_
void pe::logger::set_theme(const lec::LoggerTheme& theme) noexcept
{
	s_cfg.Theme = theme;
}

_Use_decl_annotations_
void pe::logger::set_time_format(std::string_view fmt)
{
	s_cfg.TimeFormat = std::string(fmt);
}

_Use_decl_annotations_
void pe::logger::set_show_timestamps(bool v) noexcept
{
	s_cfg.ShowTimestamps = v;
}

_Use_decl_annotations_
void pe::logger::set_show_thread_id(bool v) noexcept
{
	s_cfg.ShowThreadId = v;
}

_Use_decl_annotations_
void pe::logger::set_show_file_line(bool v) noexcept
{
	s_cfg.ShowFileAndLine = v;
}

_Use_decl_annotations_
void pe::logger::set_show_function(bool v) noexcept
{
	s_cfg.ShowFunction = v;
}

_Use_decl_annotations_
void pe::logger::set_use_utc(bool v) noexcept
{
	s_cfg.UseUtcTimestamps = v;
}

_Use_decl_annotations_
void pe::logger::set_use_relative_timestamps(bool v) noexcept
{
	s_cfg.UseRelativeTimestamps = v;
}

_Use_decl_annotations_
void pe::logger::set_indent_spaces(std::uint16_t n) noexcept
{
	s_cfg.IndentSpacesPerScope = n;
}

_Use_decl_annotations_
void pe::logger::set_frame_index(std::uint64_t frame) noexcept
{
	frame_index_storage() = frame;
}

_Use_decl_annotations_
void pe::logger::push_scope(std::string_view)
{
	auto& d = tls_depth();
	d = static_cast<std::uint16_t>(d + 1);
}

void pe::logger::pop_scope()
{
	auto& d = tls_depth();
	if (d > 0)
	{
		d = static_cast<std::uint16_t>(d - 1);
	}
}

_Use_decl_annotations_
void pe::logger::progress_begin(std::uint32_t id, std::string_view title, std::uint64_t total)
{
	s_progress[id] = { std::string(title), total, 0 };
	progress_update(id, 0, title);
}

_Use_decl_annotations_
void pe::logger::progress_update(std::uint32_t id, std::uint64_t current, std::string_view note)
{
	auto it = s_progress.find(id);
	if (it == s_progress.end())
	{
		return;
	}

	it->second.current = current;

	const auto total = (std::max<std::uint64_t>)(1, it->second.total);
	const double pct = static_cast<double>(it->second.current) * 100.0 / static_cast<double>(total);

	const int  barWidth = 50;
	const int  filled = static_cast<int>((pct / 100.0) * barWidth);
	std::string bar;
	bar.reserve(barWidth);
	bar.append(filled, '#');
	bar.append(barWidth - filled, '-');

	const auto& c = s_cfg.Theme.info;

	std::string line;
	line.reserve(64 + barWidth + note.size());
	line.append(ANSI_CLEAR_LINE);
	line.append(CR);
	line += ansi_rgb(c, s_cfg);
	line += std::format("[{}] [{}%] [{}] {}", it->second.title, std::format("{:.1f}", pct), bar, note);
	line.append(ANSI_RESET);

	// live update same line (no newline)
	write_line_ansi(line);
}

_Use_decl_annotations_
void pe::logger::progress_end(std::uint32_t id, bool ok)
{
	auto it = s_progress.find(id);
	if (it == s_progress.end())
	{
		return;
	}

	const auto& c = ok ? s_cfg.Theme.success : s_cfg.Theme.error;

	std::string line;
	line.append(ANSI_CLEAR_LINE);
	line.append(CR);
	line += ansi_rgb(c, s_cfg);
	line += std::format("[{}] {}", it->second.title, ok ? "Done"sv : "Failed"sv);
	line.append(ANSI_RESET);
	line.append(CRLF);

	write_line_ansi(line);
	s_progress.erase(it);
}

_Use_decl_annotations_
bool pe::logger::logv(lec::LogLevel level, lec::LogCategory category, std::string&& message, bool isSuccess, const std::source_location* loc)
{
#if defined(_DEBUG) || defined(DEBUG)
	if (static_cast<int>(level) < static_cast<int>(s_cfg.MinimumLevel))
	{
		return false;
	}

	std::string line;
	line.reserve(256 + message.size());

	// time
	if (s_cfg.ShowTimestamps)
	{
		if (s_cfg.UseRelativeTimestamps)
		{
			static long long last_ns = 0;
			const auto now = clock_steady::now().time_since_epoch();
			const auto now_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(now).count();
			const auto prev = std::exchange(last_ns, now_ns);
			const auto delta_ns = (prev == 0) ? 0 : (now_ns - prev);
			const double ms = static_cast<double>(delta_ns) / 1'000'000.0;

			line += ansi_rgb(s_cfg.Theme.timestamp, s_cfg);
			line += std::format("[+{:.3f} ms] ", ms);
			line.append(ANSI_RESET);
		}
		else
		{
			const auto now = clock_sys::now();
			auto t = clock_sys::to_time_t(now);
			std::tm tm{};
			if (s_cfg.UseUtcTimestamps)
			{
				gmtime_s(&tm, &t);
			}
			else
			{
				localtime_s(&tm, &t);
			}

			const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() % 1000;
			line += ansi_rgb(s_cfg.Theme.timestamp, s_cfg);
			line += std::format("[{:02}:{:02}:{:02}.{:03}] ", tm.tm_hour, tm.tm_min, tm.tm_sec, static_cast<int>(ms));
			line.append(ANSI_RESET);
		}
	}

	// frame
	{
		const auto fi = frame_index_storage();
		if (fi != 0)
		{
			line += ansi_rgb(s_cfg.Theme.frameIndex, s_cfg);
			line += std::format("[F{}] ", fi);
			line.append(ANSI_RESET);
		}
	}

	// level badge
	const lec::rgb levelClr =
		(level == lec::LogLevel::Trace) ? s_cfg.Theme.trace :
		(level == lec::LogLevel::Debug) ? s_cfg.Theme.debug :
		(level == lec::LogLevel::Info) ? (isSuccess ? s_cfg.Theme.success : s_cfg.Theme.info) :
		(level == lec::LogLevel::Warn) ? s_cfg.Theme.warn :
		(level == lec::LogLevel::Error) ? s_cfg.Theme.error :
		s_cfg.Theme.fatal;

	line += ansi_rgb(levelClr, s_cfg);
	line += std::format("[{}]", level_name(level));
	line.append(ANSI_RESET);
	line += ' ';

	// category badge
	{
		const auto idx = category_index(category);
		lec::rgb catClr = s_cfg.Theme.categoryBadge;
		if (idx < std::size(s_cfg.Theme.categoryColor))
		{
			catClr = s_cfg.Theme.categoryColor[idx];
		}

		line += ansi_rgb(catClr, s_cfg);
		line += std::format("[{}]", category_name(category));
		line.append(ANSI_RESET);
		line += ' ';
	}

	// thread badge
	if (s_cfg.ShowThreadId)
	{
		const std::string label = thread_badge(); // "MAIN"
		line += ansi_rgb(s_cfg.Theme.threadId, s_cfg);
		line += std::format("[{}]", label);
		line.append(ANSI_RESET);
		line += ' ';
	}

	// file line / function
	if (loc && (s_cfg.ShowFileAndLine || s_cfg.ShowFunction))
	{
		line += ansi_rgb(s_cfg.Theme.fileLine, s_cfg);
		if (s_cfg.ShowFileAndLine)
		{
			line += std::format("({}:{})", loc->file_name(), static_cast<int>(loc->line()));
			if (s_cfg.ShowFunction)
			{
				line += ' ';
			}
		}
		if (s_cfg.ShowFunction)
		{
			line += std::format("[{}]", loc->function_name());
		}
		line.append(ANSI_RESET);
		line += ' ';
	}

	// indent
	{
		const auto depth = tls_depth();
		if (depth && s_cfg.IndentSpacesPerScope > 0)
		{
			const size_t spaces = static_cast<size_t>(depth) * static_cast<size_t>(s_cfg.IndentSpacesPerScope);
			line.append(spaces, ' ');
		}
	}

	// message
	line += ansi_rgb(levelClr, s_cfg);
	line += message;
	line.append(ANSI_RESET);
	line.append(CRLF);

	// write out
	write_line_ansi(line);

	// debugger echo
	if (s_cfg.DuplicateToDebugger)
	{
		OutputDebugStringA(line.c_str());
	}

#endif
	return true;
}

bool pe::logger::write_line_ansi(_In_ std::string_view line)
{
	// if nno console found then dump to stdout
	if (!s_handleTerminal || s_handleTerminal == INVALID_HANDLE_VALUE)
	{
		std::fwrite(line.data(), 1, line.size(), stdout);
		std::fflush(stdout);
		return true;
	}

	DWORD written = 0;
	const BOOL ok = WriteFile
	(
		s_handleTerminal,
		line.data(),
		static_cast<DWORD>(line.size()),
		&written,
		nullptr
	);
	return ok == TRUE;
}

std::uint64_t& pe::logger::frame_index_storage()
{
	static std::uint64_t v = 0;
	return v;
}

std::uint16_t& pe::logger::tls_depth()
{
	static std::uint16_t depth = 0;
	return depth;
}
