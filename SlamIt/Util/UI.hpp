#pragma once
#include <inc/natives.h>
#include <string>
#include <vector>

namespace UI {
    void Notify(const std::string& message);
    void Notify(const std::string& message, bool removePrevious);

    void ShowText(float x, float y, float scale, const std::string& text);
    void ShowText(float x, float y, float scale, const std::string& text, int font, bool outline);

    std::string GetKeyboardResult();

    void DrawSphere(Vector3 p, float scale, int r, int g, int b, int a);
    void DrawLine(Vector3 pA, Vector3 pB, int r, int g, int b, int a);
    void ShowText3D(Vector3 location, float baseSize, const std::vector<std::string>& textLines);
    void ShowHelpText(const std::string& message);
}
