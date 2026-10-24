#include "move_square_game.h"
#include "buttons.h"
#include "display.h"
#include "esp_log.h"
#include <time.h>

#define UI_PERIOD_MS 50
#define DESPLAZAMIENTO 5
#define SQUARE_SIZE 10
#define X_LIMIT 124
#define Y_LIMIT 58

#define MIN_RANDOM_X 5 
#define MIN_RANDOM_Y 5  

static int playerPositionX;
static int playerPositionY;
static int targetX;
static int targetY;

void update_square_game_state(bool display_ok){
    
    QueueHandle_t events = buttons_event_queue();
    button_event_t ev;

    if (xQueueReceive(events, &ev, 50) == pdTRUE) {
        if (ev.pressed) {
            update_player_position(ev.id);
            if (player_touches_target()) {
                new_target_position();
            }       
        }
    }
    if (display_ok) {
        draw_ui(buttons_read());
        display_flush();
    }
}

void init_square_game(void) {
        playerPositionX = 62;
        playerPositionY = 29;
        targetX = 0;
        targetY = 0;
        srand(time(NULL)); 
        new_target_position();
}

void new_target_position(void) {
    targetX = MIN_RANDOM_X + rand() % (X_LIMIT - MIN_RANDOM_X + 1);
    targetY = MIN_RANDOM_Y + rand() % (Y_LIMIT - MIN_RANDOM_Y + 1);
}

bool player_touches_target(void) {
    return playerPositionX < targetX + SQUARE_SIZE && playerPositionX + SQUARE_SIZE > targetX &&
           playerPositionY < targetY + SQUARE_SIZE && playerPositionY + SQUARE_SIZE > targetY;
}

void update_player_position(button_id_t button) {
    switch (button) {
        case BTN_LEFT:
            playerPositionX -= DESPLAZAMIENTO; // Move left
            break;
        case BTN_RIGHT:
            playerPositionX += DESPLAZAMIENTO; // Move right
            break;
        case BTN_UP:
            playerPositionY -= DESPLAZAMIENTO; // Move up
            break;
        case BTN_DOWN:
            playerPositionY += DESPLAZAMIENTO; // Move down
            break;
        default:
            break;
    }
}


void draw_ui(uint32_t pressed)
{
    display_clear();

    display_fill_rect(playerPositionX, playerPositionY, SQUARE_SIZE, SQUARE_SIZE, true);
    display_draw_rect(targetX, targetY, SQUARE_SIZE, SQUARE_SIZE);
}