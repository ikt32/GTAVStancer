/* MenuClass.h */ /* Taken from SudoMod*/

#include <string>
#include <windows.h>

class Controls;

struct rgba {
	int r, g, b, a;
};

extern float menux;
extern rgba titleText;
extern rgba titleRect;
extern rgba scroller;
extern rgba options;
extern rgba optionsrect;

class Menu {
public:
	static int getKeyPressed(int key);
	static char* StringToChar(std::string string);
	static bool currentMenu(char* menuname);
	static void changeMenu(char* menuname);
	static void backMenu();
	
	static void drawText(char* text, int font, float x, float y, float scalex, float scaley, rgba rgba, bool center);
	static void drawNotification(char* msg);
	static void drawRect(float x, float y, float width, float height, rgba rgba);
	static void drawSprite(char* Streamedtexture, char* textureName, float x, float y, float width, float height, float rotation, rgba rgba);
	
	static void Title(char* title);
	static bool Option(char* option);
	static bool MenuOption(char* option, char* menu);
	static bool IntOption(char* option, int *var, int min, int max, int step = 1);
	static bool FloatOption(char* option, float *var, float min, float max, float step = 0.1);
	static bool DoubleOption(char *option, double *var, double min, double max, double step);
	static bool BoolOption(char* option, bool *b00l);
	static bool BoolSpriteOption(char* option, bool b00l, char* category, char* spriteOn, char* spriteOff);
	static bool IntArray(char* option, int display[], int *PlaceHolderInt);
	static bool FloatArray(char* option, float display[], int *PlaceHolderInt);
	static bool CharArray(char* option, char* display[], int *PlaceHolderInt);
	static void TeleportOption(char* option, float x, float y, float z);

	static void IniWriteInt(LPCWSTR file, LPCWSTR section, LPCWSTR key, int value);
	static int IniReadInt(LPCWSTR file, LPCWSTR section, LPCWSTR key);

	static void LoadMenuTheme(LPCWSTR file);
	static void SaveMenuTheme(LPCWSTR file);

	static void endMenu();
	static void checkKeys(Controls* controls, void(*onMain)(void), void(*onExit)(void));
};