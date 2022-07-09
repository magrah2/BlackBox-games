#pragma once

#include "games.hpp"
#include "library/BlackBox_Manager.hpp"
#include <chrono>
#include <iostream>

#ifdef BB_DEBUG
#define CHECK_USB() (void)0
#else
#define CHECK_USB()                                            \
    if (BlackBox::Manager::singleton().power().usbConnected()) \
        switching_play_charge()
#endif // BB_DEBUG

using namespace BlackBox;
using namespace std::literals::chrono_literals;
using namespace std;

const int darkMode = 50;

static const Rgb cRed = { 255, 0, 0 };
static const Rgb cGreen = { 0, 255, 0 };
static const Rgb cBlue = { 0, 0, 255 };
static const Rgb cYellow = { 150, 110, 0 };

static const Rgb cOrange = { 255, 50, 0 };
static const Rgb cPink = { 252, 0, 165 };
static const Rgb cTeal = { 0, 255, 160 };

static const Rgb cWhite = { 255, 255, 255 };
static const Rgb cBlack = { 0, 0, 0 };
static const Rgb cError = { 225, 20, 220 };


static const std::array gameColors = { cRed, cGreen, cBlue, cYellow, cOrange, cPink, cTeal, cWhite, cError };

enum Color {
    R, // Red
    G, // Green
    B, // Blue
    Y, // Yellow
    O, // Orange
    P, // Pink
    T, // Teal
    W, // White
    Error, 
};

void menu();

extern int g_lightIntensity;
int lightIntensity(Rgb rgb = cGreen);

void showColorPerim(Rgb rgb);
void showColorTop(Rgb rgb);
void clearAll();

void showGameColors();
void showGameColors2();

void showError();
void showEmptyBattery();

void showCharging();
void showPowerOn();
void showPowerOff();
void showLowVoltage();

void infinityLoop();

void switching_play_charge();

void openAllDoors();
void closeAllDoors();

void showBeacon(Rgb rgb = Rgb(255, 255, 255));

bool readButton();

int calculateButton(int pressureNow[4], int pressureLast[4]);
int readButtons();

int readBattery();

inline auto& manager = Manager::singleton();
inline auto& doors = manager.doors();
inline auto& power = manager.power();
inline auto& beacon = manager.beacon();
