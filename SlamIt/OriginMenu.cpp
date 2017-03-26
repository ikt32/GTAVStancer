/* OriginMenu.cpp */

#include <bemapiset.h>
#include "MenuClass.h"

int testint = 0;
float testfloat = 0;
bool testbool = false;

int testarray[] = { 1, 3, 5, 10 };
int testarraypointer = 0;

bool firstload = true;
LPCWSTR menuStyleLocation = L".\\OriginBase\\MenuStyle.ini";
void OriginMenu() {
	if (firstload) {
		Menu::LoadMenuTheme(menuStyleLocation);

		firstload = false;
	}

	Menu::checkKeys();

	// Has to look for mainmenu otherwise the code fails due to me setting menu to mainmenu on Keypress
	if (Menu::currentMenu("mainmenu")) {
		Menu::Title("Main Menu");

		Menu::Option("Test Option");

		Menu::MenuOption("Test Menu", "testmenu");

		Menu::IntOption("Test Int", &testint, 0, 10);
		Menu::IntOption("Test Int Custom Step", &testint, 0, 10, 5);

		Menu::FloatOption("Test Float", &testfloat, 0, 10);
		Menu::FloatOption("Test Float Custom Step", &testfloat, 0, 10, 0.2f);

		Menu::BoolOption("Test Bool", &testbool);

		Menu::IntArray("Test Int Array", testarray, &testarraypointer);

		Menu::MenuOption("Settings Menu", "settings");
	}

	if (Menu::currentMenu("testmenu")) {
		Menu::Title("Test Menu");

		Menu::Option("Filler Options");
		Menu::Option("Filler Options");
		Menu::Option("Filler Options");
		Menu::Option("Filler Options");
		Menu::Option("Filler Options");
		Menu::Option("Filler Options");
		Menu::Option("Filler Options");
		Menu::Option("Filler Options");
		Menu::Option("Filler Options");
		Menu::Option("Filler Options");
		Menu::Option("Filler Options");
		Menu::Option("Filler Options");
		Menu::Option("Filler Options");
		Menu::Option("Filler Options");
		Menu::Option("Filler Options");
		Menu::Option("Filler Options");
		Menu::Option("Filler Options");
		Menu::Option("Filler Options");
		Menu::Option("Filler Options");
		Menu::Option("Filler Options");
		Menu::Option("Filler Options");
	}

	if (Menu::currentMenu("settings")) {
		Menu::Title("Settings Menu");

		Menu::MenuOption("Theme", "settings_theme");
	}
	if (Menu::currentMenu("settings_theme")) {
		Menu::Title("Theme");

		Menu::MenuOption("Title Text", "settings_theme_titletext");
		Menu::MenuOption("Title Rect", "settings_theme_titlerect");
		Menu::MenuOption("Scroller", "settings_theme_scroller");
		Menu::MenuOption("Options Text", "settings_theme_options");
		Menu::MenuOption("Options Rect", "settings_theme_optionsrect");
		if (Menu::Option("Save Theme")) Menu::SaveMenuTheme(menuStyleLocation);
		if (Menu::Option("Load Theme")) Menu::LoadMenuTheme(menuStyleLocation);
		if (Menu::Option("Revert To Default")) {
			titleText = { 0, 0, 0, 255 };
			titleRect = { 255, 200, 0, 255 };
			scroller = { 80, 80, 80, 200 };
			options = { 0, 0, 0, 255 };
			optionsrect = { 255, 220, 30, 60 };
		}
	}
	if (Menu::currentMenu("settings_theme_titletext")) {
		Menu::Title("Title Text");

		Menu::IntOption("Red: ", &titleText.r, 0, 255);
		Menu::IntOption("Green: ", &titleText.g, 0, 255);
		Menu::IntOption("Blue: ", &titleText.b, 0, 255);
		Menu::IntOption("Alpha: ", &titleText.a, 0, 255);
	}
	if (Menu::currentMenu("settings_theme_titlerect")) {
		Menu::Title("Title Rect");

		Menu::IntOption("Red: ", &titleRect.r, 0, 255);
		Menu::IntOption("Green: ", &titleRect.g, 0, 255);
		Menu::IntOption("Blue: ", &titleRect.b, 0, 255);
		Menu::IntOption("Alpha: ", &titleRect.a, 0, 255);
	}
	if (Menu::currentMenu("settings_theme_scroller")) {
		Menu::Title("Scroller");

		Menu::IntOption("Red: ", &scroller.r, 0, 255);
		Menu::IntOption("Green: ", &scroller.g, 0, 255);
		Menu::IntOption("Blue: ", &scroller.b, 0, 255);
		Menu::IntOption("Alpha: ", &scroller.a, 0, 255);
	}
	if (Menu::currentMenu("settings_theme_options")) {
		Menu::Title("Options Text");

		Menu::IntOption("Red: ", &options.r, 0, 255);
		Menu::IntOption("Green: ", &options.g, 0, 255);
		Menu::IntOption("Blue: ", &options.b, 0, 255);
		Menu::IntOption("Alpha: ", &options.a, 0, 255);
	}
	if (Menu::currentMenu("settings_theme_optionsrect")) {
		Menu::Title("Options Rect");

		Menu::IntOption("Red: ", &optionsrect.r, 0, 255);
		Menu::IntOption("Green: ", &optionsrect.g, 0, 255);
		Menu::IntOption("Blue: ", &optionsrect.b, 0, 255);
		Menu::IntOption("Alpha: ", &optionsrect.a, 0, 255);
	}

	Menu::endMenu();

}