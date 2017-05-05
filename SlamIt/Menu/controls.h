#pragma once

class Settings;

class MenuControls
{
public:
	friend Settings;

	enum ControlType {
		MenuKey = 0,
		MenuUp,
		MenuDown,
		MenuLeft,
		MenuRight,
		MenuSelect,
		MenuCancel,
		SIZEOF_ControlType
	};

	MenuControls();
	~MenuControls();
	bool IsKeyPressed(ControlType control);
	bool IsKeyJustPressed(ControlType control);
	bool IsKeyJustReleased(ControlType control);
	bool IsKeyDownFor(ControlType control, int millis);
	void Update();
	static const int controlSize = SIZEOF_ControlType;
	int ControlKeys[controlSize];
private:
	bool controlCurr[controlSize];
	bool controlPrev[controlSize];

	unsigned long long pressTime[controlSize];
	unsigned long long releaseTime[controlSize];

};

