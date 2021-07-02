# pragma once

#include "library/BlackBox_Manager.hpp"
#include "games.hpp"
#include <chrono>
#include <iostream>

using namespace BlackBox;
using namespace std::literals::chrono_literals;
using namespace std;

static Rgb cRed = Rgb(255,0,0);
static Rgb cGreen = Rgb(0, 255, 0);
static Rgb cYellow = Rgb(255, 110, 0);
static Rgb cBlue = Rgb(0, 0, 255);
static Rgb cWhite = Rgb(255, 255, 255);
static Rgb cBlack = Rgb(0, 0, 0);
static Rgb cError = Rgb(225, 20, 220);

static std::array<Rgb, 6> gameColors = {cRed, cGreen ,cYellow, cBlue, cWhite, cError};

void menu();

void showColor(Rgb rgb);

void showGameColors();

void showError();

void showCharging();
void showPowerOn();
void showPowerOff();

void infinityLoop();

void charging();