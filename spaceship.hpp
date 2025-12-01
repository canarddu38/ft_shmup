#ifndef SPACESHIP_HPP
#define SPACESHIP_HPP

#include <iostream>
#include <ncurses.h>
#include "maths.hpp"
#include <string.h>
#include "config.hpp"

typedef struct point
{
	int	x;
	int	y;
}	point;

extern point screen_size;

class object
{
	public:
		point	pos;
		point	direction;
		int		character;
};

class spaceship : public object
{
	public:
		int				speed;
		int				projectile_speed;
		int				fire_rate;
		int				reload_delay;
		int				ammunition;
		int				wave;
		int				time_survived;
		float			health;
		float			score;
		float			score_multiplier;
		unsigned int	last_fire;
		unsigned int	last_hit;
		bool			is_passive;
		int				shoot_power;
	spaceship(int y, int x, int speed, int projectile_speed, int fire_rate, int character, float health, int reload_delay)
	{
		this->shoot_power = 0;
		this->is_passive = false;
		this->wave = 1;
		this->pos.y = y;
		this->pos.x = x;
		this->speed = speed;
		this->projectile_speed = projectile_speed;
		this->fire_rate = fire_rate;
		this->character = character;
		this->health = health;
		this->ammunition = MAX_AMMUNITION;
		this->reload_delay = reload_delay;
		this->direction.y = 0;
		this->direction.x = 1;
		this->score = 0;
		this->score_multiplier = 1;
		this->last_fire = 0;
	}
};

class	bullet : public object
{
	public:
		int			damage;
		spaceship	*owner;
	bullet(int damage, point pos, point direction, spaceship *owner)
	{
		this->character = '~';
		this->damage = damage;
		this->pos.x = pos.x + direction.x;
		this->pos.y = pos.y + direction.y;
		this->direction.x = direction.x;
		this->direction.y = direction.y;
		this->owner = owner;
	}
};

void	move_obj(object *obj, int y, int x);
void	load_enemies(spaceship *enemies_list[MAX_ENEMIES], int number, int x, int y);
int		get_last_remaining_enemy(spaceship *enemies_list[MAX_ENEMIES]);
int		move_enemies(unsigned int tick, spaceship *Player, spaceship *enemies_list[MAX_ENEMIES], bullet *bullet_list[MAX_BULLETS], point *direction);
void	move_bullets(spaceship *Player, bullet *bullet_list[MAX_BULLETS], spaceship *enemies_list[MAX_ENEMIES]);
void	delete_object(object **object);
int		index_of(char c, char *str);
void	draw_health(int health);
void	get_enemy_movement(spaceship *enemy, point *direction);
void	spawn_powerup(spaceship *enemies_list[MAX_ENEMIES], int x, int y);

#endif