#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "audio.h"
#include "buttons.h"
#include "display.h"

void enemy_option();
void choose_option(button_id_t button);
void update_cpn_game_state(bool display_ok);
void init_cpn_game(void);
void draw_cpn_ui(uint32_t pressed);