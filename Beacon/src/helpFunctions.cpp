#include "helpFunctions.hpp"


void menu() {
    auto& manager = Manager::singleton();

    showGameColors();

    auto& doors = manager.doors();
    for (auto &i : doors)
        i.open();

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

void charging() {
    auto& power = Manager::singleton().power();
    auto start = chrono::steady_clock::now();

	#ifndef BB_DEBUG
    while (power.usbConnected()) {
        showCharging();
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
    if((chrono::steady_clock::now() - start) <= 5s) {
        menu();
    }        
    else {
        showPowerOff();
        power.turnOff();
    }
	#else
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
        ring.drawCircle(gameColors[5]);
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
	ring.clear();
	ring.show();

    uint8_t percent =  power.batteryPercentage()*0.6;

    cout << "Percent:" << percent;

	for(int i = 0; i <percent; i++) {
		ring[i] = gameColors[0];
		ring.show();
		vTaskDelay(20 / portTICK_PERIOD_MS);
	}
}

void showPowerOn() {
	auto& ring = Manager::singleton().ring();
    ring.clear();
    ring.show();
    for(int i = 0; i <60; i++) {
        ring[i] = cWhite;
        ring.show();
        vTaskDelay(20 / portTICK_PERIOD_MS);
    }
}

void showPowerOff() {
	auto& ring = Manager::singleton().ring();
	ring.drawCircle(cWhite);
	ring.show();
	for(int i = 0; i <60; i++) {
		ring[i] = cBlack;
		ring.show();
		vTaskDelay(20 / portTICK_PERIOD_MS);
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