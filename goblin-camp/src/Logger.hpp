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
#pragma once

// Reworked logging module.
// Less verbose in usage (uses macros, though) -- LOG(foo), LOG(foo << bar),
// with more automatic formatting (file, line, function) and no more
// explicit flushing.

#include <sstream>

namespace Logger
{
	void Emit(const std::string& message, const char* file, int line, const char* function);
	void EmitPython(const std::string& message);
	void OpenLogFile(const std::string &);
}

#define LOG_FUNC(x, func) [&](const char* function){ std::stringstream S; S << x; Logger::Emit(S.str(), __FILE__, __LINE__, function); }(func)
#define LOG(x) LOG_FUNC(x, __FUNCTION__)
