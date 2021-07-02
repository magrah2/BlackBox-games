#include "games.hpp"

#include "helpFunctions.hpp"

void game0() {
    static uint8_t gameNum = 0;
    auto& manager = Manager::singleton();
    auto& power = manager.power();
    auto& ring = manager.ring();
    showColor(gameColors[gameNum]);
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
            beacon();

            if(readButton()) {
                state = 1;
                openStart = esp_timer_get_time() / 1000LL;
                
                openAllDors();             
            }

            break;

        case 1:
            uint32_t openNow = esp_timer_get_time() / 1000LL;

            if(openNow - openStart > 30000) {
                state = 0;

                closeAllDors();
            }

            ring.drawCircle(cRed);
            ring.show();

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
    showColor(gameColors[gameNum]);
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


        beacon(gameColors[activeCollor]);
        if(readButton()){
            doors[activeCollor].open();
            openStart = esp_timer_get_time()/1000;
        }
        if((esp_timer_get_time()/1000) - openStart >= 20000)
        {
            closeAllDors();
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