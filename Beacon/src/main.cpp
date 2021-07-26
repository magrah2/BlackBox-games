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
    // power.turnOff();
    power.turnOn();
    power.turnOn5V();
    power.turnOnLDC();

    // Manager::singleton().beacon().top().fill(cBlue);
    // vTaskDelay(10000 / portTICK_PERIOD_MS);
    // while(true){
    //     int out = readButtons();
    //     if(out != -1) {
    //         cout << out << endl;
    //     }
    //     vTaskDelay(100 / portTICK_PERIOD_MS);
    // }

    auto batteryCheck = timers.schedule(60000, [&]() {
        if(!Manager::singleton().power().checkBatteryLevel(3700, true)){
            showEmptyBattery();
            vTaskDelay(10 / portTICK_PERIOD_MS);
            power.turnOn();
        }
        return true;
    });

    auto& beacon = manager.beacon();

    showPowerOn();

    while (true) {
        if (power.usbConnected()) {
            switching_play_charge();
            cout << power.batteryVoltage(true) << "V *** " << power.batteryPercentage(false) << "%" << endl;
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}
}