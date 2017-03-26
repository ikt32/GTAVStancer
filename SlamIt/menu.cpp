#include "menu.h"
#include "../../ScriptHookV_SDK/inc/natives.h"
#include "../../ScriptHookV_SDK/inc/enums.h"
#include "../../GTAVManualTransmission/Gears/Util/Util.hpp"

MenuScreen::MenuScreen(std::string title, std::vector<MenuPair> menuOptions)
	: Title(title)
	, Options(menuOptions) { }

MenuScreen::~MenuScreen() {}

Menu::Menu() {}
Menu::~Menu() {}

void Menu::Draw(MenuScreen screen, int selectedIndex, int fontType, float fontSize, float xPos, float yPos) {
	int rows = 0;
	float distance = 0.025f;
	float menuWidth = 0.2f;
	float optionValueOffset = 0.15f;

	UI::HIDE_HELP_TEXT_THIS_FRAME();
	CAM::SET_CINEMATIC_BUTTON_ACTIVE(0);
	UI::HIDE_HUD_COMPONENT_THIS_FRAME(10);
	UI::HIDE_HUD_COMPONENT_THIS_FRAME(6);
	UI::HIDE_HUD_COMPONENT_THIS_FRAME(7);
	UI::HIDE_HUD_COMPONENT_THIS_FRAME(9);
	UI::HIDE_HUD_COMPONENT_THIS_FRAME(8);
	CONTROLS::DISABLE_CONTROL_ACTION(2, ControlNextCamera, true);
	CONTROLS::DISABLE_CONTROL_ACTION(2, ControlPhone, true);
	CONTROLS::DISABLE_CONTROL_ACTION(2, ControlVehicleCinCam, true);
	CONTROLS::DISABLE_CONTROL_ACTION(2, ControlSelectCharacterFranklin, true);
	CONTROLS::DISABLE_CONTROL_ACTION(2, ControlSelectCharacterMichael, true);
	CONTROLS::DISABLE_CONTROL_ACTION(2, ControlSelectCharacterTrevor, true);
	CONTROLS::DISABLE_CONTROL_ACTION(2, ControlSelectCharacterMultiplayer, true);
	CONTROLS::DISABLE_CONTROL_ACTION(2, ControlCharacterWheel, true);
	CONTROLS::DISABLE_CONTROL_ACTION(2, ControlMeleeAttackLight, true);
	CONTROLS::DISABLE_CONTROL_ACTION(2, ControlMeleeAttackHeavy, true);
	CONTROLS::DISABLE_CONTROL_ACTION(2, ControlMeleeAttackAlternate, true);
	CONTROLS::DISABLE_CONTROL_ACTION(2, ControlMultiplayerInfo, true);
	CONTROLS::DISABLE_CONTROL_ACTION(2, ControlMapPointOfInterest, true);
	CONTROLS::DISABLE_CONTROL_ACTION(2, ControlWeaponWheelNext, true);

	showText(xPos, yPos + rows*distance, fontSize, screen.Title.c_str());
	drawRect(xPos, yPos + rows*distance, menuWidth, distance, 32, 64, 128, 128);

	rows++;

	int itemIndex = 0;
	for (auto option : screen.Options) {
		showText(xPos, yPos + rows*distance, fontSize, option.first.c_str(), fontType, {255, 255, 255, 255});
		if (option.second == submenu) {
			showText(xPos + optionValueOffset, yPos + rows*distance, fontSize, ">", fontType, {255, 255, 255, 255});
		} else {
			showText(xPos + optionValueOffset, yPos + rows*distance, fontSize, "???", fontType, { 255, 255, 255, 255 });
		}
		if (itemIndex == selectedIndex) {
			drawRect(xPos, yPos + rows*distance, menuWidth, distance, 0, 0, 0, 200);
		} else {
			drawRect(xPos, yPos + rows*distance, menuWidth, distance, 0, 0, 0, 128);
		}
		rows++;
		itemIndex++;
	}
}

void Menu::Beep() {
	AUDIO::PLAY_SOUND_FRONTEND(-1, "NAV_UP_DOWN", "HUD_FRONTEND_DEFAULT_SOUNDSET", 0);
}

void Menu::GetButtonState(bool *a, bool *b, bool *up, bool *down, bool *l, bool *r) {
	//if (a) *a = controls.IsKeyJustPressed(controls.MenuSelect);
	//if (b) *b = controls.IsKeyJustPressed(controls.MenuCancel);
	//if (up) *up = controls.IsKeyJustPressed(controls.MenuUp);
	//if (down) *down = controls.IsKeyJustPressed(controls.MenuDown);
	//if (r) *r = controls.IsKeyJustPressed(controls.MenuLeft);
	//if (l) *l = controls.IsKeyJustPressed(controls.MenuRight);
}

void Menu::drawRect(float x, float y, float width, float height, int r, int g, int b, int a) {
	GRAPHICS::DRAW_RECT((x + (width * 0.5f)), (y + (height * 0.5f)), width, height, r, g, b, a);
}
