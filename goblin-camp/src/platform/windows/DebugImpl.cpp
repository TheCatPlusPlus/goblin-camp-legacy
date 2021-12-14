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

#include <cstdlib>
#include <windows.h>

#include <string>

#include "Logger.hpp"

/**
	Crashes the program. Obviously.
	Uses the 0xC0FFEE exception code, raises noncontinuable exception.
*/
void GCDebugInduceCrash() {
	RaiseException(0xC0FFEE, EXCEPTION_NONCONTINUABLE, 0, NULL);
}

namespace {
	inline std::wstring ToUTF16(const std::string& s)
	{
		if (s.empty())
		{
			return L"";
		}

		std::wstring buffer{};

		auto size = MultiByteToWideChar(CP_UTF8, 0, &s[0], static_cast<int>(s.size()), nullptr, 0);
		if (size == 0)
		{
			return L"ToUTF16: failed to get the buffer size";
		}

		buffer.resize(size);

		size = MultiByteToWideChar(CP_UTF8, 0, &s[0], static_cast<int>(s.size()), &buffer[0], size);
		if (size == 0)
		{
			return L"ToUTF16: failed to transcode the string";
		}

		return buffer;
	}
}

/**
	Shows 'assertion failed' dialog with a choice to exit, break to debugger or continue.
	
	\param[in] msg      Additional information.
	\param[in] expr     Expression that failed.
	\param[in] function Function that uses assert.
	\param[in] file     File that contains the function.
	\param[in] line     Line where assert was used.
	\returns            True if 'Debug' button was pressed (or the dialog couldn't be created),
	                    false if 'Continue' button was pressed.
*/
bool GCAssert(const char* msg, const char* expr, const char* function, const char* file, int line) {
	LOG("Assertion failed: '" << expr << "' (" << function << " in " << file << " at " << line << "): " << msg);

	auto fileW = ToUTF16(file);
	auto exprW = ToUTF16(expr);

	_wassert(exprW.c_str(), fileW.c_str(), line);

	return false;
}
