#include "helpFunctions.hpp"
#include <array>

void updateAverage(Coords coords);

int g_lightIntensity = 25;

int lightIntensity() {
    constexpr array<int, 4> intensities = {255, 150, 50, 10};
    for (int i = 0; i < 4; i++)
        beacon.top().drawArc(Rgb(0, intensities[i], 0), i * 15, (i * 15) + 13, ArcType::Clockwise);
    beacon.perimeter().clear();
    beacon.show();
    int button;
    if((button = readButtons()) != -1)
        return intensities[button];
    return g_lightIntensity;
}

void menu() {
    updateAverage(manager.touchpad().calculate());
    openAllDoors();
    vTaskDelay(500 / portTICK_PERIOD_MS);
    showGameColors();

    unsigned read = 0;
    while (!read) {
        for (int i = 0; i < 4; i++) {
            read += (doors[i].tamperCheck()) << i;
        }
        g_lightIntensity = lightIntensity();
        beacon.top().clear();
        showGameColors();
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }

    switch (read) {
    case 1:
        game0();
        break;
    case 2:
        game1();
        break;
    case 4:
        game2();
        break;
    case 8:
        game3();
        break;
    default:
        showError();
        // menu();
        break;
    }
}

void showEmptyBattery() {
    showError();
}
void showBattery() {

    int charge = power.batteryPercentage(true);
    int endPoint = 59 * charge / 100;
    beacon.top().drawArc(cGreen, 0, endPoint, ArcType::Clockwise);
    beacon.top().drawArc(cRed, endPoint, 59, ArcType::Clockwise);
    beacon.show(g_lightIntensity);
}

void charging() {

    showBattery();
    vTaskDelay(200 / portTICK_PERIOD_MS);
    beacon.clear();
    beacon.show(g_lightIntensity);
    vTaskDelay(2000 / portTICK_PERIOD_MS);
}

void switching_play_charge() {

    auto start = chrono::steady_clock::now();

#ifndef BB_DEBUG
    clearAll();
    closeAllDoors();
    while ((power.usbConnected()) && ((chrono::steady_clock::now() - start) < 8s)) {
        showBattery();
        vTaskDelay(200 / portTICK_PERIOD_MS);

        /*    int charge = power.batteryPercentage(true);
        int endPoint = 59 * charge / 100;
        beacon.top().drawArc(cGreen, 0, endPoint, ArcType::Clockwise);
        beacon.top().drawArc(cRed, endPoint, 59, ArcType::Clockwise);
        beacon.show(g_lightIntensity);
        vTaskDelay(300 / portTICK_PERIOD_MS);
    
        if ((chrono::steady_clock::now() - start) > 8s){
        
            beacon.clear();
            beacon.show(g_lightIntensity);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
        */
        std::cout << "Batt:" << power.batteryVoltage() << " mV - percent" << manager.power().batteryPercentage() << endl;
    }
    while (power.usbConnected()) {
        charging();
    }

    if ((chrono::steady_clock::now() - start) < 8s) {
        // openAllDoors();
        clearAll();
        vTaskDelay(100 / portTICK_PERIOD_MS);
        menu();
    } else {
        closeAllDoors();
        showPowerOff();
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        power.turnOff();
        // power.turnOff5V();manager
    }
#else
    clearAll();
    closeAllDoors();

    vTaskDelay(100 / portTICK_PERIOD_MS);
    menu();
#endif
}

void showColorTop(Rgb rgb) {

    beacon.top().fill(rgb);
    beacon.show(g_lightIntensity);
}

void showColorPerim(Rgb rgb) {

    beacon.perimeter().fill(rgb);
    beacon.show(g_lightIntensity);
}

void clearAll() {
    showColorPerim(cBlack);
    showColorTop(cBlack);
}

void showGameColors() {

    for (int i = 0; i < 4; i++) {
        beacon.side(i).fill(gameColors[i]);
    }
    beacon.show(g_lightIntensity);
}

void showError() {

    const int delay = 100;

    clearAll();

    for (int i = 0; i < 6; i++) {
        showColorPerim(cError);
        vTaskDelay(delay / portTICK_PERIOD_MS);
        showColorPerim(cBlack);
        vTaskDelay(delay / portTICK_PERIOD_MS);
    }
}

// void showCharging() {
//
//
//     clearAll();

//     uint8_t percent = power.batteryPercentage()*0.6;

//     // cout << "Percent:" << percent << "% ===";
//     cError.stretchChannelsEvenly(8);

// 	for(int i = 0; i <40; i++) {
// 		beacon.perimeter()[i] = cError;
// 		beacon.show(g_lightIntensity);
// 		vTaskDelay(10 / portTICK_PERIOD_MS);
// 	}

//     cError.stretchChannelsEvenly(50);
// }
/*
void showCharging() {
    
    
    clearAll();
    for (int i = 0; i < 60; i++) {
        beacon.top()[i] = cError;
        beacon.show(g_lightIntensity);
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
*/
void showPowerOn() {

    clearAll();
    for (int i = 0; i < 60; i++) {
        beacon.top()[i] = cWhite;
        beacon.show(g_lightIntensity);
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void showPowerOff() {

    showColorTop(cWhite);
    showColorPerim(cBlack);

    for (int i = 0; i < 60; i++) {
        beacon.top()[i] = cBlack;
        beacon.show(g_lightIntensity);
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
/*
void showLowVoltage() {
    
    showColorPerim(cBlack);
    showColorTop(Rgb(200, 255, 0));
    beacon.show(g_lightIntensity);
    for (int i = 0; i < 60; i++) {
        beacon.top()[i] = cBlack;
        beacon.show(g_lightIntensity);
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
*/
void infinityLoop() {

    while (true) {
        vTaskDelay(100 / portTICK_PERIOD_MS);

#ifndef BB_DEBUG
        if (power.usbConnected()) {
            switching_play_charge();
        }
#endif
    }
}

void openAllDoors() {
    for (auto& i : doors)
        i.open();
}

void closeAllDoors() {
    for (auto& i : doors)
        i.close();
    vTaskDelay(100 / portTICK_PERIOD_MS);
}

void showBeacon(Rgb rgb) {

    static uint16_t pos = 0;
    showColorPerim(cBlack);
    beacon.perimeter()[pos] = rgb;
    beacon.show(g_lightIntensity);

    pos++;

    if (pos >= 52) {
        pos = 0;
    }
}

bool readButton() {
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

int calculateButton(int pressureNow[4], int pressureLast[4]) {
    int diferensions[4];
    uint sumDif[4];
    int output[2]; // 0 is button - 1 is value of difference
    for (int a = 0; a < 4; a++) {
        diferensions[a] = abs(pressureLast[a] - pressureNow[a]);
    }
    for (int a = 0; a < 3; a++) {
        sumDif[a] = diferensions[a] + diferensions[a + 1];
    }
    sumDif[3] = diferensions[3] + diferensions[0];
    output[1] = sumDif[1];
    for (int a = 0; a < 4; a++) {
        if (sumDif[a] >= output[1]) {
            output[1] = sumDif[a];
            output[0] = a;
        }
    }
    printf("\n");
    if (output[0] == 0) {
        output[0] = 2;
    } else if (output[0] == 2) {
        output[0] = 0;
    }

    return output[0];
}

bool isInRange(int value, int min, int max) {
    return (value >= min) && (value <= max);
}
struct ButtonRead {
    int button = -1;
    bool change = false;
};
static Coords average;
void updateAverage(Coords coords) {
    if (average.pressure == 0) {
        average.x = coords.x;
        average.y = coords.y;
        average.pressure = coords.pressure;
    }
    average.x = ((float(average.x) * 0.75) + coords.x) / 1.75;
    average.y = ((float(average.y) * 0.75) + coords.y) / 1.75;
    average.pressure = ((float(average.pressure) * 0.75) + coords.pressure) / 1.75;
}

int readButtons() {
    auto& ldc = manager.touchpad();

    static constexpr int pressureThreshold = 5;
    static constexpr int coordThreshold = 10;

    static int lastButton = -1;

    Coords coords = ldc.calculate();
    int quadrant = -1;
    cout << coords.x << "\t" << coords.y << "\t" << coords.pressure << "\t";
    cout << average.x << "\t" << average.y << "\t" << average.pressure << "\t";
    cout << (coords.x - average.x) << "\t" << (coords.y - average.y) << "\t" << (coords.pressure - average.pressure) << "\n";

    if ((coords.pressure - average.pressure) > pressureThreshold) {
        if (!(isInRange((coords.x - average.x), -(coordThreshold * 2), (coordThreshold * 2)) && isInRange((coords.y - average.y), -(coordThreshold * 2), (coordThreshold * 2)))) {
            if (isInRange((coords.x - average.x), -((coordThreshold * 2)), (coordThreshold * 2)))
                quadrant = ((coords.y - average.y) > coordThreshold) ? 3 : 1;
            else
                quadrant = ((coords.x - average.x) > coordThreshold) ? 0 : 2;
        } else {
            quadrant = -1;
        }
    } else if (((coords.pressure - average.pressure) < max((pressureThreshold / 4), 1) && (coords.pressure - average.pressure) > -max((pressureThreshold / 4), 1)))
        updateAverage(coords);

    ButtonRead out = {
        .button = (lastButton != quadrant) ? quadrant : -1,
        .change = lastButton != quadrant,
    };

    lastButton = quadrant;
    return out.button;
}

// int readBattery(){

// }