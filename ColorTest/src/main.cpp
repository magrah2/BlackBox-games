// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "library/BlackBox_Manager.hpp"
// #include <iostream>

// using namespace BlackBox;
// using namespace std;

// extern "C" {
// void app_main() {

//     // Setup code here

//     while (true) {

//         // Loop code here

//         vTaskDelay(100 / portTICK_PERIOD_MS); // DO NOT REMOVE THIS!
//     }
// }
// }

#include "library/BlackBox_Manager.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

using namespace BlackBox;

extern "C" {

void app_main() {
    Manager& man = Manager::singleton(); // Get instance of Manager class

    man.init(); // Initialize BlackBox's peripherials

    man.power().turnOn5V(); // Turn on 5V power, meaning LEDs and lock

    Ring& ring = man.ring(); // Get instance of Ring class

    ring.clear(); // Clear display(Ring)
    ring.setDarkModeValue(50); // Set maximum value for darkmode
    ring.enableDarkMode(); // Enable dark mode
    ring.rotate(8);

    
    while (true) {
        // Draw arcs with given parameters. See documentation for more information 
        ring.drawArc(Rgb(255,0,0), 0, 14);      // red
        ring.drawArc(Rgb(0, 255,0), 15, 29);    // green
        ring.drawArc(Rgb(255,110, 0), 30, 44);   // orange
        ring.drawArc(Rgb(0,0, 255), 45, 59);    // blue

        ring.show(); // Show changes on display

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
}