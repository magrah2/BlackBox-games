#include "games.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <cstdint>
#include <chrono>
#include <vector>

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
    closeAllDors();
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
                openAllDors();
                showColorTop(cRed);
            }
            break;
        case 1:
            showBeacon();
            uint32_t openNow = esp_timer_get_time() / 1000LL;

            if(openNow - openStart > 30000) {
                state = 0;
                closeAllDors();
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

    closeAllDors();

    int activeCollor = abs((esp_random()/4294967296.0)*4);
    int collorStart = esp_timer_get_time()/1000;
    int openStart = esp_timer_get_time()/1000;

    while(true) {
        if (power.usbConnected()) {
            charging();
        }


        showBeacon(gameColors[activeCollor]);
        if(readButton()){
            showColorTop(gameColors[activeCollor]);
            doors[activeCollor].open();
            openStart = esp_timer_get_time()/1000;
        }
        if((esp_timer_get_time()/1000) - openStart >= 20000)
        {
            closeAllDors();
            showColorTop(cBlack);
        }   

        if((esp_timer_get_time()/1000) - collorStart > 12000000)
        {
            activeCollor += 1;
            if(activeCollor >= 4)
            {
                activeCollor = 0;
            }
            collorStart = esp_timer_get_time()/1000;
        }
        vTaskDelay(30 / portTICK_PERIOD_MS);

    }

}

void game2() {
    static uint8_t gameNum = 2;
    auto& power = Manager::singleton().power();
    showColorPerim(gameColors[gameNum]);

    infinityLoop();

    if (power.usbConnected()) {
        charging();
    }
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
        vTaskDelay(100 / portTICK_PERIOD_MS);
    } while (input.size() < 10);

    for (int i = 0; i < 4; i++) {
        int result = 1;
        for (int j = 0; j < 10; j++) {
            result *= (input[j] == combinations[i][j]);
        }
        if (result) {
            doors[i].open();
            showColorTop(gameColors[i]);
            vTaskDelay(30000 / portTICK_PERIOD_MS);
            closeAllDors();
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
    closeAllDors();

    while (true) {
        int button = -1;
        button = readButtons();
        cout << "Button:" << button << endl;
        if (button != -1) {
            inputing(button);
        }

        if (power.usbConnected()) {
            charging();
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}