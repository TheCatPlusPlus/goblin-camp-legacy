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

#include <string>
#include <vector>
#include <list>
#include <map>

#include <boost/serialization/split_member.hpp>
#include <boost/shared_ptr.hpp>
#include <libtcod.hpp>

#include "Entity.hpp"
#include "Coordinate.hpp"
#include "Attack.hpp"

typedef int ItemCategory;
typedef int ItemType;

class ItemCat {
public:
	ItemCat();
	bool flammable;
	std::string name;
	std::string GetName();
	ItemCategory parent;
};

struct ItemPreset {
	ItemPreset();
	int graphic;
	TCODColor color;
	std::string name;
	std::set<ItemCategory> specificCategories;
	std::set<ItemCategory> categories;
	std::vector<ItemCategory> components;
	int nutrition;
	ItemType growth;
	std::list<ItemType> fruits;
	bool organic;
	int container;
	int multiplier;
	std::string fitsInRaw;
	std::string containInRaw;
	std::string constructedInRaw;
	ItemCategory fitsin;
	ItemCategory containIn;
	bool decays;
	int decaySpeed;
	std::vector<ItemType> decayList;
	Attack attack;
	int resistances[RES_COUNT];
	int bulk;
	int condition;
	std::string fallbackGraphicsSet;
	int graphicsHint;
};

class Item : public Entity {
	friend class boost::serialization::access;
	friend class Game;
	friend class ItemListener;

private:
	template<class Archive>
	void save(Archive & ar, const unsigned int version) const;
	template<class Archive>
	void load(Archive & ar, const unsigned int version);
	BOOST_SERIALIZATION_SPLIT_MEMBER()

	ItemType type;
	std::set<ItemCategory> categories;
	bool flammable;
	bool attemptedStore;
	int decayCounter;

	static boost::unordered_map<std::string, ItemType> itemTypeNames;
	static boost::unordered_map<std::string, ItemCategory> itemCategoryNames;

	int resistances[RES_COUNT];

protected:
	Attack attack;
	int condition;
	TCODColor color;
	int graphic;
	Item(Coordinate = Coordinate(0,0), ItemType = 0, int owner = 0,
		std::vector<boost::weak_ptr<Item> > = std::vector<boost::weak_ptr<Item> >());
	boost::weak_ptr<Item> container;
	bool internal;

public:
	static std::string ItemTypeToString(ItemType);
	static ItemType StringToItemType(std::string);
	static std::string ItemCategoryToString(ItemCategory);
	static ItemCategory StringToItemCategory(std::string);

	static std::vector<ItemCategory> Components(ItemType);
	static ItemCategory Components(ItemType, int);

	static void LoadPresets(std::string);
	static void ResolveContainers();

	static std::vector<ItemCat> Categories;
	static std::vector<ItemPreset> Presets;

	virtual ~Item();

	int GetGraphicsHint() const;
	virtual void Draw(Coordinate, TCODConsole*);
	void PutInContainer(boost::weak_ptr<Item> = boost::weak_ptr<Item>());
	boost::weak_ptr<Item> ContainedIn();
	virtual void Position(Coordinate);
	virtual Coordinate Position();
	ItemType Type();
	int GetGraphic();
	TCODColor Color();
	void Color(TCODColor);
	bool IsCategory(ItemCategory);
	virtual void Reserve(bool);
	virtual void SetFaction(int);
	virtual int GetFaction() const;
	Attack GetAttack() const;
	int RelativeValue();
	int Resistance(int) const;
	virtual void SetVelocity(int);
	void UpdateVelocity();
	void SetInternal();
	int GetDecay() const;
	void Impact(int speedChange);
};

class OrganicItem : public Item {
	friend class boost::serialization::access;
	friend class game;

private:
	template<class Archive>
	void save(Archive & ar, const unsigned int version) const;
	template<class Archive>
	void load(Archive & ar, const unsigned int version);
	BOOST_SERIALIZATION_SPLIT_MEMBER()

	int nutrition;
	ItemType growth;

public:
	OrganicItem(Coordinate=Coordinate(0,0), ItemType=0);
	int Nutrition();
	void Nutrition(int);
	ItemType Growth();
	void Growth(ItemType);
};
