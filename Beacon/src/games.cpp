#include "games.hpp"

#include "helpFunctions.hpp"

void game0() {
    static uint8_t gameNum = 0;
    auto& power = Manager::singleton().power();
    showColor(gameColors[gameNum]);

    infinityLoop();

    if (power.usbConnected()) {
        charging();
    }
}

void game1() {
    static uint8_t gameNum = 1;
    auto& power = Manager::singleton().power();
    showColor(gameColors[gameNum]);

    infinityLoop();

    if (power.usbConnected()) {
        charging();
    }
}

void game2() {
    static uint8_t gameNum = 2;
    auto& power = Manager::singleton().power();
    showColor(gameColors[gameNum]);

    infinityLoop();

    if (power.usbConnected()) {
        charging();
    }
}

void game3() {
    static uint8_t gameNum = 3;
    auto& power = Manager::singleton().power();
    showColor(gameColors[gameNum]);

    infinityLoop();

    if (power.usbConnected()) {
        charging();
    }
}