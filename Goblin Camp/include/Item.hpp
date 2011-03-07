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
#include <boost/unordered_map.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/shared_ptr.hpp>
#include <libtcod.hpp>

#include "Entity.hpp"
#include "Attack.hpp"

typedef int ItemCategory;
typedef int ItemType;

class Coordinate;

class ItemCat {
public:
	ItemCat();
	bool flammable;
	std::string name;
	std::string GetName();
	ItemCategory parent;
	inline bool operator==(const ItemCat& other) const {
		return (flammable == other.flammable &&
			name == other.name &&
			parent == other.parent);
	}
	inline bool operator!=(const ItemCat& other) const {
		return !(operator==(other));
	}
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
	std::vector<std::pair<StatusEffectType, int> > addsEffects;
	std::vector<std::pair<StatusEffectType, int> > removesEffects;
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
	Item(Coordinate = Coordinate(0,0), ItemType = -1, int owner = -1,
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
	static void UpdateEffectItems();

	static std::vector<ItemCat> Categories;
	static std::vector<ItemCat> ParentCategories;
	static std::vector<ItemPreset> Presets;
	static std::multimap<StatusEffectType, ItemType> EffectRemovers;
	static std::multimap<StatusEffectType, ItemType> GoodEffectAdders;

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
	bool IsFlammable();
	int DecreaseCondition(); //Only decreases condition, does NOT handle item removal or debris creation!
};

#include <boost/serialization/version.hpp>
BOOST_CLASS_VERSION(Item, 0)

template<class Archive>
void Item::save(Archive & ar, const unsigned int version) const {
	ar & boost::serialization::base_object<Entity>(*this);
	ar & graphic;
	std::string itemType(Item::ItemTypeToString(type));
	ar & itemType;
	ar & color.r;
	ar & color.g;
	ar & color.b;
	int categoryCount = (int)categories.size();
	ar & categoryCount;
	for (std::set<ItemCategory>::iterator cati = categories.begin(); cati != categories.end(); ++cati) {
		std::string itemCat(Item::ItemCategoryToString(*cati));
		ar & itemCat;
	}
	ar & flammable;
	ar & attemptedStore;
	ar & decayCounter;
	ar & attack;
	ar & resistances;
	ar & condition;
	ar & container;
	ar & internal;
}

template<class Archive>
void Item::load(Archive & ar, const unsigned int version) {
	ar & boost::serialization::base_object<Entity>(*this);
	ar & graphic;
	bool failedToFindType = false;
	std::string typeName;
	ar & typeName;
	type = Item::StringToItemType(typeName);
	if (type == -1) {
		type = Item::StringToItemType("debris");
		failedToFindType = true;
	}
	ar & color.r;
	ar & color.g;
	ar & color.b;
	int categoryCount = 0;
	ar & categoryCount;
	categories.clear();
	for (int i = 0; i < categoryCount; ++i) {
		std::string categoryName;
		ar & categoryName;
		int categoryType = Item::StringToItemCategory(categoryName);
		if (categoryType >= 0 && categoryType < Item::Categories.size()) categories.insert(categoryType);
	}
	if (categories.empty()) categories.insert(Item::StringToItemCategory("garbage"));
	ar & flammable;
	if (failedToFindType) flammable = true; //Just so you can get rid of it
	ar & attemptedStore;
	ar & decayCounter;
	ar & attack;
	ar & resistances;
	ar & condition;
	ar & container;
	ar & internal;
}


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

#include <boost/serialization/version.hpp>
BOOST_CLASS_VERSION(OrganicItem, 0)

template<class Archive>
void OrganicItem::save(Archive & ar, const unsigned int version) const {
	ar & boost::serialization::base_object<Item>(*this);
	ar & nutrition;
	ar & growth;
}
template<class Archive>
void OrganicItem::load(Archive & ar, const unsigned int version) {
	ar & boost::serialization::base_object<Item>(*this);
	ar & nutrition;
	ar & growth;
}
