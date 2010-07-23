/* Copyright 2010 Ilkka Halila
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

// Routines to find, load and save GC's data files.
// Uses user personal directories to store saved games, config, local mods, etc.
//
// On Linux:
//   - personal dir = ~/.goblincamp
//   - saves        = ~/.goblincamp/saves
//   - screenshots  = ~/.goblincamp/screenshots
//   - config       = ~/.goblincamp/config.ini
//   - font         = ~/.goblincamp/terminal.png
//   - local mods   = ~/.goblincamp/mods
//   - global data  = <bin-dir>/../share/goblin-camp, or <bin-dir>/
//   - executable   = /usr/bin/goblin-camp (or /usr/local/bin/goblin-camp, or dir structured like build/dist/variant)
// On Windows:
//   - personal dir = <My Documents>\My Games\Goblin Camp
//   - saves        = <My Documents>\My Games\Goblin Camp\saves
//   - screenshots  = <My Documents>\My Games\Goblin Camp\screenshots
//   - config       = <My Documents>\My Games\Goblin Camp\config.ini
//   - font         = <My Documents>\My Games\Goblin Camp\terminal.png
//   - local mods   = <My Documents>\My Games\Goblin Camp\mods
//   - global data  = <InstallDir> (default is C:\Program Files\Goblin Camp)
//   - executable   = <InstallDir>\goblin-camp.exe
// On OSX (I already hate their documentation; if it's wrong, please tell me):
//   - personal dir = ~/Library/Application Support/Goblin Camp
//   - saves        = ~/Library/Application Support/Goblin Camp/saves
//   - screenshots  = ~/Library/Application Support/Goblin Camp/screenshots
//   - config       = ~/Library/Application Support/Goblin Camp/config.ini
//   - font         = ~/Library/Application Support/Goblin Camp/terminal.png
//   - local mods   = ~/Library/Application Support/Goblin Camp/mods
//   - global data  = /Applications/Goblin Camp.app/Contents/Resources
//   - executable   = /Applications/Goblin Camp.app/Contents/MacOS/goblin-camp
// This should allow GC to be installed without any administrative privileges, or
// installed once system-wide and used comfortably without admin privileges on both systems.

namespace Data {
	// Checks if required user directories are present, and creates them if not.
	// Also loads user config file, creating it if necessary.
	void Init();
	// Loads global data and all local mods.
	void Load();
	// Populates the vector with the names of saved games.
	void GetSavedGames(TCODList<std::string>&);
	// Loads saved game.
	void LoadGame(const std::string&);
	// Saves game.
	void SaveGame(const std::string&);
	// Saves screenshot. Handles autonumbering.
	void SaveScreenshot();
}