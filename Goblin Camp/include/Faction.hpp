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

#include <list>
#include <map>
#include <vector>
#include <string>

#include <boost/serialization/serialization.hpp>
#include <boost/shared_ptr.hpp>

class NPC;
class Coordinate;

#define PLAYERFACTION 0

typedef int FactionType;

class Faction {
	friend class boost::serialization::access;

private:
	template<class Archive>
	void save(Archive & ar, const unsigned int version) const;
	template<class Archive>
	void load(Archive & ar, const unsigned int version);
	BOOST_SERIALIZATION_SPLIT_MEMBER()

	std::list<boost::weak_ptr<NPC> > members;
	std::map<Coordinate, bool> trapVisible;

	std::string name;

	static std::map<std::string, int> factionNames;

public:
	Faction(std::string = "Noname faction");
	void AddMember(boost::weak_ptr<NPC>);
	void RemoveMember(boost::weak_ptr<NPC>);
	void TrapDiscovered(Coordinate);
	bool IsTrapVisible(Coordinate);
	void TrapSet(Coordinate, bool visible);

	static FactionType StringToFactionType(std::string);
	static std::string FactionTypeToString(FactionType);
	static std::vector<boost::shared_ptr<Faction> > factions;

	void Reset();
};

BOOST_CLASS_VERSION(Faction, 0)

template<class Archive>
void Faction::save(Archive & ar, const unsigned int version) const {
	ar & members;
	ar & trapVisible;
	ar & name;
}

template<class Archive>
void Faction::load(Archive & ar, const unsigned int version) {
	ar & members;
	ar & trapVisible;
	ar & name;
}