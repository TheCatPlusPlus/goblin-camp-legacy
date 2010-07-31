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

#include <string>
#include <vector>

#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/weak_ptr.hpp>
#include <libtcod.hpp>

#include "Game.hpp"
#include "UIComponents.hpp"

class MenuChoice {
public:
	MenuChoice(std::string = "", boost::function<void()> = boost::bind(Game::DoNothing));
	std::string label;
	boost::function<void()> callback;
};

class Menu: public Panel {
private:
    static std::map<std::string, Menu *> constructionCategoryMenus;
protected:
	std::vector<MenuChoice> choices;
	int _selected;
    std::string title;
	void CalculateSize();
public:
	Menu(std::vector<MenuChoice>, std::string="");
	virtual ~Menu();
	virtual void Draw(int, int, TCODConsole*);
	virtual MenuResult Update(int, int, bool, TCOD_key_t);
	void selected(int);
	void AddChoice(MenuChoice);
	void Callback(unsigned int);

	static Menu* mainMenu;
	static Menu* MainMenu();
	static Menu* constructionMenu;
	static Menu* ConstructionMenu();
	static Menu* basicsMenu;
	static Menu* BasicsMenu();
	static Menu* WorkshopsMenu();
	static Menu* ordersMenu;
	static Menu* OrdersMenu();
	static Menu* FurnitureMenu();
    static Menu* ConstructionCategoryMenu(std::string);

	static void YesNoDialog(std::string text, boost::function<void()> leftAction, boost::function<void()> rightAction,
                            std::string leftButton = "Yes", std::string rightButton = "No");
	static ItemCategory WeaponChoiceDialog();
};

class JobMenu : public Scrollable {
public:
	JobMenu() {}
	void Draw(int, int, int, int, int, TCODConsole*);
    int TotalHeight();
	static Dialog* jobListingMenu;
	static Dialog* JobListingMenu();
};

class AnnounceMenu : public Scrollable {
public:
	AnnounceMenu() {}
	void Draw(int, int, int, int, int, TCODConsole*);
    int TotalHeight();
	static Dialog* announcementsMenu;
	static Dialog* AnnouncementsMenu();
};

class NPCMenu : public Dialog {
public:
	NPCMenu();
	static void DrawNPC(std::pair<int, boost::shared_ptr<NPC> >, int, int, int, bool, TCODConsole*);
	static Dialog* npcListMenu;
	static Dialog* NPCListMenu();
};

class ConstructionMenu : public Dialog {
private:
	Construction* construct;
    class ProductList : public Scrollable {
    private:
        Construction* construct;
    public:
        ProductList(Construction* nconstruct): construct(nconstruct), height(0), productPlacement(std::vector<int>()) {}
        int height;
        std::vector<int> productPlacement;
        void Draw(int x, int y, int scroll, int width, int height, TCODConsole *);
        int TotalHeight();
        MenuResult Update(int x, int y, bool clicked, TCOD_key_t key);
    };
public:
	ConstructionMenu(int nwidth, int nheight):
    Dialog(std::vector<Drawable *>(), "", nwidth, nheight) {}
	static ConstructionMenu* constructionInfoMenu;
    static Construction* cachedConstruct;
	static ConstructionMenu* ConstructionInfoMenu(Construction*);
	void Construct(Construction*);
    void Rename();
    void Dismantle();
    void CancelJob(int);
    void AddJob(int);
    void SwitchAllowed(int);
    void DrawCategory(ItemCat, int, int, int, bool, TCODConsole *);
    void DrawJob(ItemType, int, int, int, bool, TCODConsole *);
};

class StockManagerMenu : public Menu {
private:
	int scroll;
	std::string filter;
public:
	StockManagerMenu();
	void Draw(int, int, TCODConsole*);
    MenuResult Update(int, int, bool, TCOD_key_t);
	static StockManagerMenu* stocksMenu;
	static StockManagerMenu* StocksMenu();
	void ScrollDown();
	void ScrollUp();
	void Open();
};

class SquadsMenu : public Menu {
private:
	std::string squadName;
	int squadMembers;
	int squadPriority;
	boost::weak_ptr<Squad> chosenSquad;
public:
	SquadsMenu();
	void Draw(int, int, TCODConsole*);
    MenuResult Update(int, int, bool, TCOD_key_t);
	static SquadsMenu* squadMenu;
	static SquadsMenu* SquadMenu();
	void Open();
};