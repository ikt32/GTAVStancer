#pragma once
#include <vector>

/*
 * Format: { name, type }
 * name: Menu item name
 * type: Menu item type: "option" or "submenu"
 */ 
typedef std::pair<std::string, std::string> MenuPair;

/*
 * Each "screen" has a title and then shows some options beneath it
 */
class MenuScreen
{
public:
	MenuScreen(std::string title, std::vector<MenuPair> menuOptions);
	~MenuScreen();
	std::string Title;
	std::vector<MenuPair> Options;
};


class Menu {
public:
	Menu();
	~Menu();
	void Draw(MenuScreen screen, int selectedIndex, int fontType, float fontSize, float xPos, float yPos);
	void Beep();
	void Menu::GetButtonState(bool *a, bool *b, bool *up, bool *down, bool *l, bool *r);
	int CurrentIndex;
private:
	std::string option = "option";
	std::string submenu = "submenu";
	void drawRect(float x, float y, float width, float height, int r, int g, int b, int a);
};
