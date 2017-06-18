#include "settings.h"
#include "simpleini/SimpleIni.h"
#include "keyboard.h"
#include "presets.h"
#include <vector>
#include "tinyxml2/tinyxml2.h"
#include <fstream>

Settings::Settings() { }


Settings::~Settings() { }

void Settings::SetFiles(const std::string &general) {
	settingsGeneralFile = general;
}

void Settings::ReadSettings() {

	CSimpleIniA settingsGeneral;
	settingsGeneral.SetUnicode();
	settingsGeneral.LoadFile(settingsGeneralFile.c_str());
	
	enableMod = settingsGeneral.GetBoolValue("OPTIONS", "EnableMod", false);
	autoApply = settingsGeneral.GetBoolValue("OPTIONS", "AutoApply", false);
}


void Settings::SaveSettings() {
	CSimpleIniA settings;
	settings.SetUnicode();
	settings.LoadFile(settingsGeneralFile.c_str());

	settings.SetBoolValue("OPTIONS", "EnableMod", enableMod);
	settings.SetBoolValue("OPTIONS", "AutoApply", autoApply);
	settings.SaveFile(settingsGeneralFile.c_str());
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
		float trackWidth;
		float height;
		pRoot->FirstChildElement("front")->QueryFloatAttribute("camber", &camber);
		pRoot->FirstChildElement("front")->QueryFloatAttribute("trackWidth", &trackWidth);
		pRoot->FirstChildElement("front")->QueryFloatAttribute("height", &height);
		struct Preset::WheelInfo front = { camber, trackWidth, height };

		pRoot->FirstChildElement("rear")->QueryFloatAttribute("camber", &camber);
		pRoot->FirstChildElement("rear")->QueryFloatAttribute("trackWidth", &trackWidth);
		pRoot->FirstChildElement("rear")->QueryFloatAttribute("height", &height);
		struct Preset::WheelInfo rear = { camber, trackWidth, height };

		presets.push_back(Preset(front, rear, name, plate));
		pRoot = pRoot->NextSibling();
	}
	
	return presets;
}

void Settings::AppendPreset(Preset preset, const std::string &fileName) {
	using namespace tinyxml2;
	tinyxml2::XMLDocument doc;
	XMLError err = doc.LoadFile(fileName.c_str());

	if (err == XML_ERROR_FILE_NOT_FOUND || err == XML_ERROR_EMPTY_DOCUMENT) {
		std::ofstream fs;
		fs.open(fileName, std::ios::out | std::ios_base::app);
		fs << "\n";
		fs.close();
	}
	else if (err != XML_SUCCESS) {
		throw std::runtime_error("Couldn't open/load XML with XMLError: " + std::to_string(static_cast<int>(err)));
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
	pfront->SetAttribute("trackWidth", preset.Front.TrackWidth);
	pfront->SetAttribute("height", preset.Front.Height);
	pRoot->InsertEndChild(pfront);

	XMLElement *prear = doc.NewElement("rear");
	prear->SetAttribute("camber", preset.Rear.Camber);
	prear->SetAttribute("trackWidth", preset.Rear.TrackWidth);
	prear->SetAttribute("height", preset.Rear.Height);
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
			pfront->SetAttribute("trackWidth", preset.Front.TrackWidth);
			pfront->SetAttribute("height", preset.Front.Height);

			XMLElement *prear = pRoot->FirstChildElement("rear");
			prear->SetAttribute("camber", preset.Rear.Camber);
			prear->SetAttribute("trackWidth", preset.Rear.TrackWidth);
			prear->SetAttribute("height", preset.Rear.Height);
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
			doc.DeleteNode(pRoot);
			doc.SaveFile(fileName.c_str());
			return true;
		}
		pRoot = pRoot->NextSibling();
	}
	return false;
}
