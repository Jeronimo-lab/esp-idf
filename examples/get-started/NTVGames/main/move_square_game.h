
#pragma once
#include <stdbool.h>
#include "buttons.h" 

void update_square_game_state(bool display_ok);
void init_square_game(void); 
void update_player_position(button_id_t button);
void draw_ui(uint32_t button_state);