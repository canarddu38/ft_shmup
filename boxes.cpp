#include "spaceship.hpp"
#include "boxes.hpp"
#include <ncurses.h>


void draw_box(int start_y, int start_x, int height, int width)
{

    mvaddch(start_y, start_x, ACS_ULCORNER);   
    mvaddch(start_y, start_x + width, ACS_URCORNER);
    mvaddch(start_y + height, start_x, ACS_LLCORNER);
    mvaddch(start_y + height, start_x + width, ACS_LRCORNER);

    for (int i = 1; i < width; i++) {
        mvaddch(start_y, start_x + i, ACS_HLINE);
        mvaddch(start_y + height, start_x + i, ACS_HLINE);
    }

    for (int i = 1; i < height; i++) {
        mvaddch(start_y + i, start_x, ACS_VLINE);
        mvaddch(start_y + i, start_x + width, ACS_VLINE);
    }
}

void	draw_magazine(int ammunition)
{
	for (int i = 0; i < 25; i++)
	{
		if (i < ammunition)
			mvaddch(screen_size.y + 2 + HEALTH_BAR_HEIGHT, 3 + i * 2, 'o');
		else
		{
			mvaddch(screen_size.y + 2 + HEALTH_BAR_HEIGHT, 3 + i * 2, '.');
			mvchgat(screen_size.y + 2 + HEALTH_BAR_HEIGHT, 3 + i * 2, 1, A_NORMAL, 2, NULL);
		}
	}
}
void	draw_health(int health)
{
	draw_box(screen_size.y + 1, 2, HEALTH_BAR_HEIGHT, HEALTH_BAR_LENGHT);
	
	mvchgat(screen_size.y + 2, 3 , HEALTH_BAR_LENGHT - 1, A_NORMAL, 2, NULL);
	mvchgat(screen_size.y + 2, 3 , health/2, A_NORMAL, 1, NULL);
	mvchgat(screen_size.y + 3, 3 , HEALTH_BAR_LENGHT - 1, A_NORMAL, 2, NULL);
	mvchgat(screen_size.y + 3, 3 , health/2 + health % 2, A_NORMAL, 1, NULL);
}

void	draw_score(spaceship *Player)
{
	mvprintw(screen_size.y + 1 , HEALTH_BAR_LENGHT + 15 , "%-15s : %.0f", "Score", Player->score);
	mvprintw(screen_size.y + 2 , HEALTH_BAR_LENGHT + 15 , "%-15s : %d", "Time survived", Player->time_survived);
	mvprintw(screen_size.y + 3 , HEALTH_BAR_LENGHT + 15 , "%-15s : x%.1f", "Multiplier", Player->score_multiplier);
	mvprintw(screen_size.y + 4 , HEALTH_BAR_LENGHT + 15 , "%-15s : %.1d", "Waves Defeated", Player->wave);
}

void	draw_box_infos(spaceship *Player)
{
	draw_box(screen_size.y, 0, BOX_INFOS_HEIGHT - 1, screen_size.x - 1);
	draw_health(Player->health);
	draw_magazine(Player->ammunition);
	draw_score(Player);
}