#pragma once

#include "library/BlackBox_Manager.hpp"

#include <functional>

using namespace BlackBox;
void powerOff(); // R B

void lightHouse(); // R R
void multiLightHouse(); // R G
void multiTreasure(); // R B

void ghosts(); // G R
void kingOfTheHill(); // G G
void fillMeInNoTime(); // G B

void mine(); // B R
void trains(); // B G

extern std::vector<std::function<void()>> games;
