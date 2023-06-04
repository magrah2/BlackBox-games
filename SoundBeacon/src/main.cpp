#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "i2s.hpp"
#include "library/BlackBox_Manager.hpp"
#include "library/BlackBox_Ring.hpp"
#include "library/BlackBox_timers.hpp"
#include <chrono>
#include <iostream>

using namespace std::literals::chrono_literals;

extern "C" {
void app_main() {
    auto& manager = BlackBox::Manager::singleton();

    auto& timers = BlackBox::Timers::get();

    manager.init();

    auto& power = manager.power();
    // power.turnOff();
    power.turnOn();
    power.turnOn5V();
    power.turnOnLDC();

    std::ignore = timers.schedule(60000, [&]() {
        if (!power.checkBatteryLevel(3700, !power.usbConnected())) {
            power.turnOff();
        }
        return true;
    });

    const i2s_port_t i2s_num = static_cast<i2s_port_t>(0);

    setup_i2s(i2s_num);

    // oh ocme on, why is Beacon<>::Perimeter _private_?
    auto perim = manager.beacon().perimeter();
    auto side = manager.beacon().top();

    for (unsigned i = 0; i < side.count(); i++) {
        side[i] = Rgb(0, 0, 0);
    }

    float t = 0.f;
    while (true) {
        constexpr std::size_t n = 128;
        std::size_t read = 0;
        uint8_t bytes[n];
        i2s_read(i2s_num, bytes, n, &read, 100000 / portTICK_PERIOD_MS);

        std::cout << int(read) << ":";
        for (uint8_t v : bytes) {
            std::cout << int(v) << ",";
        }
        std::cout << std::endl;

        for (unsigned i = 0; i < perim.count(); i++) {
            perim[i] = Rgb(255, 191, 0);
        }

        int ival = 64 * std::sin(t) + 128;
        manager.beacon().show(ival);

        t += 0.01f;
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
}
