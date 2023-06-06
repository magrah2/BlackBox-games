#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "i2s.hpp"
#include "library/BlackBox_Manager.hpp"
#include "library/BlackBox_Ring.hpp"
#include "library/BlackBox_timers.hpp"
#include <chrono>
#include <iostream>

using namespace std::literals::chrono_literals;

bool readButton(BlackBox::Manager& manager) {
    auto& ldc = manager.ldc();
    static int pressureNow;
    static int pressureLast;
    static bool firstRead = true;
    bool pressNow;

    if (firstRead) {
        ldc.syncChannels();
        pressureLast = ldc[0] + ldc[1] + ldc[2] + ldc[3];
        firstRead = false;
    }

    ldc.syncChannels();

    pressureNow = ldc[0] + ldc[1] + ldc[2] + ldc[3];

    int diff = pressureNow - pressureLast;
    if (diff < 0) {
        diff = 0;
    }

    if (diff >= 600000) {
        pressNow = true;
    } else {
        pressNow = false;
    }
    pressureLast = pressureNow;

    return pressNow;
}

uint32_t get_i2s_val(i2s_port_t i2s_num) {
    constexpr std::size_t n = 1;
    std::size_t read = 0;
    uint32_t bytes[n];
    i2s_read(i2s_num, bytes, sizeof(uint32_t) * n, &read, 100000 / portTICK_PERIOD_MS);
    bytes[0] >>= (32 - 18);
    return bytes[0];
}

struct sound_accum {

    void push(int32_t v) {
        sum += v;
    }

    bool is_triggered() {
        return sum > trigger_val;
    }

    void tick() {
        sum = std::clamp(sum - decay, 0, max_sum);
    }

    int32_t sum = 0;

    int32_t decay = 256;
    int32_t trigger_val = 6000;
    int32_t max_sum = 256000;
};

using tp = std::chrono::steady_clock::time_point;

struct door_handler {

    bool silent_time() {
        return opened_at + 4s > std::chrono::steady_clock::now();
    }

    void tick(bool is_triggered) {
        if (silent_time()) {
            return;
        }
        if (is_triggered) {
            door.close();
            opened = false;
        } else {
            if (!opened) {
                door.open();
                opened_at = std::chrono::steady_clock::now();
                opened = true;
            }
        }
    }

    bool opened = false;
    tp opened_at;
    BlackBox::Door& door;
};

extern "C" {
void app_main() {
    auto& manager = BlackBox::Manager::singleton();

    auto& timers = BlackBox::Timers::get();

    manager.init();

    auto& power = manager.power();
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
    auto top = manager.beacon().top();
    BlackBox::Door& door = manager.door(0);

    int32_t last = get_i2s_val(i2s_num);

    door_handler dh { false, std::chrono::steady_clock::now(), door };
    sound_accum acc;
    while (true) {
        int32_t n = get_i2s_val(i2s_num);
        int32_t v = n - last;
        last = n;

        int v2 = abs(v);
        v2 = std::min(v2, 3200);

        acc.push(v2);

        int m = acc.sum / 64;

        std::cout << acc.sum << "\t" << m << "\t" << v2 << "\t" << acc.is_triggered() << "\t" << dh.opened_at.time_since_epoch().count() << std::endl;

        for (unsigned i = 0; i < perim.count(); i++) {
            perim[i] = Rgb(acc.is_triggered() ? 255 : 0, acc.is_triggered() ? 0 : std::min(m, 255),
                dh.silent_time() ? 255 : 0);
        }
        int top_n = top.count() * acc.sum / acc.max_sum;
        for (unsigned i = 0; i < top.count(); i++) {
            bool fired = acc.is_triggered() && i < top_n;
            top[i] = Rgb(fired ? 255 : 0, 0, 0);
        }

        manager.beacon().show(16);
        acc.tick();
        dh.tick(acc.is_triggered());

        if (!power.usbConnected()) {
            door.close();
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            power.turnOff();
        }

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
}
