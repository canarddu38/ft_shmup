#include <ncurses.h>
#include "laser.hpp"
#include "spaceship.hpp"
#include "boxes.hpp"
#include "config.hpp"

void	announce_vertical_laser(int x)
{
	mvprintw(0, x - 1, "===");
	mvprintw(screen_size.y - 1, x - 1, "===");
	mvchgat(0, x - 1 , 3, A_NORMAL, BLUE_ANNOUCE_LASER_COLOR, NULL);
	mvchgat(screen_size.y - 1, x - 1 , BLUE_ANNOUCE_LASER_COLOR, A_NORMAL, 3, NULL);
	for (int i = 1; i <= screen_size.y - 2; i++)
		mvaddch(i, x, ACS_VLINE);
}

void	fire_vertical_laser(int x)
{
	for (int i = 1; i <= screen_size.y - 2; i++)
	{
		mvaddch(i, x, ' ');
		mvchgat(i, x - 1 , 3, A_NORMAL, BLUE_FIRE_LASER_COLOR, NULL);
	}
}

void	clear_vertical_laser(int x)
{
	for (int i = 1; i <= screen_size.y - 2; i++)
		mvchgat(i, x - 1 , 3, A_NORMAL, 0, NULL);
}

void	is_in_vertical_laser(spaceship *Player, int laser,unsigned int tick)
{
	if (Player->pos.x >= laser - 1 && Player->pos.x <= laser + 1)
	{
		if (Player->last_hit + TICKRATE >= tick)
			return ;
		Player->health = MAX(Player->health - 50, 0);
		Player->last_hit = tick;
		draw_health(Player->health);
	}
}

int	get_next_announcment(laser *lasers, unsigned int tick)
{
	int	next_laser = 0;

	for (int i = 0; i < LASER_NUMBER; i++)
	{
		if ((unsigned int)(lasers[i].announcment) > tick / TICKRATE)
			if (lasers[i].announcment < next_laser || next_laser == 0)
				next_laser = lasers[i].announcment;
	}
	return (next_laser);
}

int	get_next_start(laser *lasers, unsigned int tick)
{
	int	next_laser = 0;

	for (int i = 0; i < LASER_NUMBER; i++)
	{
		if ((unsigned int)(lasers[i].start) > tick / TICKRATE)
			if (lasers[i].start < next_laser || next_laser == 0)
				next_laser = lasers[i].start;
	}
	return (next_laser);
}

int	get_next_end(laser *lasers, int time)
{
	int	next_laser = 0;

	for (int i = 0; i < LASER_NUMBER; i++)
	{
		if (lasers[i].end > time)
			if (lasers[i].end < next_laser || next_laser == 0)
				next_laser = lasers[i].end;
	}
	return (next_laser);
}

void	do_laser(laser *lasers, int announcment, void f(int))
{
	for (int i = 0; i < LASER_NUMBER; i++)
	{
		if (lasers[i].announcment == announcment)
			f(lasers[i].pos);
	}
}
