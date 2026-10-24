#include "move_square_game.h"
#include "buttons.h"
#include "display.h"
#include "esp_log.h"
#include <time.h>

#define UI_PERIOD_MS 50
static bool navaja_option = false;
static bool papiro_option = false;
static bool cuarzo_option = false;

static bool enemy_navaja = false;
static bool enemy_papiro = false;
static bool enemy_cuarzo = false;

void enemy_option() {
    int min = 1;
    int max = 3;
    int numero = min + rand() % (max - min + 1);
    switch (numero) {
        case 1:  
            enemy_navaja = true;
            enemy_papiro = false;
            enemy_cuarzo = false;
        break;
        case 2: 
            enemy_papiro = true;
            enemy_cuarzo = false;
            enemy_navaja = false;
        break;
        case 3: 
            enemy_cuarzo = true;
            enemy_papiro = false;
            enemy_navaja = false;
            
        break;
        default:
            enemy_papiro = false;
            enemy_cuarzo = false;
            enemy_navaja = false;
            break;
    }

}

void choose_option(button_id_t button) {
    switch (button) {
        case BTN_KEY1:  
            navaja_option = true;
            papiro_option = false;
            cuarzo_option = false;
        break;
        case BTN_KEY2: 
            papiro_option = true;
            cuarzo_option = false;
            navaja_option = false;
        break;
        case BTN_KEY3: 
        cuarzo_option = true;
            papiro_option = false;
            navaja_option = false;
            
        break;
        default:
            cuarzo_option = false;
            papiro_option = false;
            navaja_option = false;
            break;
    }
}

void update_cpn_game_state(bool display_ok){
    
    QueueHandle_t events = buttons_event_queue();
    button_event_t ev;

    if (xQueueReceive(events, &ev, 50) == pdTRUE){ 
            if (ev.pressed) {
                choose_option(ev.id);
                enemy_option();
            }
        }

        if (display_ok) {
            draw_ui(buttons_read());
            display_flush();
        }
    
}

void init_cpn_game(void) {

        srand(time(NULL)); 
}


void draw_ui(uint32_t pressed)
{
    display_clear();
    display_text(85, 5, "Cuarzo", enemy_cuarzo);
    display_text(45, 5, "Papiro", enemy_papiro);
    display_text(5, 5, "Navaja", enemy_navaja);
    display_text(85, 50, "Cuarzo", cuarzo_option);
    display_text(45, 50, "Papiro", papiro_option);
    display_text(5, 50, "Navaja", navaja_option);

    if((enemy_cuarzo && cuarzo_option) || (enemy_papiro && papiro_option) || (navaja_option && enemy_navaja)) {
        display_text(45, 30, "EMPATE", true);
    }

    if((navaja_option && enemy_papiro) || (papiro_option && enemy_cuarzo) || (cuarzo_option && enemy_navaja)) {
        display_text(45, 30, "GANASTE", true);
    }

    if((navaja_option && enemy_cuarzo) || (papiro_option && enemy_navaja) || (cuarzo_option && enemy_papiro)) {
        display_text(45, 30, "PERDISTE", true);
    }

}