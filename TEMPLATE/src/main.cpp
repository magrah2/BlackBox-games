#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "library/BlackBox_Manager.hpp"
#include <iostream>

using namespace BlackBox;
using namespace std;

extern "C" {
void app_main() {

    // Setup code here

    while (true) {

        // Loop code here

        vTaskDelay(100 / portTICK_PERIOD_MS); // DO NOT REMOVE THIS!
    }
}
}
