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

#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

#include "Random.hpp"
#include "Announce.hpp"
#include "Events.hpp"
#include "Game.hpp"
#include "GCamp.hpp"
#include "Camp.hpp"
#include "StockManager.hpp"
#include "data/Config.hpp"
#include "Faction.hpp"

Events::Events(Map* vmap) :
map(vmap),
	hostileSpawningMonsters(std::vector<int>()),
	timeSinceHostileSpawn(0),
        migratingAnimals(std::vector<int>()),
	peacefulAnimals(std::vector<int>())
{
	for (unsigned int i = 0; i < NPC::Presets.size(); ++i) {
		if (NPC::Presets[i].tags.find("attacksrandomly") != NPC::Presets[i].tags.end())
			hostileSpawningMonsters.push_back(i);
		if (NPC::Presets[i].tags.find("localwildlife") != NPC::Presets[i].tags.end())
			peacefulAnimals.push_back(i);
		if (NPC::Presets[i].tags.find("immigrant") != NPC::Presets[i].tags.end())
			immigrants.push_back(i);
                if (NPC::Presets[i].tags.find("migratory") != NPC::Presets[i].tags.end())
			migratingAnimals.push_back(i);
        }
}

void Events::Update(bool safe) {
	if (!safe) {
		++timeSinceHostileSpawn;
		if (Random::Generate(UPDATES_PER_SECOND * 60 * 15 - 1) == 0 || timeSinceHostileSpawn > (UPDATES_PER_SECOND * 60 * 25)) {
			SpawnHostileMonsters();
		}
	}

	if (Random::Generate(UPDATES_PER_SECOND * 60 * 2 - 1) == 0) {
		SpawnBenignFauna();
	}

	//Remove immigrants that have left/died
	for (std::vector<boost::weak_ptr<NPC> >::iterator immi = existingImmigrants.begin(); immi != existingImmigrants.end();) {
		if (!immi->lock()) immi = existingImmigrants.erase(immi);
		else ++immi;
	}

	if (existingImmigrants.size() < Game::Inst()->OrcCount() / 7 && 
		Random::Generate(UPDATES_PER_SECOND * 60 * 30) == 0) {
			SpawnImmigrants();
	}
	
	if (Random::Generate(UPDATES_PER_SECOND * 60 * 60 * 12) == 0) {
		SpawnMigratingAnimals();
	}
}

namespace {
	void GenerateEdgeCoordinates(Map* map, Coordinate& a, Coordinate& b) {
		int counter = 0;
		do {
			switch (Random::Generate(3)) {
			case 0:
				a.X(0);
				a.Y(Random::Generate(map->Height() - 21));
				b.X(1);
				b.Y(a.Y() + 20);
				break;

			case 1:
				a.X(Random::Generate(map->Width() - 21));
				a.Y(0);
				b.X(a.X() + 20);
				b.Y(1);
				break;

			case 2:
				a.X(map->Width() - 2);
				a.Y(Random::Generate(map->Height() - 21));
				b.X(map->Width() - 1);
				b.Y(a.Y() + 20);
				break;

			case 3:
				a.X(Random::Generate(map->Width() - 21));
				a.Y(map->Height() - 2);
				b.X(a.X() + 20);
				b.Y(map->Height() - 1);
				break;
			}
			++counter;
		} while ((Map::Inst()->IsUnbridgedWater(a.X(), a.Y()) || Map::Inst()->IsUnbridgedWater(b.X(), b.Y())) && counter < 100);
	}
}

void Events::SpawnHostileMonsters() {
	std::vector<NPCType> possibleMonsters;
	for (std::vector<int>::iterator hosti = hostileSpawningMonsters.begin(); hosti != hostileSpawningMonsters.end(); ++hosti) {
		if (NPC::Presets[*hosti].tier <= Camp::Inst()->GetTier() - 2) {
			possibleMonsters.push_back((NPCType)*hosti);
		} else if (NPC::Presets[*hosti].tier <= Camp::Inst()->GetTier()) {
			possibleMonsters.push_back((NPCType)*hosti); // This is intentional, it raises the odds that monsters at or lower
			possibleMonsters.push_back((NPCType)*hosti); // than this tier are spawned vs. one tier higher.
		} else if (NPC::Presets[*hosti].tier == Camp::Inst()->GetTier() + 1) {
			possibleMonsters.push_back((NPCType)*hosti);
		}
	}

	if (!possibleMonsters.empty()) {
		NPCType monsterType = Random::ChooseElement(possibleMonsters);
		int hostileSpawnCount = Game::DiceToInt(NPC::Presets[monsterType].group);

		std::string msg;
		if (hostileSpawnCount > 1) 
			msg = (boost::format("%s have been sighted outside your %s!") 
			% NPC::Presets[monsterType].plural % Camp::Inst()->GetName()).str();
		else msg = (boost::format("A %s has been sighted outside your %s!")
			% NPC::Presets[monsterType].name % Camp::Inst()->GetName()).str();

		Coordinate a,b;
		GenerateEdgeCoordinates(map, a, b);

		Game::Inst()->CreateNPCs(hostileSpawnCount, monsterType, a, b);
		Announce::Inst()->AddMsg(msg, TCODColor::red, Coordinate((a.X() + b.X()) / 2, (a.Y() + b.Y()) / 2));
		timeSinceHostileSpawn = 0;
		if (Config::GetCVar<bool>("pauseOnDanger")) 
			Game::Inst()->AddDelay(UPDATES_PER_SECOND, boost::bind(&Game::Pause, Game::Inst()));
	}
}

void Events::SpawnBenignFauna() {
	if (peacefulAnimals.size() > 0 && Game::Inst()->PeacefulFaunaCount() < 20) {
		//Generate benign fauna
		std::vector<NPCType> possibleFauna;
		for (std::vector<int>::iterator iter = peacefulAnimals.begin(); iter != peacefulAnimals.end(); ++iter) {
			if (NPC::Presets[*iter].tier <= Camp::Inst()->GetTier())
				possibleFauna.push_back((NPCType)*iter);
		}

		for (int i = 0; i < Random::Generate(1, 10); ++i) {
			unsigned type = Random::ChooseIndex(possibleFauna);
			Coordinate target;
			do {
				target.X(Random::Generate(map->Width() - 1));
				target.Y(Random::Generate(map->Height() - 1));
			} while (!map->IsWalkable(target.X(), target.Y()) || Distance(Camp::Inst()->Center(), target) < 100
				|| (map->GetType(target.X(), target.Y()) != TILEGRASS && map->GetType(target.X(), target.Y()) != TILESNOW
				&& map->GetType(target.X(), target.Y()) != TILEMUD));
			Game::Inst()->CreateNPC(target, possibleFauna[type]);
		}
	}
}

void Events::SpawnImmigrants() {
	std::vector<NPCType> possibleImmigrants;
	for (std::vector<int>::iterator immi = immigrants.begin(); immi != immigrants.end(); ++immi) {
		if (NPC::Presets[*immi].tier <= Camp::Inst()->GetTier()) {
			possibleImmigrants.push_back((NPCType)*immi);
		}
	}

	if (!possibleImmigrants.empty()) {
		NPCType monsterType = Random::ChooseElement(possibleImmigrants);
		int spawnCount = Game::DiceToInt(NPC::Presets[monsterType].group);

		std::string msg;
		if (spawnCount > 1) 
			msg = (boost::format("%s join your %s!") 
			% NPC::Presets[monsterType].plural % Camp::Inst()->GetName()).str();
		else msg = (boost::format("A %s joins your %s!")
			% NPC::Presets[monsterType].name % Camp::Inst()->GetName()).str();

		Coordinate a, b;
		GenerateEdgeCoordinates(map, a, b);

		for (int i = 0; i < spawnCount; ++i) {
			int npcUid = Game::Inst()->CreateNPC(Coordinate(Random::Generate(a.X(), b.X()), Random::Generate(a.Y(), b.Y())),
				monsterType);
			existingImmigrants.push_back(Game::Inst()->GetNPC(npcUid));
		}

		Announce::Inst()->AddMsg(msg, TCODColor(0,150,255), Coordinate((a.X() + b.X()) / 2, (a.Y() + b.Y()) / 2));
	}
}

void Events::SpawnMigratingAnimals() {
	if (!migratingAnimals.empty()) {
		NPCType monsterType = Random::ChooseElement(migratingAnimals);
		int migrationSpawnCount = Game::DiceToInt(NPC::Presets[monsterType].group);

		std::string msg;
		msg = (boost::format("A %s migration is occurring outside your camp.") % NPC::Presets[monsterType].name).str();
		
		Coordinate a,b;
		GenerateEdgeCoordinates(map, a, b);

		std::vector<int> migrants = Game::Inst()->CreateNPCs(migrationSpawnCount, monsterType, a, b);
		for(std::vector<int>::iterator mgrnt = migrants.begin(); mgrnt != migrants.end(); mgrnt++) {
			boost::shared_ptr<NPC> npc = Game::Inst()->GetNPC(*mgrnt);
			npc->SetFaction(Faction::StringToFactionType("Migrator"));
		}
		Announce::Inst()->AddMsg(msg, TCODColor::green, Coordinate((a.X() + b.X()) / 2, (a.Y() + b.Y()) / 2));
	}
}