#include "games.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <chrono>
#include <cstdint>
#include <vector>

#include "helpFunctions.hpp"

using namespace std::literals::chrono_literals;
void gameEnd() {
    closeAllDoors();
    clearAll();
    switching_play_charge();
}
//red game
void game0() {
    static uint8_t gameNum = 0;
    auto& manager = Manager::singleton();
    auto& power = manager.power();
    auto& beacon = manager.beacon();
    showColorPerim(gameColors[gameNum]);
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    // infinityLoop();
    closeAllDoors();
    uint8_t state = 0;
    uint32_t openStart;

    while (true) {
#ifndef BB_DEBUG
        if (power.usbConnected()) {
            switching_play_charge();
        }
#endif

        switch (state) {
        case 0:
            showBeacon();
            if (readButton()) {
                state = 1;
                openStart = esp_timer_get_time() / 1000LL;
                openAllDoors();
                showColorTop(cRed);
            }
            break;
        case 1:
            showBeacon();
            uint32_t openNow = esp_timer_get_time() / 1000LL;

            if (openNow - openStart > 30000) {
                state = 0;
                closeAllDoors();
                showColorTop(cBlack);
            }
            break;
        }
        vTaskDelay(30 / portTICK_PERIOD_MS);
    }
}
// green game
void game1() {
    static uint8_t gameNum = 1;
    auto& manager = Manager::singleton();
    auto& power = manager.power();
    auto& doors = manager.doors();
    bool openDoors = false;
    showColorPerim(gameColors[gameNum]);
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    // infinityLoop();

    closeAllDoors();

    auto activeColor = esp_random() % 4;
    auto colorStart = std::chrono::steady_clock::now();
    auto openStart = std::chrono::steady_clock::now();

    while (true) {
#ifndef BB_DEBUG
        if (power.usbConnected()) {
            switching_play_charge();
        }
#endif

        showBeacon(gameColors[activeColor]);

        if (readButton()) {
            openDoors = true;
            showColorTop(gameColors[activeColor]);
            doors[activeColor].open();
            openStart = std::chrono::steady_clock::now();
        }

        if (((std::chrono::steady_clock::now() - openStart) >= 20s) && openDoors) {
            closeAllDoors();
            showColorTop(cBlack);
            openDoors = false;
        }

        if ((std::chrono::steady_clock::now() - colorStart) >= 2min) {
            activeColor++;
            activeColor %= 4;
            colorStart = std::chrono::steady_clock::now();
        }

        vTaskDelay(30 / portTICK_PERIOD_MS);
    }
}
//yellow game
void game2() {
    static uint8_t gameNum = 2;
    auto& power = Manager::singleton().power();
    showColorPerim(gameColors[gameNum]);

    for (int i = 0; i < 4; i++)
        Manager::singleton().doors()[i].close();

    vTaskDelay(1000 / portTICK_PERIOD_MS);
    showPowerOff();
    power.turnOff();

    // infinityLoop();

    // if (power.usbConnected()) {
    //     switching_play_charge();
    // }
}

void inputing(int button) {
    static std::vector<int> input;
    auto lastPressTime = std::chrono::steady_clock::now();

    constexpr int combinations[4][10] = {
        { Y, G, B, Y, R, G, B, B, R, Y },
        { B, B, G, G, R, Y, Y, G, B, R },
        { R, Y, Y, B, G, R, Y, B, Y, G },
        { Y, B, G, Y, R, B, Y, R, G, Y }
    };

    auto start = std::chrono::steady_clock::now();
    input.clear();

    do {
        if (lastPressTime + 5s < std::chrono::steady_clock::now()) {
            input.clear();
            beacon.top().fill(cError);
            beacon.show(g_lightIntensity);
            vTaskDelay(400 / portTICK_PERIOD_MS);
            beacon.top().clear();
            beacon.show(g_lightIntensity);
            vTaskDelay(400 / portTICK_PERIOD_MS);
            return;
        }
        if (button != -1) {
            input.push_back(button);
            cout << button << endl;
            beacon.top().fill(gameColors[button]);
            beacon.show(g_lightIntensity);
            vTaskDelay(300 / portTICK_PERIOD_MS);
            beacon.top().clear();
            beacon.show(g_lightIntensity);
            lastPressTime = std::chrono::steady_clock::now();
        }
        // if ((std::chrono::steady_clock::now() - start) >= 1min) {
        //     showPowerOff();
        //     input.clear();
        //     return;
        // }
        button = readButtons();
        // vTaskDelay(400 / portTICK_PERIOD_MS);
    } while (input.size() < 10);

    for (int i = 0; i < 4; i++) {
        int result = 1;
        for (int j = 0; j < 10; j++) {
            result *= (input[j] == combinations[i][j]);
        }
        if (result) {
            doors[i].open();
            showColorTop(gameColors[i]);
            vTaskDelay(20000 / portTICK_PERIOD_MS);
            closeAllDoors();
            return;
        }
    }
}

extern void updateAverage(Coords);
//blue game
void game3() {
    Coords out = manager.touchpad().calculate(); // Calculate coordinates of touch

    updateAverage(out);

    clearAll();
    showGameColors();
    showColorTop(gameColors[3]);
    closeAllDoors();

    while (true) {
        int button = -1;
        showColorTop(cWhite);
        button = readButtons();
        cout << "Button:" << button << endl;
        if (button != -1) {
            manager.beacon().top().clear();
            manager.beacon().show(g_lightIntensity);
            vTaskDelay(200 / portTICK_PERIOD_MS);
            inputing(button);
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);

#ifndef BB_DEBUG
        if (power.usbConnected()) {
            switching_play_charge();
        }
#endif
    }
}

#include "library/BlackBox_timers.hpp"
#include <bitset>

void game4() {
    auto& timers = Timers::get();
    closeAllDoors();
    std::bitset<4> activated;

    while (true) {
        activated = 0;

        auto restStart = std::chrono::steady_clock::now();
        beacon.fill(cBlack);
        beacon.show(g_lightIntensity);

        while (restStart + 5min > std::chrono::steady_clock::now()) {
#ifndef BB_DEBUG
            if (power.usbConnected()) {
                switching_play_charge();
            }
#endif
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }

        auto start = std::chrono::steady_clock::now();

        while (!activated.all() && (start + 60s > std::chrono::steady_clock::now())) {
            int button = readButtons();
            if (button != -1)
                activated.set(button);
            for (int i = 0; i < 4; i++)
                beacon.top().drawArc(activated[i] ? cBlack : gameColors[i], i * 15, 15 + i * 15, ArcType::Clockwise);
            beacon.perimeter().fill(cWhite);
            beacon.perimeter().drawArc(cBlack, beacon.perimeter().count() - 1, std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - start).count() * beacon.perimeter().count() / 60, ArcType::CounterClockwise);
            std::cout << std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - start).count() * beacon.perimeter().count() / 60 << std::endl;
            beacon.show(g_lightIntensity);

#ifndef BB_DEBUG
            if (power.usbConnected()) {
                switching_play_charge();
            }
#endif
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
        if (start + 60s > std::chrono::steady_clock::now()) {
            std::cout << "open" << std::endl;
            doors[esp_random() % 4].open();

            auto openDoors = std::chrono::steady_clock::now();
            for (int i = 0; i < beacon.perimeter().count(); i++) {
                beacon.perimeter().fill(cRed);
                beacon.perimeter().drawArc(cBlack, beacon.perimeter().count() - 1, i, ArcType::CounterClockwise);
                beacon.show(g_lightIntensity);
#ifndef BB_DEBUG
                if (power.usbConnected()) {
                    switching_play_charge();
                }
#endif
                vTaskDelay(500 / portTICK_PERIOD_MS);
            }
            closeAllDoors();
        } else
            std::cout << "timeout" << std::endl;
    }
}
