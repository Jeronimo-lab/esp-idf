#include "move_square_game.h"
#include "buttons.h"
#include "display.h"
#include "esp_log.h"

int position_x; 
int position_y;
void update_square_game_state(bool display_ok){
    QueueHandle_t events = buttons_event_queue();
    button_event_t ev;

    while (xQueueReceive(events, &ev, 0) == pdTRUE) {
        if (ev.pressed) {
            update_player_position(ev.id);
        }
    }


    if (display_ok) {
        draw_ui(buttons_read());
        display_flush();
    }
}

void init_square_game(void) {
    position_x = 59;
    position_y = 27;
}


void update_player_position(button_id_t button) {
ESP_LOGI("game", "boton %d -> x=%d y=%d", button, position_x, position_y);
    switch (button) {
        case BTN_LEFT:
            position_x -= 5; // Move left
            break;
        case BTN_RIGHT:
            position_x += 5; // Move right
            break;
        case BTN_UP:
            position_y -= 5; // Move up
            break;
        case BTN_DOWN:
            position_y += 5; // Move down
            break;
        default:
            break;
    }
}


void draw_ui(uint32_t pressed){
    display_clear();
    display_fill_rect(position_x, position_y, 10, 10, true);
}