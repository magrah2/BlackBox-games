#include "games.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <chrono>
#include <cstdint>
#include <vector>

#include "helpFunctions.hpp"

using namespace std::literals::chrono_literals;
using namespace std::chrono;

std::vector<std::function<void()>> games = { lightHouse, multiLightHouse, multiTreasure, ghosts, kingOfTheHill, fillMeInNoTime, mine, trains };

void gameEnd() {
    closeAllDoors();
    clearAll();
    switching_play_charge();
}
//red game
void lightHouse() {
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
void multiLightHouse() {
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
    auto colorStart = steady_clock::now();
    auto openStart = steady_clock::now();

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
            openStart = steady_clock::now();
        }

        if (((steady_clock::now() - openStart) >= 20s) && openDoors) {
            closeAllDoors();
            showColorTop(cBlack);
            openDoors = false;
        }

        if ((steady_clock::now() - colorStart) >= 2min) {
            activeColor++;
            activeColor %= 4;
            colorStart = steady_clock::now();
        }

        vTaskDelay(30 / portTICK_PERIOD_MS);
    }
}

void powerOff() {
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
    auto lastPressTime = steady_clock::now();

    constexpr int combinations[4][10] = {
        { Y, G, B, Y, R, G, B, B, R, Y },
        { B, B, G, G, R, Y, Y, G, B, R },
        { R, Y, Y, B, G, R, Y, B, Y, G },
        { Y, B, G, Y, R, B, Y, R, G, Y }
    };

    auto start = steady_clock::now();
    input.clear();

    do {
        if (lastPressTime + 5s < steady_clock::now()) {
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
            lastPressTime = steady_clock::now();
        }
        // if ((steady_clock::now() - start) >= 1min) {
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
void multiTreasure() {
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

        CHECK_USB();
    }
}

#include "library/BlackBox_timers.hpp"
#include <bitset>

void ghosts() {
    auto& timers = Timers::get();
    closeAllDoors();
    std::bitset<4> activated;

    while (true) {
        activated = 0;

        auto restStart = steady_clock::now();
        beacon.fill(cBlack);
        beacon.show(g_lightIntensity);

        while (restStart + 5min > steady_clock::now()) {
            CHECK_USB();
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }

        auto start = steady_clock::now();

        while (!activated.all() && (start + 60s > steady_clock::now())) {
            int button = readButtons();
            if (button != -1)
                activated.set(button);
            for (int i = 0; i < 4; i++)
                beacon.top().drawArc(activated[i] ? cBlack : gameColors[i], i * 15, 15 + i * 15, ArcType::Clockwise);
            beacon.perimeter().fill(cWhite);
            beacon.perimeter().drawArc(cBlack, beacon.perimeter().count() - 1, duration_cast<seconds>(steady_clock::now() - start).count() * beacon.perimeter().count() / 60, ArcType::CounterClockwise);
            std::cout << duration_cast<seconds>(steady_clock::now() - start).count() * beacon.perimeter().count() / 60 << std::endl;
            beacon.show(g_lightIntensity);

            CHECK_USB();
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
        if (start + 60s > steady_clock::now()) {
            std::cout << "open" << std::endl;
            doors[esp_random() % 4].open();

            auto openDoors = steady_clock::now();
            for (int i = 0; i < beacon.perimeter().count(); i++) {
                beacon.perimeter().fill(cRed);
                beacon.perimeter().drawArc(cBlack, beacon.perimeter().count() - 1, i, ArcType::CounterClockwise);
                beacon.show(g_lightIntensity);
                CHECK_USB();
                vTaskDelay(500 / portTICK_PERIOD_MS);
            }
            closeAllDoors();
        } else
            std::cout << "timeout" << std::endl;
    }
}

void showState(std::array<unsigned, 4>& states) {
    uint64_t sum = 0;
    for (int i = 0; i < states.size(); i++) {
        sum += states[i];
    }

    unsigned index = 0;
    beacon.top().clear();
    for (int i = 0; i < 4; i++) {
        beacon.top().drawArc(gameColors[i], index, index + (states[i] * 60) / sum, ArcType::Clockwise);
        index += (states[i] * 60) / sum;
    }
    beacon.show(g_lightIntensity);
}

void kingOfTheHill() {
    std::array<unsigned, 4> states = { 15, 15, 15, 15 };
    int activeColor = -1;
    showGameColors();
    vTaskDelay(500 / portTICK_PERIOD_MS);

    while (true) {
        if (activeColor == -1) {
            showState(states);
            while (activeColor == -1) {
                for (int i = 0; i < 4; i++) {
                    if (doors[i].tamperCheck()) {
                        activeColor = i;
                        break;
                    }
                }
                CHECK_USB();
                vTaskDelay(10 / portTICK_PERIOD_MS);
            }
            doors[activeColor].close();
            showColorTop(gameColors[activeColor]);
            vTaskDelay(100 / portTICK_PERIOD_MS);
            showColorTop(cBlack);
        }

        int counter = 0;
        while (!readButton()) {
            if (counter == 10) {
                states[activeColor]++;
                counter = 0;
            }
            counter++;
            vTaskDelay(100 / portTICK_PERIOD_MS);
            showState(states);
            CHECK_USB();
        }

        openAllDoors();
        vTaskDelay(500 / portTICK_PERIOD_MS);
        activeColor = -1;

        CHECK_USB();
    }
}

void fillMeInNoTime() {
    int state = 0;

    unsigned count = 59;

    auto startTime = steady_clock::now();

    enum {
        Init = 0,
        Open,
        Closed,
        LockDown,
        LockdownPressed,
    };

    std::bitset<4> votes = 0;

    while (true) {
        CHECK_USB();

        if (state == Init) {
            showColorTop(cGreen);
            showGameColors();
            openAllDoors();
            vTaskDelay(300 / portTICK_PERIOD_MS);
            count = 60;
            state = 1;
        }

        if (state == Open) {
            unsigned read = 0;
            for (auto& door : doors)
                read += door.tamperCheck();

            if (read == 4) {
                startTime = steady_clock::now();
                closeAllDoors();
                state = 2;
            }
        }

        if (state == Closed) {
            if (steady_clock::now() - startTime >= 3s) {
                count--;
                startTime = steady_clock::now();
                beacon.top().clear();
                beacon.top().drawArc(cGreen, 0, count, BlackBox::ArcType::Clockwise);
                beacon.show(g_lightIntensity);
            }

            if (readButton()) {
                openAllDoors();
                vTaskDelay(300 / portTICK_PERIOD_MS);
                state = 1;
            }

            if (count == 0) {
                state = LockDown;
                closeAllDoors();
                for (int i = 0; i < 2; i++) {
                    showColorPerim(cRed);
                    vTaskDelay(200 / portTICK_PERIOD_MS);
                    showColorPerim(cBlack);
                    vTaskDelay(200 / portTICK_PERIOD_MS);
                }
                showGameColors();
                votes.reset();
            }
        }

        if (state == LockDown) {
            int btn;
            if ((btn = readButtons()) != -1) {
                votes.set(btn);
                vTaskDelay(500 / portTICK_PERIOD_MS);
            }

            Rgb color;
            for (int i = 0; i < 4; i++) {
                (color = gameColors[i]).stretchChannelsEvenly(votes[i] ? 255 : 75);
                beacon.top().drawArc(color, i * 15, 15 + i * 15, BlackBox::ArcType::Clockwise);
            };
            beacon.show(g_lightIntensity);

            if (votes.all())
                state = Init;
        }

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void mine() {
    enum {
        Init = 0,
        Closed,
        Open,
        Upgrade,
        Confirmation
    };

    int state = Init;

    auto startTime = steady_clock::now();

    int btn = -1;

    std::array<int, 4> levels = { 1, 1, 1, 1 };
    while (true) {
        CHECK_USB();

        if (state == Init) {
            for (size_t i = 0; i < 4; i++)
                beacon.top().drawArc(gameColors[i], i * 15, i * 15 + 15, BlackBox::ArcType::Clockwise);

            closeAllDoors();
            showColorPerim(cBlack);
            vTaskDelay(300 / portTICK_PERIOD_MS);
            state = Closed;
        }

        if (state == Closed) {
            btn = readButtons();

            showBeacon();

            if (btn != -1) {
                for (size_t i = 0; i < levels[btn]; i++)
                    doors[i].open();
                vTaskDelay(300 / portTICK_PERIOD_MS);
                startTime = steady_clock::now();
                state = Open;
            }
        }

        if (state == Open) {
            int read = 0;
            for (auto& door : doors)
                read += door.readTamperCheckButton();

            if (read == 0) {
                closeAllDoors();
                showColorPerim(gameColors[btn]);
                showColorTop(gameColors[btn]);
                vTaskDelay(300 / portTICK_PERIOD_MS);
                startTime = steady_clock::now();
                state = Upgrade;

            } else if (steady_clock::now() - startTime >= 5s) {
                closeAllDoors();
                for (size_t i = 0; i < 4; i++)
                    beacon.top().drawArc(gameColors[i], i * 15, i * 15 + 15, BlackBox::ArcType::Clockwise);
                beacon.show(g_lightIntensity);
                vTaskDelay(300 / portTICK_PERIOD_MS);
                state = Closed;
            }
        }

        if (state == Upgrade) {
            if (steady_clock::now() - startTime >= 10s) {
                state = Closed;
            } else if (readButton()) {
                for (size_t i = 0; i < 4; i++)
                    beacon.top().drawArc(gameColors[i], i * 15, i * 15 + 15, BlackBox::ArcType::Clockwise);
                beacon.show(g_lightIntensity);

                if (levels[btn] < 4)
                    levels[btn]++;

                for (size_t i = 0; i < levels[btn]; i++)
                    doors[i].open();
                vTaskDelay(300 / portTICK_PERIOD_MS);
                startTime = steady_clock::now();
                state = Open;
            }
        }

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

static auto getLogicLevels() {
    int x = doors[0].tamperCheck() - 1 * doors[1].tamperCheck();
    int y = doors[2].tamperCheck() - 1 * doors[3].tamperCheck();
    return std::make_pair(x, y);
}

void trains() {
    struct Operator {
        Rgb color;
        std::function<int(int, int)> op;
        int operator()(int x, int y) {
            if (!x && !y)
                return 0;
            return op(x, y);
        }
    };

    Operator AND = {
        .color = cGreen,
        .op = [](int x, int y) {
            if (!x || !y)
                return 0;
            return (x == 1 && y == 1) ? 1 : -1;
        }
    };

    Operator OR = {
        .color = cBlue,
        .op = [](int x, int y) {
            if (!x || !y)
                return 0;
            return ((x == 1 && y == -1)
                       || (x == -1 && y == 1)
                       || (x == 1 && y == 1))
                ? 1
                : -1;
        }
    };

    Operator NAND = {
        .color = cPink,
        .op = [](int x, int y) {
            if (!x || !y)
                return 0;
            return ((x == 1 && y == -1)
                       || (x == -1 && y == 1)
                       || (x == -1 && y == -1))
                ? 1
                : -1;
        }
    };

    Operator XOR = {
        .color = cYellow,
        .op = [](int x, int y) {
            if (!x || !y)
                return 0;
            return ((x == 1 && y == -1)
                       || (x == -1 && y == 1))
                ? 1
                : -1;
        }
    };

    std::array operators = { AND, OR, NAND, XOR };

    enum {
        Init,
        Reading,
        Switching,
        Fail,
        Timeout,
    };

    int state = Init;

    Circular::CircularInteger<4> direction = esp_random() % 4;
    int activeOperator = esp_random() % 4;

    unsigned errors = 0;
    bool pressed = false;

    auto startTime = steady_clock::now();
    auto originTime = steady_clock::now();

    int blink = true;

    openAllDoors();
    vTaskDelay(300 / portTICK_PERIOD_MS);

    while (true) {
        CHECK_USB();
        for (int i = 0; i < 4; i++) {
            beacon.side(i).fill((i == direction) ? cWhite : cBlack);
        }
        beacon.show(g_lightIntensity);

        if (state == Init) {
            errors = 0;
            int tmp;
            while ((tmp = esp_random() % 4) == activeOperator) {
            }
            activeOperator = tmp;
            state = Reading;
        }

        auto& active = operators[activeOperator];

        if (state == Reading) {
            showColorTop(active.color);
            auto [x, y] = getLogicLevels();
            if (x && y) {
                int out = active(x, y);
                if (out == 1) {
                    state = Switching;
                    originTime = steady_clock::now();
                    startTime = steady_clock::now();
                    pressed = false;
                } else if (out == -1) {
                    errors++;
                    vTaskDelay(300 / portTICK_PERIOD_MS);
                }
            }

            if (errors >= 3)
                state = Fail;
        }

        if (state == Switching) {
            if (steady_clock::now() - startTime >= (pressed ? 3s : 10s) || steady_clock::now() - originTime >= 30s) {
                startTime = steady_clock::now();
                state = Timeout;
            }

            if (readButton()) {
                ++direction;
                vTaskDelay(300 / portTICK_PERIOD_MS);
            }

            for (int i = 0; i < 4; i++) {
                beacon.side(i).fill((blink <= 5 && i == direction) ? cWhite : cBlack);
                beacon.top().drawArc((blink <= 5 && i == direction) ? cWhite : cBlack,
                    i * 15, i * 15 + 15, BlackBox::ArcType::Clockwise);
            }
            beacon.show(g_lightIntensity);
            (++blink) %= 10;
        }

        if (state == Fail) {
            for (int i = 0; i < 4; i++) {
                showColorPerim(cRed);
                vTaskDelay(300 / portTICK_PERIOD_MS);
                showColorPerim(cBlack);
                vTaskDelay(300 / portTICK_PERIOD_MS);
            }
            startTime = steady_clock::now();
            state = Timeout;
        }

        if (state == Timeout) {
            if (steady_clock::now() - startTime >= 30s)
                state = Init;
            beacon.top().fill(cBlack);
            beacon.top().drawArc(cWhite, 0, duration_cast<seconds>(steady_clock::now() - startTime).count() * 2, BlackBox::ArcType::Clockwise);
            beacon.show(g_lightIntensity);
        }

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
