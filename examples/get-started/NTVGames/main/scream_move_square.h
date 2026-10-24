
#pragma once

#include "buttons.h" 

void update_scream_game_state(bool display_ok);
void init_scream_game(bool audio_ok); 
void draw_scream_ui(uint32_t pressed, int mic_level);
