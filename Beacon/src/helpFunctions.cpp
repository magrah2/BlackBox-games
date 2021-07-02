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
    error();
}

void charging() {
    auto& manager = Manager::singleton();
    auto& power = manager.power();
    auto start = chrono::steady_clock::now();

	#ifndef BB_DEBUG
    while (power.usbConnected()) {
        showCharging();
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


void showColor(Rgb rgb) {
    auto& ring = Manager::singleton().ring();
    ring.drawCircle(rgb);
    ring.show();
}

void showGameColors() {
	auto& ring = Manager::singleton().ring();
	ring.drawArc(cRed ,0, 14);      // red
	ring.drawArc(cGreen, 15, 29);    // green
	ring.drawArc(cYellow , 30, 44);   // orange
	ring.drawArc(cBlue , 45, 59);    // blue
	ring.show();
}

void showError() {
    auto& manager = Manager::singleton();
    auto& ring = manager.ring();
	const int delay = 100;

    for(int i = 0; i <6; i++) {
        ring.drawCircle(cError);
        ring.show();
        vTaskDelay(delay / portTICK_PERIOD_MS);
        ring.clear();
        ring.show();
        vTaskDelay(delay / portTICK_PERIOD_MS);
    }
}

void showCharging() {
	auto& ring = Manager::singleton().ring();
	auto& power = Manager::singleton().power();
	ring.setDarkModeValue(7);
    ring.clear();
	ring.show();

    uint8_t percent = power.batteryPercentage()*0.6;

    // cout << "Percent:" << percent << "% ===";

	for(int i = 0; i <percent; i++) {
		ring[i] = cError;
		ring.show();
		vTaskDelay(10 / portTICK_PERIOD_MS);
	}

    ring.setDarkModeValue(50);
}

void showPowerOn() {
	auto& ring = Manager::singleton().ring();
    ring.clear();
    ring.show();
    for(int i = 0; i <60; i++) {
        ring[i] = cWhite;
        ring.show();
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void showPowerOff() {
	auto& ring = Manager::singleton().ring();
	ring.drawCircle(cWhite);
	ring.show();
	for(int i = 0; i <60; i++) {
		ring[i] = cBlack;
		ring.show();
		vTaskDelay(10 / portTICK_PERIOD_MS);
	}        
}

void showLowVoltage() {
	auto& ring = Manager::singleton().ring();
	ring.drawCircle(Rgb(200, 255, 0));
	ring.show();
	for(int i = 0; i <60; i++) {
		ring[i] = cBlack;
		ring.show();
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

void beacon(Rgb rgb) {
    auto& ring = Manager::singleton().ring();
    static uint16_t pos = 60;
    ring.clear();
    ring[pos] = rgb;
    ring.show();

    pos++;

    if(pos >= 111) {
        pos = 60;
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