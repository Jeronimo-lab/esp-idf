#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"


#include "buttons.h"
#include "display.h"

static const char *TAG = "demo";
static bool file_loaded = false;


void display_waiting_animation(int x)
{
        display_text(x, 24, ".  ", false);
        display_flush();       
        vTaskDelay(pdMS_TO_TICKS(250));
        display_text(x, 24, ".. ", false);
        display_flush();
        vTaskDelay(pdMS_TO_TICKS(250));
        display_text(x, 24, "...", false);
        display_flush();
        vTaskDelay(pdMS_TO_TICKS(500));
        display_flush();
}

void init(void){
    bool display_ok = display_init() == ESP_OK;
    if (!display_ok) {
        ESP_LOGE(TAG, "display init failed - continuing without screen");
    }
    
        display_clear();
        display_text(10, 8, "Esperando archivo!", false);
        display_text(10, 24, ".", false);
        
}

void app_main(void){
    init();

    while(!file_loaded){
        display_waiting_animation(10);
    }
}


