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
#include "stdafx.hpp"

#include <boost/format.hpp>

#include "Random.hpp"
#include "Stockpile.hpp"
#include "Game.hpp"
#include "Map.hpp"
#include "StockManager.hpp"
#include "Camp.hpp"

Stockpile::Stockpile(ConstructionType type, int newSymbol, Coordinate target) :
	Construction(type, target),
	symbol(newSymbol),
	a(target),
	b(target)
{
	condition = maxCondition;
	reserved.insert(std::pair<Coordinate,bool>(target,false));
	Container *container = new Container(target, 0, 1000, -1);
	container->AddListener(this);
	containers.insert(std::pair<Coordinate,boost::shared_ptr<Container> >(target, boost::shared_ptr<Container>(container)));
	colors.insert(std::pair<Coordinate, TCODColor>(target, TCODColor::lerp(color, Map::Inst()->GetColor(target.X(), target.Y()), 0.75f)));
	for (int i = 0; i < Game::ItemCatCount; ++i) {
		amount.insert(std::pair<ItemCategory, int>(i,0));
		allowed.insert(std::pair<ItemCategory, bool>(i,false));
		if (Item::Categories[i].parent && boost::iequals(Item::Categories[i].parent->GetName(), "Container")) {
				limits.insert(std::pair<ItemCategory, int>(i,0));
		}
	}
	Camp::Inst()->UpdateCenter(Center(), true);
	Camp::Inst()->ConstructionBuilt(type);
}

Stockpile::~Stockpile() {
	//Loop through all the containers
	for (std::map<Coordinate, boost::shared_ptr<Container> >::iterator conti = containers.begin(); conti != containers.end(); ++conti) {
		//Loop through all the items in the containers
		for (std::set<boost::weak_ptr<Item> >::iterator itemi = conti->second->begin(); itemi != conti->second->end(); ++itemi) {
			//If the item is also a container, remove 'this' as a listener
			if (itemi->lock() && itemi->lock()->IsCategory(Item::StringToItemCategory("Container"))) {
				if (boost::dynamic_pointer_cast<Container>(itemi->lock())) {
					boost::shared_ptr<Container> container = boost::static_pointer_cast<Container>(itemi->lock());
					container->RemoveListener(this);
				}
			}
		}
	}

	for (int x = a.X(); x <= b.X(); ++x) {
		for (int y = a.Y(); y <= b.Y(); ++y) {
			if (Map::Inst()->GetConstruction(x,y) == uid) {
				Map::Inst()->Buildable(x,y,true);
				Map::Inst()->SetWalkable(x,y,true);
				Map::Inst()->SetConstruction(x,y,-1);
			}
		}
	}
}

int Stockpile::Build() {return 1;}

//TODO: Remove repeated code
boost::weak_ptr<Item> Stockpile::FindItemByCategory(ItemCategory cat, int flags, int value) {

	//These two are used only for MOSTDECAYED
	int decay = -1;
	boost::shared_ptr<Item> savedItem;

	int itemsFound = 0; /*This keeps track of how many items we've found of the right category,
						we can use this to know when we've searched through all of the items*/

	for (std::map<Coordinate, boost::shared_ptr<Container> >::iterator conti = containers.begin(); 
		conti != containers.end() && itemsFound < amount[cat]; ++conti) {
		if (!conti->second->empty()) {
			boost::weak_ptr<Item> witem = *conti->second->begin();
			if (boost::shared_ptr<Item> item = witem.lock()) {
				if (item->IsCategory(cat) && !item->Reserved()) {
					//The item is the one we want, check that it fullfills all the requisite flags
					++itemsFound;

					if (flags & NOTFULL && boost::dynamic_pointer_cast<Container>(item)) {
						boost::shared_ptr<Container> container = boost::static_pointer_cast<Container>(item);
						//value represents bulk in this case. Needs to check Full() because bulk=value=0 is a possibility
						if (container->Full() || container->Capacity() < value) continue;
					}

					if (flags & BETTERTHAN) {
						if (item->RelativeValue() <= value) continue;
					} 

					if (flags & APPLYMINIMUMS) {
						/*For now this only affects seeds. With this flag set don't return
						seeds if at or below the set minimum for them*/
						if (item->IsCategory(Item::StringToItemCategory("Seed"))) {
							if (StockManager::Inst()->TypeQuantity(item->Type()) <=
								StockManager::Inst()->Minimum(item->Type()))
								continue;
						}
					} 

					if (flags & EMPTY && boost::dynamic_pointer_cast<Container>(item)) {
						if (!boost::static_pointer_cast<Container>(item)->empty() ||
							boost::static_pointer_cast<Container>(item)->GetReservedSpace() > 0) continue;
					}

					if (flags & MOSTDECAYED) {
						if (decay == -1 || decay > item->GetDecay()) { //First item or closer to decay
							decay = item->GetDecay();
							savedItem = item;
						}
						continue; //Always continue, we need to look through all the items
					}

					return item;

				} else if (boost::dynamic_pointer_cast<Container>(item)) {
					//This item is not the one we want, but it might contain what we're looking for.
					boost::weak_ptr<Container> cont = boost::static_pointer_cast<Container>(item);

					for (std::set<boost::weak_ptr<Item> >::iterator itemi = cont.lock()->begin(); itemi != cont.lock()->end(); ++itemi) {
						boost::shared_ptr<Item> innerItem(itemi->lock());
						if (innerItem && innerItem->IsCategory(cat) && !innerItem->Reserved()) {

							++itemsFound;

							if (flags & BETTERTHAN) {
								if (innerItem->RelativeValue() <= value) continue;
							}
							if (flags & APPLYMINIMUMS) {
								/*For now this only affects seeds. With this flag set don't return
								seeds if at or below the set minimum for them*/
								if (innerItem->IsCategory(Item::StringToItemCategory("Seed"))) {
									if (StockManager::Inst()->TypeQuantity(innerItem->Type()) <=
										StockManager::Inst()->Minimum(innerItem->Type())) 
											continue;
								}
							}

							if (flags & MOSTDECAYED) {
								if (decay == -1 || decay > innerItem->GetDecay()) { //First item or closer to decay
									decay = innerItem->GetDecay();
									savedItem = innerItem;
								}
								continue; //Always continue, we need to look through all the items
							}
							
							return innerItem;
						}
					}
				}
			}
		}
	}

	return savedItem;
}

boost::weak_ptr<Item> Stockpile::FindItemByType(ItemType typeValue, int flags, int value) {

	//These two are used only for MOSTDECAYED
	int decay = -1;
	boost::shared_ptr<Item> savedItem;

	int itemsFound = 0; //This keeps track of how many items we've found of the right category
	ItemCategory cat = *Item::Presets[typeValue].categories.begin(); /*Choose whatever happens to be the first
																	 category. This'll give us an inaccurate
																	 count, but it'll still make this faster*/

	for (std::map<Coordinate, boost::shared_ptr<Container> >::iterator conti = containers.begin(); 
		conti != containers.end() && itemsFound < amount[cat]; ++conti) {
		if (!conti->second->empty()) {
			boost::weak_ptr<Item> witem = *conti->second->begin();
			if (boost::shared_ptr<Item> item = witem.lock()) {
				if (item->Type() == typeValue && !item->Reserved()) {
					++itemsFound;
					if (flags & NOTFULL && boost::dynamic_pointer_cast<Container>(item)) {
						boost::shared_ptr<Container> container = boost::static_pointer_cast<Container>(item);
						//value represents bulk in this case
						if (container->Full() || container->Capacity() < value) continue;
					}

					if (flags & BETTERTHAN) {
						if (item->RelativeValue() <= value) continue;
					} 

					if (flags & APPLYMINIMUMS) {
						/*For now this only affects seeds. With this flag set don't return
						seeds if at or below the set minimum for them*/
						if (item->IsCategory(Item::StringToItemCategory("Seed"))) {
							if (StockManager::Inst()->TypeQuantity(item->Type()) <=
								StockManager::Inst()->Minimum(item->Type()))
								continue;
						}
					} 

					if (flags & EMPTY && boost::dynamic_pointer_cast<Container>(item)) {
						if (!boost::static_pointer_cast<Container>(item)->empty() ||
							boost::static_pointer_cast<Container>(item)->GetReservedSpace() > 0) continue;
					}

					if (flags & MOSTDECAYED) {
						if (decay == -1 || decay > item->GetDecay()) { //First item or closer to decay
							decay = item->GetDecay();
							savedItem = item;
						}
						continue; //Always continue, we need to look through all the items
					}
					
					return item;
				} else if (boost::dynamic_pointer_cast<Container>(item)) {
					boost::weak_ptr<Container> cont = boost::static_pointer_cast<Container>(item);
					for (std::set<boost::weak_ptr<Item> >::iterator itemi = cont.lock()->begin(); itemi != cont.lock()->end(); ++itemi) {
						boost::shared_ptr<Item> innerItem(itemi->lock());
						if (innerItem && innerItem->Type() == typeValue && !innerItem->Reserved()) {
							++itemsFound;
							if (flags & BETTERTHAN) {
								if (innerItem->RelativeValue() <= value) continue;
							}

							if (flags & APPLYMINIMUMS) {
								/*For now this only affects seeds. With this flag set don't return
								seeds if at or below the set minimum for them*/
								if (innerItem->IsCategory(Item::StringToItemCategory("Seed"))) {
									if (StockManager::Inst()->TypeQuantity(innerItem->Type()) <=
										StockManager::Inst()->Minimum(innerItem->Type())) 
										continue;
								} 
							}

							if (flags & MOSTDECAYED) {
								if (decay == -1 || decay > innerItem->GetDecay()) { //First item or closer to decay
									decay = innerItem->GetDecay();
									savedItem = innerItem;
								}
								continue; //Always continue, we need to look through all the items
							}

							return innerItem;
						}
					}
				}
			}
		}
	}
	return savedItem;
}

void Stockpile::Expand(Coordinate from, Coordinate to) {
	//We can assume that from < to

	//The algorithm: Check each tile inbetween from and to, and if a tile is adjacent to this
	//stockpile, add it. Do this max(width,height) times.
	int repeats = std::max(to.X() - from.X(), to.Y() - from.Y());
	for (int repeatCount = 0; repeatCount <= repeats; ++repeatCount) {
		for (int ix = from.X(); ix <= to.X(); ++ix) {
			for (int iy = from.Y(); iy <= to.Y(); ++iy) {
				if (Map::Inst()->GetConstruction(ix,iy) == -1 && Map::Inst()->Buildable(ix,iy) &&
					Construction::Presets[type].tileReqs.find(Map::Inst()->Type(ix,iy)) != Construction::Presets[type].tileReqs.end()) {
					if (Map::Inst()->GetConstruction(ix-1,iy) == uid ||
						Map::Inst()->GetConstruction(ix+1,iy) == uid ||
						Map::Inst()->GetConstruction(ix,iy-1) == uid ||
						Map::Inst()->GetConstruction(ix,iy+1) == uid) {
							//Current tile is walkable, buildable, and adjacent to the current stockpile
							Map::Inst()->SetConstruction(ix,iy,uid);
							Map::Inst()->Buildable(ix,iy,false);
							Map::Inst()->SetTerritory(ix,iy,true);
							//Update corner values
							if (ix < a.X()) a.X(ix);
							if (ix > b.X()) b.X(ix);
							if (iy < a.Y()) a.Y(iy);
							if (iy > b.Y()) b.Y(iy);
							reserved.insert(std::pair<Coordinate,bool>(Coordinate(ix,iy),false));
							Container *container = new Container(Coordinate(ix,iy), 0, 1000, -1);
							container->AddListener(this);
							containers.insert(std::pair<Coordinate,boost::shared_ptr<Container> >(Coordinate(ix,iy), boost::shared_ptr<Container>(container)));

							//Update color
							colors.insert(std::pair<Coordinate, TCODColor>(Coordinate(ix,iy), TCODColor::lerp(color, Map::Inst()->GetColor(ix, iy), 0.75f)));
					}
				}
			}
		}
	}
}

void Stockpile::Draw(Coordinate upleft, TCODConsole* console) {
	int screenx, screeny;

	for (int x = a.X(); x <= b.X(); ++x) {
		for (int y = a.Y(); y <= b.Y(); ++y) {
			if (Map::Inst()->GetConstruction(x,y) == uid) {
				screenx = x  - upleft.X();
				screeny = y - upleft.Y();
				if (screenx >= 0 && screenx < console->getWidth() && screeny >= 0 &&
					screeny < console->getHeight()) {
						if (dismantle) console->setCharBackground(screenx,screeny, TCODColor::darkGrey);
						console->setCharForeground(screenx, screeny, colors[Coordinate(x,y)]);
						console->setChar(screenx, screeny, (graphic[1]));

						if (!containers[Coordinate(x,y)]->empty()) {
							boost::weak_ptr<Item> item = *containers[Coordinate(x,y)]->begin();
							if (item.lock()) {
								item.lock()->Draw(upleft, console);
							}
						}
				}
			}
		}
	}
}

bool Stockpile::Allowed(ItemCategory cat) {
	return allowed[cat];
}

//Return true if any given category is allowed, this allows stockpiles to take axes, even if slashing weapons are disallowed for ex.
bool Stockpile::Allowed(std::set<ItemCategory> cats) {
	for (std::set<ItemCategory>::iterator cati = cats.begin(); cati != cats.end(); ++cati) {
		if (Allowed(*cati)) return true;
	}
	return false;
}

bool Stockpile::Full(ItemType type) {
	for (int ix = a.X(); ix <= b.X(); ++ix) {
		for (int iy = a.Y(); iy <= b.Y(); ++iy) {
			if (Map::Inst()->GetConstruction(ix,iy) == uid) {
				Coordinate location(ix,iy);
				//If the stockpile has hit the limit then it's full for this itemtype
				if (type != 1) {
					for (std::set<ItemCategory>::iterator cati = Item::Presets[type].categories.begin();
						cati != Item::Presets[type].categories.end(); ++cati) {
							if (GetLimit(*cati) > 0 && amount[*cati] >= GetLimit(*cati)) return true;
					}
				}

				//If theres a free space then it obviously is not full
				if (containers[location]->empty() && !reserved[location]) return false;

				//Check if a container exists for this ItemCategory that isn't full
				boost::weak_ptr<Item> item = containers[location]->GetFirstItem();
				if (item.lock() && item.lock()->IsCategory(Item::StringToItemCategory("Container"))) {
					boost::shared_ptr<Container> container = boost::static_pointer_cast<Container>(item.lock());
					if (type != -1 && container->IsCategory(Item::Presets[type].fitsin) && 
						container->Capacity() >= Item::Presets[type].bulk) return false;
				}
			}
		}
	}
	return true;
}

Coordinate Stockpile::FreePosition() {
	//First attempt to find a random position
	for (int i = 0; i < std::max(1, (signed int)containers.size()/4); ++i) {
		std::map<Coordinate, boost::shared_ptr<Container> >::iterator conti = boost::next(containers.begin(), Random::Choose(containers));
		if (conti->second->empty() && !reserved[conti->first]) return conti->first;
	}
	//If that fails still iterate through each position because a free position _should_ exist
	for (int ix = a.X(); ix <= b.X(); ++ix) {
		for (int iy = a.Y(); iy <= b.Y(); ++iy) {
			if (Map::Inst()->GetConstruction(ix,iy) == uid) {
				if (containers[Coordinate(ix,iy)]->empty() && !reserved[Coordinate(ix,iy)]) return Coordinate(ix,iy);
			}
		}
	}
	return Coordinate(-1,-1);
}

void Stockpile::ReserveSpot(Coordinate pos, bool val, ItemType type) { 
	reserved[pos] = val;

	/*Update amounts based on reserves if limits exist for the item
	This is necessary to stop too many stockpilation jobs being queued up*/
	if (type >= 0) {
		for (std::set<ItemCategory>::iterator cati = Item::Presets[type].categories.begin();
			cati != Item::Presets[type].categories.end(); ++cati) {
				if (GetLimit(*cati) >= 0) {
					amount[*cati] += val ? 1 : -1;
				}
		}
	}
}

boost::weak_ptr<Container> Stockpile::Storage(Coordinate pos) {
	return containers[pos];
}

void Stockpile::SwitchAllowed(ItemCategory cat, bool childrenAlso) {
	allowed[cat] = !allowed[cat];
	if (childrenAlso) {
		for (std::map<ItemCategory, bool>::iterator alli = boost::next(allowed.find(cat)); alli != allowed.end(); ++alli) {
			if (Item::Categories[alli->first].parent &&
				Item::Categories[alli->first].parent->name == Item::Categories[cat].name) {
				alli->second = allowed[cat];
			} else {
				break;
			}
		}
	}
	Game::Inst()->RefreshStockpiles();
}

void Stockpile::SetAllAllowed(bool nallowed) {
	for(unsigned int i = 0; i < Item::Categories.size(); i++) {
		allowed[i] = nallowed;
	}
	Game::Inst()->RefreshStockpiles();
}

void Stockpile::ItemAdded(boost::weak_ptr<Item> item) {
	std::set<ItemCategory> categories = Item::Presets[item.lock()->Type()].categories;
	for(std::set<ItemCategory>::iterator it = categories.begin(); it != categories.end(); it++) {
		amount[*it] = amount[*it] + 1;
	}

    if(item.lock()->IsCategory(Item::StringToItemCategory("Container"))) {
        boost::shared_ptr<Container> container = boost::static_pointer_cast<Container>(item.lock());
        for(std::set<boost::weak_ptr<Item> >::iterator i = container->begin(); i != container->end(); i++) {
            ItemAdded(*i);
        }
        container->AddListener(this);
    }
}

void Stockpile::ItemRemoved(boost::weak_ptr<Item> item) {
    if(item.lock()->IsCategory(Item::StringToItemCategory("Container"))) {
        boost::shared_ptr<Container> container = boost::static_pointer_cast<Container>(item.lock());
        container->RemoveListener(this);
        for(std::set<boost::weak_ptr<Item> >::iterator i = container->begin(); i != container->end(); i++) {
            ItemRemoved(*i);
        }
    }

	std::set<ItemCategory> categories = Item::Presets[item.lock()->Type()].categories;
	for(std::set<ItemCategory>::iterator it = categories.begin(); it != categories.end(); it++) {
		amount[*it] = amount[*it] - 1;
	}
}

struct AmountCompare {
	bool operator()(const std::pair<ItemCategory, int> &lhs, const std::pair<ItemCategory, int> &rhs) {
		return lhs.second > rhs.second;
	}
};

void Stockpile::GetTooltip(int x, int y, Tooltip *tooltip) {
	if (containers.find(Coordinate(x,y)) != containers.end()) {
		if(!containers[Coordinate(x, y)]->empty()) {
			boost::weak_ptr<Item> item = containers[Coordinate(x, y)]->GetFirstItem();
			if(item.lock()) {
				item.lock()->GetTooltip(x, y, tooltip);
			}
		}
	}

	tooltip->AddEntry(TooltipEntry(name, TCODColor::white));
	std::vector<std::pair<ItemCategory, int> > vecView = std::vector<std::pair<ItemCategory, int> >();
	for(std::map<ItemCategory, int>::iterator it = amount.begin(); it != amount.end(); it++) {
		if(Item::Categories[it->first].parent == 0 && it->second > 0) {
			vecView.push_back(*it);
		}
	}
	if(!vecView.empty()) {
		std::sort(vecView.begin(), vecView.end(), AmountCompare());
		int count = 0;
		for(size_t i = 0; i < vecView.size(); i++) {
			if(++count > 30) {
				tooltip->AddEntry(TooltipEntry(" ...", TCODColor::grey));
				return;
			}
			tooltip->AddEntry(TooltipEntry((boost::format(" %s x%d") % Item::ItemCategoryToString(vecView[i].first) % vecView[i].second).str(), TCODColor::grey));

			for(std::vector<ItemCat>::iterator cati = Item::Categories.begin(); cati != Item::Categories.end(); cati++) {
				if(cati->parent && Item::StringToItemCategory(cati->parent->GetName()) == vecView[i].first) {
					int amt = amount[Item::StringToItemCategory(cati->GetName())];
					if (amt > 0) {
						if(++count > 30) {
							tooltip->AddEntry(TooltipEntry(" ...", TCODColor::grey));
							return;
						}
						tooltip->AddEntry(TooltipEntry((boost::format("	 %s x%d") % cati->GetName() % amt).str(), TCODColor::grey));
					}
				}
			}
		}
	}
}

Coordinate Stockpile::Center() {
	return Coordinate((a.X() + b.X() - 1) / 2, (a.Y() + b.Y() - 1) / 2);
}

void Stockpile::TranslateInternalContainerListeners() {
	for (std::map<Coordinate, boost::shared_ptr<Container> >::iterator it = containers.begin();
		it != containers.end(); ++it) {
			it->second->TranslateContainerListeners();
	}
}

void Stockpile::AdjustLimit(ItemCategory category, int amount) {
	if (limits.find(category) != limits.end()) {
		limits[category] = amount;
	}
}

int Stockpile::GetLimit(ItemCategory category) { 
	if (limits.find(category) != limits.end())
		return limits[category];
	else
		return -1;
}