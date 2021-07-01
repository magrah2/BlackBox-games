#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "library/BlackBox_Manager.hpp"
#include "library/BlackBox_timers.hpp"
#include <chrono>
#include <iostream>

using namespace BlackBox;
using namespace std::literals::chrono_literals;
using namespace std;


void game1();
void game2();
void game3();
void game4();

void error();

void menu();
void charging();

extern "C" {
void app_main() {

    auto& manager = Manager::singleton();

    auto& timers = Timers::get();

    manager.init();

    auto& power = manager.power();

    auto batteryCheck = timers.schedule(60000, [&]() {
        Manager::singleton().power().checkBatteryLevel(3700, true);
        return true;
    });



    while (true) {
        if (power.usbConnected()) {
            charging();
        }

        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}
}

void game4() {
    auto& power = Manager::singleton().power();

    if (power.usbConnected()) {
        charging();
    }
}

void game3() {
    auto& power = Manager::singleton().power();

    if (power.usbConnected()) {
        charging();
    }
}

void game2() {
    auto& power = Manager::singleton().power();

    if (power.usbConnected()) {
        charging();
    }
}

void game1() {
    auto& power = Manager::singleton().power();

    if (power.usbConnected()) {
        charging();
    }
}

void menu() {
    auto& manager = Manager::singleton();

    auto& doors = manager.doors();
    for (auto &i : doors)
        i.open();

    unsigned read = 0;
    while (!read) {
        for(int i = 0; i < 4; i++)
            read += (!doors[i].tamperCheck()) << i;
    }

    switch (read) {
    case 1:
        game1();
        break;
    case 2:
        game2();
        break;
    case 4:
        game3();
        break;
    case 8:
        game4();
        break;
    default:
        error();
        menu();
        break;
    }
}

void charging() {
    auto& power = Manager::singleton().power();
    auto start = chrono::steady_clock::now();
    while (power.usbConnected())
        vTaskDelay(100 / portTICK_PERIOD_MS);
    if ((chrono::steady_clock::now() - start) <= 10s)
        power.turnOff();
    else
        menu();
}