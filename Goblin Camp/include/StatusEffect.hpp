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

#include <libtcod.hpp>
#include <string>

enum NPCStat {
	ATTACKSKILL,
	ATTACKPOWER,
	ATTACKSPEED,
	DEFENCESKILL,
	STAT_COUNT
};

enum StatusEffectType {
	HUNGER = 0,
	THIRST,
	PANIC,
	CONCUSSION
};

struct StatusEffect {
	StatusEffect(StatusEffectType);
	int graphic;
	TCODColor color;
	std::string name;
	int type;
	int cooldown;
	int cooldownDefault;
	double statChanges[STAT_COUNT]; //These are percentage values of the original value (100% = no change)
};