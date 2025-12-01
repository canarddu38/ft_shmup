#ifndef BOXES_HPP
#define BOXES_HPP

#include "spaceship.hpp"

#define BOX_INFOS_HEIGHT 7
#define HEALTH_BAR_HEIGHT 3
#define HEALTH_BAR_LENGHT 51
#define COLOR_GRAY 8

void 	draw_box(int start_y, int start_x, int height, int width);
void	draw_box_infos(spaceship *Player);
void	draw_magazine(int ammunition);
void	draw_score(spaceship *Player);
void	draw_health(int health);

#endif