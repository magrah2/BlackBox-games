#include "helpFunctions.hpp"

void menu() {
    auto& manager = Manager::singleton();
    auto& doors = Manager::singleton().doors();

    showGameColors();
    // openAllDors();

    unsigned read = 0;
    while (!read) {
        for(int i = 0; i < 4; i++) {
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

void charging() {
    auto& manager = Manager::singleton();
    auto& power = manager.power();
    auto start = chrono::steady_clock::now();

	#ifndef BB_DEBUG
    while (power.usbConnected()) {
        showCharging();
        // showPowerOff();
        vTaskDelay(200 / portTICK_PERIOD_MS);
        cout << "Batt:" << manager.power().batteryVoltage() << " V - percent" << manager.power().batteryPercentage() << endl;
    }
    if((chrono::steady_clock::now() - start) <= 8s) {
        openAllDors();
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        menu();
    }        
    else {
        closeAllDors();       
        showPowerOff();
        power.turnOff();
        // power.turnOff5V();Manager::singleton()
    }
	#else
    openAllDors();
    vTaskDelay(1000 / portTICK_PERIOD_MS);
	menu();
	#endif
}


void showColorTop(Rgb rgb) {
    auto& beacon = Manager::singleton().beacon();
    for(int i = 0; i < 60; i++) {
        beacon.top()[i] = rgb;
    }
    beacon.show(50);
}

void showColorPerim(Rgb rgb) {
    auto& beacon = Manager::singleton().beacon();
    for(int i = 0; i < 52; i++) {
        beacon.perimeter()[i] = rgb;
    }
    beacon.show(50);
}

void clearAll() {
    showColorPerim(cBlack);
    showColorTop(cBlack);    
}

void showGameColors() {
	auto& beacon = Manager::singleton().beacon();
	for(int i = 0; i <4; i++) {
        for(int j = 0; j <12; j++) {
            beacon.onSide(i, j) = gameColors[i];
        }
    }
	beacon.show(50);
}

void showError() {
    auto& manager = Manager::singleton();
    auto& beacon = manager.beacon();
	const int delay = 100;

    clearAll();

    for(int i = 0; i <6; i++) {
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

void showCharging() {
	auto& beacon = Manager::singleton().beacon();
    auto& power = Manager::singleton().power();


    clearAll();
    for(int i = 0; i <60; i++) {
        beacon.top()[i] = cError;
        beacon.show(5);
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void showPowerOn() {
	auto& beacon = Manager::singleton().beacon();
    clearAll();
    for(int i = 0; i <60; i++) {
        beacon.top()[i] = cWhite;
        beacon.show(50);
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void showPowerOff() {
	auto& beacon = Manager::singleton().beacon();
	showColorTop(cWhite);
    showColorPerim(cBlack);
    
	for(int i = 0; i <60; i++) {
		beacon.top()[i] = cBlack;
		beacon.show(50);
		vTaskDelay(10 / portTICK_PERIOD_MS);
	}        
}

void showLowVoltage() {
	auto& beacon = Manager::singleton().beacon();
    showColorPerim(cBlack);
	showColorTop(Rgb(200, 255, 0));
	beacon.show(50);
	for(int i = 0; i <60; i++) {
		beacon.top()[i] = cBlack;
		beacon.show(50);
		vTaskDelay(10 / portTICK_PERIOD_MS);
	}        
}

void infinityLoop() {
    auto& power = Manager::singleton().power();

	while(true) {
		vTaskDelay(100 / portTICK_PERIOD_MS);

#ifndef BB_DEBUG
        if (power.usbConnected()) {
            charging();
        }
#endif
    }
}

void openAllDors() {
    auto& doors = Manager::singleton().doors();
    for (auto &i : doors)
        i.open();
}

void closeAllDors() {
    auto& doors = Manager::singleton().doors();
    for (auto &i : doors)
        i.close();
}

void showBeacon(Rgb rgb) {
    auto& beacon = Manager::singleton().beacon();
    static uint16_t pos = 0;
    showColorPerim(cBlack);
    beacon.perimeter()[pos] = rgb;
    beacon.show(50);

    pos++;

    if(pos >= 52) {
        pos = 0;
    }
}

bool readButton() {
    auto& ldc = Manager::singleton().ldc();
    static int pressureNow;
    static int pressureLast;
    static bool firstRead = true;
    bool pressNow;

    if(firstRead) {
        ldc.syncChannels();
        pressureLast = ldc[0] + ldc[1] + ldc[2] + ldc[3];
        firstRead = false;
    }

    ldc.syncChannels();

    pressureNow = ldc[0] + ldc[1] + ldc[2] + ldc[3];

    int diff = pressureNow - pressureLast;
    if(diff < 0) {
        diff = 0;
    }
    
    if(diff >= 600000) {
        pressNow = true;
    }
    else {
        pressNow = false;
    }
    pressureLast = pressureNow;


    return pressNow;
}

int calculateButton(int pressureNow[4], int pressureLast[4]){
    int diferensions[4];
    uint sumDif[4];
    int output[2]; // 0 is button - 1 is value of difference
    for (int a = 0; a < 4; a++){
        diferensions[a] = abs(pressureLast[a] - pressureNow[a]);
    }
    for (int a = 0; a < 3; a++){
        sumDif[a] = diferensions[a]+diferensions[a+1];
    }
    sumDif[3] = diferensions[3]+diferensions[0];
    output[1] = sumDif[1];
    for(int a = 0; a < 4; a++){
    	if(sumDif[a] >= output[1]){
            output[1] = sumDif[a];
            output[0] = a;
        }
    }
    printf("\n");
    if(output[0] == 0){
        output[0] = 2;
    }
    else if(output[0] == 2){
        output[0] = 0;
    }

    return output[0];
}

int readButtons(){
    auto& ldc = Manager::singleton().ldc();
    auto& beacon = Manager::singleton().beacon();
    int pressureNow[4];
    static int pressureLast[4] = {0,0,0,0};
    int pressureSumNow = 0;
    int pressureSumLast = 0;
    int pressNow = -1;
    ldc.syncChannels();
    for(int a = 0; a < 4; a++){
        pressureNow[a] = ldc[a];
        pressureSumNow += pressureNow[a];
        pressureSumLast += pressureLast[a];
    }
    if(pressureSumLast == 0){
        pressNow = -1;
    }
    else if(pressureSumNow - pressureSumLast > 400000){
        pressNow = calculateButton(pressureNow, pressureLast);
        beacon.top().fill(gameColors[pressNow]);
        beacon.show();
        // showColorTop(cBlue);
        vTaskDelay(100 / portTICK_PERIOD_MS);
        beacon.top().clear();
        beacon.show();
    }
    for (int a = 0; a < 4; a++)
    {
        pressureLast[a] = pressureNow[a];
    }

    if(pressNow != -1) {
        cout << "RB:" << pressNow << endl;
    }
    return pressNow;
}

// int readBattery(){

// }