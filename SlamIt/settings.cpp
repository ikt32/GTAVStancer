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
    enableHeight = settingsGeneral.GetBoolValue("OPTIONS", "EnableHeight", false);
}


void Settings::SaveSettings() {
	CSimpleIniA settings;
	settings.SetUnicode();
	settings.LoadFile(settingsGeneralFile.c_str());

	settings.SetBoolValue("OPTIONS", "EnableMod", enableMod);
    settings.SetBoolValue("OPTIONS", "AutoApply", autoApply);
    settings.SetBoolValue("OPTIONS", "EnableHeight", enableHeight);
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
		float visualHeight = -1337.0f;
        Preset::WheelPhys frontWheels { -1337.0f, -1337.0f };
        Preset::WheelPhys rearWheels { -1337.0f, -1337.0f };
        Preset::WheelVis visualSize { -1337.0f, -1337.0f, -1, -1};

		pRoot->FirstChildElement("front")->QueryFloatAttribute("camber", &camber);
		pRoot->FirstChildElement("front")->QueryFloatAttribute("trackWidth", &trackWidth);
		pRoot->FirstChildElement("front")->QueryFloatAttribute("height", &height);
		struct Preset::Suspension front = { camber, trackWidth, height };

		pRoot->FirstChildElement("rear")->QueryFloatAttribute("camber", &camber);
		pRoot->FirstChildElement("rear")->QueryFloatAttribute("trackWidth", &trackWidth);
		pRoot->FirstChildElement("rear")->QueryFloatAttribute("height", &height);
		struct Preset::Suspension rear = { camber, trackWidth, height };

        if (pRoot->FirstChildElement("visualHeight") != nullptr) {
            pRoot->FirstChildElement("visualHeight")->QueryFloatAttribute("value", &visualHeight);
        }

        if (pRoot->FirstChildElement("frontWheels") != nullptr) {
            float rad, width;
            pRoot->FirstChildElement("frontWheels")->QueryFloatAttribute("tyreRadius", &rad);
            pRoot->FirstChildElement("frontWheels")->QueryFloatAttribute("tyreWidth", &width);
            frontWheels = { rad, width };
        }

        if (pRoot->FirstChildElement("rearWheels") != nullptr) {
            float rad, width;
            pRoot->FirstChildElement("rearWheels")->QueryFloatAttribute("tyreRadius", &rad);
            pRoot->FirstChildElement("rearWheels")->QueryFloatAttribute("tyreWidth", &width);
            rearWheels = { rad, width };
        }

        if (pRoot->FirstChildElement("visualSize") != nullptr) {
            int wheelType, wheelIndex;
            float size, width;
            pRoot->FirstChildElement("visualSize")->QueryFloatAttribute("size", &size);
            pRoot->FirstChildElement("visualSize")->QueryFloatAttribute("width", &width);
            pRoot->FirstChildElement("visualSize")->QueryIntAttribute("type", &wheelType);
            pRoot->FirstChildElement("visualSize")->QueryIntAttribute("index", &wheelIndex);
            visualSize = { size, width, wheelType, wheelIndex };
        }

		presets.push_back(Preset(front, rear, frontWheels, rearWheels, visualSize ,visualHeight, name, plate));
		pRoot = pRoot->NextSibling();
	}
	
	return presets;
}

void Settings::insertPreset(Preset preset, tinyxml2::XMLDocument &doc, tinyxml2::XMLNode *pRoot) {
    tinyxml2::XMLElement *pvehicleName = doc.NewElement("vehicleName");
    pvehicleName->SetText(preset.Name().c_str());
    pRoot->InsertEndChild(pvehicleName);

    tinyxml2::XMLElement *pplate = doc.NewElement("plate");
    pplate->SetText(preset.Plate().c_str());
    pRoot->InsertEndChild(pplate);

    tinyxml2::XMLElement *pfront = doc.NewElement("front");
    pfront->SetAttribute("camber", preset.FrontSuspension.Camber);
    pfront->SetAttribute("trackWidth", preset.FrontSuspension.TrackWidth);
    pfront->SetAttribute("height", preset.FrontSuspension.Height);
    pRoot->InsertEndChild(pfront);

    tinyxml2::XMLElement *prear = doc.NewElement("rear");
    prear->SetAttribute("camber", preset.RearSuspension.Camber);
    prear->SetAttribute("trackWidth", preset.RearSuspension.TrackWidth);
    prear->SetAttribute("height", preset.RearSuspension.Height);
    pRoot->InsertEndChild(prear);

    tinyxml2::XMLElement *pvisualHeight = doc.NewElement("visualHeight");
    pvisualHeight->SetAttribute("value", preset.VisualHeight);
    pRoot->InsertEndChild(pvisualHeight);

    tinyxml2::XMLElement *pfrontWheels = doc.NewElement("frontWheels");
    pfrontWheels->SetAttribute("tyreRadius", preset.FrontWheels.TyreRadius);
    pfrontWheels->SetAttribute("tyreWidth", preset.FrontWheels.TyreWidth);
    pRoot->InsertEndChild(pfrontWheels);

    tinyxml2::XMLElement *prearWheels = doc.NewElement("rearWheels");
    prearWheels->SetAttribute("tyreRadius", preset.RearWheels.TyreRadius);
    prearWheels->SetAttribute("tyreWidth", preset.RearWheels.TyreWidth);
    pRoot->InsertEndChild(prearWheels);

    tinyxml2::XMLElement *pvisualSize = doc.NewElement("visualSize");
    pvisualSize->SetAttribute("size", preset.VisualSize.WheelSize);
    pvisualSize->SetAttribute("width", preset.VisualSize.WheelWidth);
    pvisualSize->SetAttribute("type", preset.VisualSize.WheelType);
    pvisualSize->SetAttribute("index", preset.VisualSize.WheelIndex);
    pRoot->InsertEndChild(pvisualSize);
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
	
    XMLNode *pRoot = doc.NewElement("preset");
	doc.InsertFirstChild(pRoot);

	insertPreset(preset, doc, pRoot);

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

	while (pRoot != nullptr) {
		if (pRoot->FirstChildElement("vehicleName")->GetText() == preset.Name() &&
			pRoot->FirstChildElement("plate")->GetText() == preset.Plate()) {
            pRoot->DeleteChildren();

            insertPreset(preset, doc, pRoot);

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
