#include "scream_move_square.h"
#include "audio.h"
#include "buttons.h"
#include "display.h"
#include "esp_log.h"
#include <time.h>

#define UI_PERIOD_MS 50
#define SQUARE_SIZE 10
#define Y_MAX 54

static int playerPositionX;
static int playerPositionY;
static bool audio_okey;

void modify_height(int mic_level) {
    playerPositionY = Y_MAX - (mic_level * Y_MAX) / 80;
}

void update_scream_game_state(bool display_ok){
    
    int mic_level = audio_okey ? audio_mic_level() : 0;

    modify_height(mic_level);

    if (display_ok) {
        draw_scream_ui(buttons_read(), mic_level);
        display_flush();
    }
}

void init_scream_game(bool audio_ok) {
        playerPositionX = 62;
        playerPositionY = 54;
        audio_okey = audio_ok;

}



void draw_scream_ui(uint32_t pressed, int mic_level)
{
    char line[24];

    display_clear();

    display_fill_rect(playerPositionX, playerPositionY, SQUARE_SIZE, SQUARE_SIZE, true);

    if (audio_okey) {
        snprintf(line, sizeof(line), "%3d", mic_level);
        display_text(107, 14, line, false);
    } else {
        display_text(107, 14, "ERR", false);
    }
}



