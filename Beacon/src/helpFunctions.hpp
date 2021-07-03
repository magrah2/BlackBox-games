# pragma once

#include "library/BlackBox_Manager.hpp"
#include "games.hpp"
#include <chrono>
#include <iostream>

using namespace BlackBox;
using namespace std::literals::chrono_literals;
using namespace std;

const int darkMode = 50;

static const Rgb cRed = {255,0,0};
static const Rgb cGreen = {0, 255, 0};
static const Rgb cYellow = {255, 110, 0};
static const Rgb cBlue = {0, 0, 255};
static const Rgb cWhite = {255, 255, 255};
static const Rgb cBlack = {0, 0, 0};
static const Rgb cError = {225, 20, 220};

static const std::array<Rgb, 6> gameColors = {cRed, cGreen ,cYellow, cBlue, cWhite, cError};

enum Color {
    R,
    G,
    Y,
    B,
    W,
    Error,
};

void menu();

void showColorPerim(Rgb rgb);
void showColorTop(Rgb rgb);
void clearAll();

void showGameColors();

void showError();
void showEmptyBattery();

void showCharging();
void showPowerOn();
void showPowerOff();
void showLowVoltage();

void infinityLoop();

void charging();

void openAllDors();
void closeAllDors();

void showBeacon(Rgb rgb = Rgb(255, 255, 255));

bool readButton();

int calculateButton(int pressureNow[4], int pressureLast[4]);
int readButtons();

int readBattery();