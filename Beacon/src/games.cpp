#include "games.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <cstdint>
#include <chrono>
#include <vector>

#include "helpFunctions.hpp"

using namespace std::literals::chrono_literals;

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

    while (true)
    {
        if (power.usbConnected()) {
            charging();
        }

        switch (state)
        {
        case 0:
            showBeacon();
            if(readButton()) {
                state = 1;
                openStart = esp_timer_get_time() / 1000LL;
                openAllDoors();
                showColorTop(cRed);
            }
            break;
        case 1:
            showBeacon();
            uint32_t openNow = esp_timer_get_time() / 1000LL;

            if(openNow - openStart > 30000) {
                state = 0;
                closeAllDoors();
                showColorTop(cBlack);
            }
            break;
        }
        vTaskDelay(30 / portTICK_PERIOD_MS);
    }
}

void game1() {
    static uint8_t gameNum = 1;
    auto& manager = Manager::singleton();
    auto& power = manager.power();
    auto& doors = manager.doors();
    showColorPerim(gameColors[gameNum]);
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    // infinityLoop();

    closeAllDoors();

    auto activeColor = esp_random() % 4;
    auto colorStart = std::chrono::steady_clock::now();
    auto openStart = std::chrono::steady_clock::now();

    while(true) {
        if (power.usbConnected()) {
            charging();
        }

        showBeacon(gameColors[activeColor]);

        if (readButton()) {
            showColorTop(gameColors[activeColor]);
            doors[activeColor].open();
            openStart = std::chrono::steady_clock::now();
        }

        if ((std::chrono::steady_clock::now() - openStart) >= 20s) {
            closeAllDoors();
            showColorTop(cBlack);
        }

        if ((std::chrono::steady_clock::now() - colorStart) >= 2min) {
            activeColor++;
            activeColor %= 4;
            colorStart = std::chrono::steady_clock::now();
        }

        vTaskDelay(30 / portTICK_PERIOD_MS);

    }

}

void game2() {
    static uint8_t gameNum = 2;
    auto& power = Manager::singleton().power();
    showColorPerim(gameColors[gameNum]);

    for (int i = 0; i < 4; i++)
        Manager::singleton().doors()[i].close();
    
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    power.turnOff();

    // infinityLoop();

    // if (power.usbConnected()) {
    //     charging();
    // }
}

void inputing(int button) {
    auto& doors = Manager::singleton().doors();
    auto& beacon = Manager::singleton().beacon();

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
        if(lastPressTime + 5s < std::chrono::steady_clock::now()){
            input.clear();
            beacon.top().fill(cError);
            beacon.show(25);
            vTaskDelay(100 / portTICK_PERIOD_MS);
            beacon.top().clear();
            beacon.show(25);
            return;
        }
        if (button != -1){
            input.push_back(button);
            lastPressTime = std::chrono::steady_clock::now();
        }
        // if ((std::chrono::steady_clock::now() - start) >= 1min) {
        //     showPowerOff();
        //     input.clear();
        //     return;
        // }
        button = readButtons();
        vTaskDelay(200 / portTICK_PERIOD_MS);
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

void game3() {
    auto& manager
        = Manager::singleton();
    auto& power = manager.power();
    auto& doors = Manager::singleton().doors();
    clearAll();
    showGameColors();
    showColorTop(gameColors[3]);
    closeAllDoors();

    while (true) {
        int button = -1;
        button = readButtons();
        cout << "Button:" << button << endl;
        showColorTop(gameColors[3]);
        if (button != -1) {
            manager.beacon().top().clear();
            inputing(button);
        }

        if (power.usbConnected()) {
            charging();
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}