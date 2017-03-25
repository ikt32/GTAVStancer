#include "settings.h"
#include "controls.h"
#include "../../GTAVManualTransmission/Gears/Util/simpleini/SimpleIni.h"
#include "keyboard.h"
#include "presets.h"
#include <vector>
#include "../tinyxml2/tinyxml2.h"

Settings::Settings(const std::string &settingsFile): settingsFile(settingsFile) {
}


Settings::~Settings() {
}

void Settings::ReadSettings(Controls *control) {

	CSimpleIniA settings;
	settings.SetUnicode();
	settings.LoadFile(settingsFile.c_str());
	
	enableMod = settings.GetBoolValue("OPTIONS", "EnableMod", false);
	autoApply = settings.GetBoolValue("OPTIONS", "AutoApply", false);
	
	control->controlKeys[Controls::ControlType::MenuKey]	= str2key(settings.GetValue("MENU", "MenuKey",		"VK_OEM_4"));
	control->controlKeys[Controls::ControlType::MenuUp]		= str2key(settings.GetValue("MENU", "MenuUp",		"UP"));
	control->controlKeys[Controls::ControlType::MenuDown]	= str2key(settings.GetValue("MENU", "MenuDown",		"DOWN"));
	control->controlKeys[Controls::ControlType::MenuLeft]	= str2key(settings.GetValue("MENU", "MenuLeft",		"LEFT"));
	control->controlKeys[Controls::ControlType::MenuRight]	= str2key(settings.GetValue("MENU", "MenuRight",	"RIGHT"));
	control->controlKeys[Controls::ControlType::MenuSelect] = str2key(settings.GetValue("MENU", "MenuSelect",	"RETURN"));
	control->controlKeys[Controls::ControlType::MenuCancel] = str2key(settings.GetValue("MENU", "MenuCancel",	"BACKSPACE"));
}

std::vector<Preset> Settings::ReadPresets(const std::string &fileName) {
	using namespace tinyxml2;
	tinyxml2::XMLDocument doc;
	XMLError err = doc.LoadFile(fileName.c_str());

	if (err != XML_SUCCESS) {
		throw new std::runtime_error("Can't load " + fileName);
	}
	XMLHandle hDoc(&doc);
	
	XMLNode * pRoot = doc.FirstChildElement("preset");
	if (!pRoot) {
		throw new std::runtime_error("Can't read " + fileName);
	}
	std::vector<Preset> presets;

	while (pRoot != nullptr) {
		std::string name = pRoot->FirstChildElement("vehicleName")->GetText();
		std::string plate = pRoot->FirstChildElement("plate")->GetText();
		float camber;
		double distance;
		float height;
		pRoot->FirstChildElement("front")->QueryFloatAttribute("camber", &camber);
		pRoot->FirstChildElement("front")->QueryDoubleAttribute("distance", &distance);
		pRoot->FirstChildElement("front")->QueryFloatAttribute("height", &height);
		struct Preset::WheelInfo front = { camber, distance, height };

		pRoot->FirstChildElement("rear")->QueryFloatAttribute("camber", &camber);
		pRoot->FirstChildElement("rear")->QueryDoubleAttribute("distance", &distance);
		pRoot->FirstChildElement("rear")->QueryFloatAttribute("height", &height);
		struct Preset::WheelInfo rear = { camber, distance, height };

		presets.push_back(Preset(front, rear, name, plate));
		pRoot = pRoot->NextSibling();
	}
	
	return presets;
}

bool Settings::AutoApply() {
	return autoApply;
}
bool Settings::EnableMod() {
	return enableMod;
}
