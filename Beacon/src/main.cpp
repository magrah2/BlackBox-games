#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "library/BlackBox_Manager.hpp"
#include "library/BlackBox_timers.hpp"
#include "helpFunctions.hpp"
#include <chrono>
#include <iostream>

using namespace BlackBox;
using namespace std::literals::chrono_literals;
using namespace std;

void menu();

extern "C" {
void app_main() {

    auto& manager = Manager::singleton();

    auto& timers = Timers::get();

    manager.init();

    auto& power = manager.power();
    power.turnOn();
    power.turnOn5V();
    power.turnOnLDC();

    auto batteryCheck = timers.schedule(60000, [&]() {
        Manager::singleton().power().checkBatteryLevel(3700, true);
        return true;
    });

    auto& ring = manager.ring();

    ring.setDarkModeValue(50);
    ring.enableDarkMode();
    ring.rotate(8);

    showPowerOn();

    while (true) {
        if (power.usbConnected()) {
            charging();
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}
}