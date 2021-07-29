#include "helpFunctions.hpp"

void menu() {
    auto& manager = Manager::singleton();
    auto& doors = Manager::singleton().doors();

    openAllDoors();
    vTaskDelay(500 / portTICK_PERIOD_MS);
    showGameColors();


    unsigned read = 0;
    while (!read) {
        for (int i = 0; i < 4; i++) {
            read += (doors[i].tamperCheck()) << i;
        }
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
void showBattery(){
    auto& manager = Manager::singleton();
    auto& power = manager.power();
    auto& beacon = Manager::singleton().beacon();
    int charge = power.batteryPercentage(true);
    int endPoint = 59 * charge / 100;
    beacon.top().drawArc(cGreen, 0, endPoint, ArcType::Clockwise);
    beacon.top().drawArc(cRed, endPoint, 59, ArcType::Clockwise);
    beacon.show(16);   
}

void charging(){
    auto& manager = Manager::singleton();
    auto& beacon = Manager::singleton().beacon();
    showBattery();
    vTaskDelay(200 / portTICK_PERIOD_MS);
    beacon.clear();
    beacon.show();
    vTaskDelay(2000 / portTICK_PERIOD_MS);
}

void switching_play_charge() {
    auto& manager = Manager::singleton();
    auto& power = manager.power();
    auto start = chrono::steady_clock::now();
    auto& doors = Manager::singleton().doors();

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
        beacon.show(20);
        vTaskDelay(300 / portTICK_PERIOD_MS);
    
        if ((chrono::steady_clock::now() - start) > 8s){
        
            beacon.clear();
            beacon.show();
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
        */
        std::cout << "Batt:" << power.batteryVoltage() << " mV - percent" << manager.power().batteryPercentage() << endl;    
    }
    while(power.usbConnected()) {
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
        // power.turnOff5V();Manager::singleton()
    }
#else
    openAllDoors();
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    menu();
#endif
}

void showColorTop(Rgb rgb) {
    auto& beacon = Manager::singleton().beacon();
    beacon.top().fill(rgb);
    beacon.show(25);
}

void showColorPerim(Rgb rgb) {
    auto& beacon = Manager::singleton().beacon();
    beacon.perimeter().fill(rgb);
    beacon.show(25);
}

void clearAll() {
    showColorPerim(cBlack);
    showColorTop(cBlack);
}

void showGameColors() {
    auto& beacon = Manager::singleton().beacon();
    for (int i = 0; i < 4; i++) {
        beacon.side(i).fill(gameColors[i]);
    }
    beacon.show(25);
}

void showError() {
    auto& manager = Manager::singleton();
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
// 	auto& beacon = Manager::singleton().beacon();
// 	auto& power = Manager::singleton().power();

//     clearAll();

//     uint8_t percent = power.batteryPercentage()*0.6;

//     // cout << "Percent:" << percent << "% ===";
//     cError.stretchChannelsEvenly(8);

// 	for(int i = 0; i <40; i++) {
// 		beacon.perimeter()[i] = cError;
// 		beacon.show();
// 		vTaskDelay(10 / portTICK_PERIOD_MS);
// 	}

//     cError.stretchChannelsEvenly(50);
// }
/*
void showCharging() {
    auto& beacon = Manager::singleton().beacon();
    auto& power = Manager::singleton().power();

    clearAll();
    for (int i = 0; i < 60; i++) {
        beacon.top()[i] = cError;
        beacon.show(5);
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
*/
void showPowerOn() {
    auto& beacon = Manager::singleton().beacon();
    clearAll();
    for (int i = 0; i < 60; i++) {
        beacon.top()[i] = cWhite;
        beacon.show(25);
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void showPowerOff() {
    auto& beacon = Manager::singleton().beacon();
    showColorTop(cWhite);
    showColorPerim(cBlack);

    for (int i = 0; i < 60; i++) {
        beacon.top()[i] = cBlack;
        beacon.show(25);
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
/*
void showLowVoltage() {
    auto& beacon = Manager::singleton().beacon();
    showColorPerim(cBlack);
    showColorTop(Rgb(200, 255, 0));
    beacon.show(25);
    for (int i = 0; i < 60; i++) {
        beacon.top()[i] = cBlack;
        beacon.show(25);
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
*/
void infinityLoop() {
    auto& power = Manager::singleton().power();

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
    auto& doors = Manager::singleton().doors();
    for (auto& i : doors)
        i.open();
}

void closeAllDoors() {
    auto& doors = Manager::singleton().doors();
    for (auto& i : doors)
        i.close();
    vTaskDelay(100 / portTICK_PERIOD_MS);
}

void showBeacon(Rgb rgb) {
    auto& beacon = Manager::singleton().beacon();
    static uint16_t pos = 0;
    showColorPerim(cBlack);
    beacon.perimeter()[pos] = rgb;
    beacon.show(25);

    pos++;

    if (pos >= 52) {
        pos = 0;
    }
}

bool readButton() {
    auto& ldc = Manager::singleton().ldc();
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

int readButtons() {
    auto& ldc = Manager::singleton().ldc();
    auto& beacon = Manager::singleton().beacon();
    int pressureNow[4];
    static int pressureLast[4] = { 0, 0, 0, 0 };
    int pressureSumNow = 0;
    int pressureSumLast = 0;
    int pressNow = -1;
    ldc.syncChannels();
    for (int a = 0; a < 4; a++) {
        pressureNow[a] = ldc[a];
        pressureSumNow += pressureNow[a];
        pressureSumLast += pressureLast[a];
    }
    if (pressureSumLast == 0) {
        pressNow = -1;
    } else if (pressureSumNow - pressureSumLast > 400000) {
        pressNow = calculateButton(pressureNow, pressureLast);
        beacon.top().fill(gameColors[pressNow]);
        beacon.show(25);
        // showColorTop(cBlue);
        vTaskDelay(100 / portTICK_PERIOD_MS);
        beacon.top().clear();
        beacon.show(25);
        vTaskDelay(400 / portTICK_PERIOD_MS);

    }
    for (int a = 0; a < 4; a++) {
        pressureLast[a] = pressureNow[a];
    }

    if (pressNow != -1) {
        cout << "RB:" << pressNow << endl;
    }
    return pressNow;
}

// int readBattery(){

// }