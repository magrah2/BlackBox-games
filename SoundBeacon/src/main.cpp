#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "i2s.hpp"
#include "library/BlackBox_Manager.hpp"
#include "library/BlackBox_Ring.hpp"
#include "library/BlackBox_timers.hpp"
#include <chrono>
#include <iostream>

using namespace std::literals::chrono_literals;

uint32_t get_i2s_val(i2s_port_t i2s_num) {
    constexpr std::size_t n = 1;
    std::size_t read = 0;
    uint32_t bytes[n];
    i2s_read(i2s_num, bytes, sizeof(uint32_t) * n, &read, 100000 / portTICK_PERIOD_MS);
    bytes[0] >>= (32 - 18);
    return bytes[0];
}

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

    int32_t last = get_i2s_val(i2s_num);

    int sum = 0;
    while (true) {
        int32_t n = get_i2s_val(i2s_num);
        int32_t v = n - last;
        last = n;

        //std::cout << v << std::endl;

        int v2 = abs(v);
        if (v2 < 16) {
            v2 = 0;
        }
        v2 = std::min(v2, 3200);
        sum += v2;

        int m = sum / 64;

        std::cout << sum << "\t" << m << "\t" << v2 << std::endl;

        sum = std::clamp(sum - 128, 0, 32000);

        for (unsigned i = 0; i < perim.count(); i++) {
            perim[i] = Rgb(sum > 2000 ? 255 : 0, std::min(m, 255), 0);
        }

        manager.beacon().show(255);

        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
}
}
