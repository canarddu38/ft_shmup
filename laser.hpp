#ifndef LASER_HPP
#define LASER_HPP

#define BLUE_ANNOUCE_LASER_COLOR 3
#define BLUE_FIRE_LASER_COLOR 4
#define TIER_1_ANNOUNCE_LASER_START 30
#define TIER_1_FIRE_LASER_START 32
#define TIER_1_FIRE_LASER_END 33

#define LASER_NUMBER 395

#include "spaceship.hpp"

typedef struct laser
{
	int	announcment;
	int	start;
	int	end;
	int	pos;
	int	type;
}	laser;

void	is_in_vertical_laser(spaceship *Player, int laser, unsigned int tick);
int		get_next_announcment(laser *lasers, unsigned int tick);
void	do_laser(laser *lasers, int announcment, void f(int));
int		get_next_start(laser *lasers, unsigned int tick);
int		get_next_end(laser *lasers, int tine);
void	announce_vertical_laser(int x);
void	clear_vertical_laser(int x);
void	fire_vertical_laser(int x);

#endif