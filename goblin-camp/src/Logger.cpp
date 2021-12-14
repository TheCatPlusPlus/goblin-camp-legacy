/* Copyright 2010-2011 Ilkka Halila
This file is part of Goblin Camp.

Goblin Camp is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Goblin Camp is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License 
along with Goblin Camp. If not, see <http://www.gnu.org/licenses/>.*/
#include "stdafx.hpp"

#include <fstream>
#include <cassert>

#include <boost/date_time/local_time/local_time.hpp>
#include <boost/filesystem.hpp>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace fs = boost::filesystem;

#include "Logger.hpp"

namespace Logger
{
	void Emit(const std::string &message, const char *file, int line, const char *function)
	{
		spdlog::info("[{}:{}] [{}]\n{}", file, line, function, message);
	}

	void EmitPython(const std::string &message)
	{
		spdlog::info("{}", message);
	}

	void OpenLogFile(const std::string &logFile)
	{
		auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		consoleSink->set_pattern("%v\n");

		auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFile, true);
		fileSink->set_pattern("[%H:%M:%S] %v\n");

		spdlog::set_level(spdlog::level::debug);
		spdlog::flush_on(spdlog::level::debug);

		spdlog::default_logger()->sinks().clear();
		spdlog::default_logger()->sinks().push_back(consoleSink);
		spdlog::default_logger()->sinks().push_back(fileSink);

		LOG("Log opened " << boost::posix_time::second_clock::local_time());
	}
}
