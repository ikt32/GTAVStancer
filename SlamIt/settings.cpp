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


void Settings::SaveSettings() {
	CSimpleIniA settings;
	settings.SetUnicode();
	settings.LoadFile(settingsFile.c_str());

	settings.SetBoolValue("OPTIONS", "EnableMod", enableMod);
	settings.SetBoolValue("OPTIONS", "AutoApply", autoApply);
	settings.SaveFile(settingsFile.c_str());
}

std::vector<Preset> Settings::ReadPresets(const std::string &fileName) {
	std::vector<Preset> presets;
	
	using namespace tinyxml2;
	tinyxml2::XMLDocument doc;
	XMLError err = doc.LoadFile(fileName.c_str());

	if (err != XML_SUCCESS) {
		return presets;
	}
	
	XMLNode * pRoot = doc.FirstChildElement("preset");

	while (pRoot != nullptr) {
		std::string name = "MISSINGNAME";
		std::string plate = "MISSINGPLATE";

		if (pRoot->FirstChildElement("vehicleName")->GetText() != nullptr) 
			name = pRoot->FirstChildElement("vehicleName")->GetText();

		if (pRoot->FirstChildElement("plate")->GetText() != nullptr) 
			plate = pRoot->FirstChildElement("plate")->GetText();
		
		float camber;
		float distance;
		float height;
		pRoot->FirstChildElement("front")->QueryFloatAttribute("camber", &camber);
		pRoot->FirstChildElement("front")->QueryFloatAttribute("distance", &distance);
		pRoot->FirstChildElement("front")->QueryFloatAttribute("height", &height);
		struct Preset::WheelInfo front = { camber, distance, height };

		pRoot->FirstChildElement("rear")->QueryFloatAttribute("camber", &camber);
		pRoot->FirstChildElement("rear")->QueryFloatAttribute("distance", &distance);
		pRoot->FirstChildElement("rear")->QueryFloatAttribute("height", &height);
		struct Preset::WheelInfo rear = { camber, distance, height };

		presets.push_back(Preset(front, rear, name, plate));
		pRoot = pRoot->NextSibling();
	}
	
	return presets;
}

void Settings::AppendPreset(Preset preset, const std::string &fileName) {
	using namespace tinyxml2;
	tinyxml2::XMLDocument doc;
	XMLError err = doc.LoadFile(fileName.c_str());

	if (err == XML_ERROR_EMPTY_DOCUMENT) {
		
	}
	else if (err != XML_SUCCESS) {
		return;
	}
	
	XMLElement *pRoot = doc.NewElement("preset");
	doc.InsertFirstChild(pRoot);

	XMLElement *pvehicleName = doc.NewElement("vehicleName");
	pvehicleName->SetText(preset.Name().c_str());
	pRoot->InsertEndChild(pvehicleName);

	XMLElement *pplate = doc.NewElement("plate");
	pplate->SetText(preset.Plate().c_str());
	pRoot->InsertEndChild(pplate);
	
	XMLElement *pfront = doc.NewElement("front");
	pfront->SetAttribute("camber", preset.Front.Camber);
	pfront->SetAttribute("distance", preset.Front.Distance);
	pfront->SetAttribute("height", preset.Front.Height);
	pRoot->InsertEndChild(pfront);

	XMLElement *prear = doc.NewElement("rear");
	prear->SetAttribute("camber", preset.Front.Camber);
	prear->SetAttribute("distance", preset.Front.Distance);
	prear->SetAttribute("height", preset.Front.Height);
	pRoot->InsertEndChild(prear);

	doc.SaveFile(fileName.c_str());
}

bool Settings::OverwritePreset(Preset preset, const std::string &fileName) {
	using namespace tinyxml2;
	tinyxml2::XMLDocument doc;
	XMLError err = doc.LoadFile(fileName.c_str());

	if (err != XML_SUCCESS) {
		return false;
	}

	XMLNode * pRoot = doc.FirstChildElement("preset");

	std::vector<Preset> presets;

	while (pRoot != nullptr) {
		if (pRoot->FirstChildElement("vehicleName")->GetText() == preset.Name() &&
			pRoot->FirstChildElement("plate")->GetText() == preset.Plate()) {
			XMLElement *pfront = pRoot->FirstChildElement("front");
			pfront->SetAttribute("camber", preset.Front.Camber);
			pfront->SetAttribute("distance", preset.Front.Distance);
			pfront->SetAttribute("height", preset.Front.Height);

			XMLElement *prear = pRoot->FirstChildElement("rear");
			prear->SetAttribute("camber", preset.Front.Camber);
			prear->SetAttribute("distance", preset.Front.Distance);
			prear->SetAttribute("height", preset.Front.Height);
			doc.SaveFile(fileName.c_str());
			return true;
		}
		pRoot = pRoot->NextSibling();
	}
	return false;
}

bool Settings::DeletePreset(Preset preset, const std::string &fileName) {
	using namespace tinyxml2;
	tinyxml2::XMLDocument doc;
	XMLError err = doc.LoadFile(fileName.c_str());

	if (err != XML_SUCCESS) {
		return false;
	}

	XMLNode * pRoot = doc.FirstChildElement("preset");

	std::vector<Preset> presets;

	while (pRoot != nullptr) {
		if (pRoot->FirstChildElement("vehicleName")->GetText() == preset.Name() &&
			pRoot->FirstChildElement("plate")->GetText() == preset.Plate()) {
			//TODO: Delete *this node...
			//pRoot->DeleteChild(pRoot);
			doc.SaveFile(fileName.c_str());
			return true;
		}
		pRoot = pRoot->NextSibling();
	}
	return false;
}
