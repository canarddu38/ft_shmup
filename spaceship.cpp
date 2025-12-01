#include "spaceship.hpp"

char	enemies_types[TOTAL_ENEMIES_TYPES] = ENEMIES_TYPES;

void	get_enemy_movement(spaceship *enemy, point *direction)
{
	direction->x = -1;
	direction->y = 0;
	int	type = index_of(enemy->character, enemies_types);
	if (type < 0 || type >= TOTAL_AMMO_TYPES)
		return;
	switch (type)
	{
		case 0:
			direction->x = -1;
			direction->y = -1 + 2 * random_range(0, 2);
			break;
		case 1:
			direction->x = -1;
			direction->y = random_range(0, 7) - 3;
			break;
		case 2:
			direction->x = -2;
			direction->y = -1 + 2 * random_range(0, 2);
			break;
	}
}

void	spawn_powerup(spaceship *enemies_list[MAX_ENEMIES], int x, int y)
{
	int	i = 0;
	while (i < MAX_ENEMIES && enemies_list[i])
		i++;
	if (i >= MAX_ENEMIES) return;
	enemies_list[i] = new spaceship(random_range(y, screen_size.y - y), x, 0, 0, 0, '*', 0, 0);
	enemies_list[i]->is_passive = true;
	enemies_list[i]->direction.x = -1;
	enemies_list[i]->direction.y = 0;
	move_obj(enemies_list[i], 0, 0);
}

void	load_enemies(spaceship *enemies_list[MAX_ENEMIES], int number, int x, int y)
{
	int	i;
	int	type = 0;
	for (i = 0; i < number && i < MAX_ENEMIES && i < screen_size.y - 2; i++)
	{

		type = random_range(0, TOTAL_ENEMIES_TYPES);
		enemies_list[i] = new spaceship(random_range(y, screen_size.y - y),
										x,
										0, 0, 0,
										enemies_types[type],
										10 * type,
										0
		);
		enemies_list[i]->score = type * 10;
		enemies_list[i]->direction.x = -1;
		enemies_list[i]->direction.y = 0;
		move_obj(enemies_list[i], 0, 0);
	}
}

int	move_enemies(unsigned int tick, spaceship *Player, spaceship *enemies_list[MAX_ENEMIES], bullet *bullet_list[MAX_BULLETS], point *direction)
{
	int count = 0;
	for (int i=0; i < MAX_ENEMIES; i++)
	{
		if (enemies_list[i])
		{
			get_enemy_movement(enemies_list[i], direction);
			if (enemies_list[i]->pos.x <= 2)
				move_obj(enemies_list[i], 0, screen_size.x - 4);
			for (int j=0; j < MAX_ENEMIES; j++)
			{
				int security = 0;
				while (j != i && security < 5 && enemies_list[j] && enemies_list[i]->pos.x + direction->x == enemies_list[j]->pos.x
						   && enemies_list[i]->pos.y + direction->y == enemies_list[j]->pos.y)
				{
					get_enemy_movement(enemies_list[i], direction);
					security++;
				}
				if (security == 5)
				{
					direction->y = 0;
					direction->x = -1;
					break;
				}
			}
				
			move_obj(enemies_list[i], direction->y, direction->x);
			if (enemies_list[i]->pos.y == screen_size.y - 1)
				return count;
			if ((unsigned int)random_range(0, 100) <= ENEMY_ATTACK_PROBABILITY && enemies_list[i]->is_passive == false)
			{
				int	j = 0;
				while (j < MAX_BULLETS && bullet_list[j])
					j++;
				if (j < MAX_BULLETS && Player->ammunition > 0 && (Player->last_fire + (TICKRATE / Player->fire_rate) <= tick))
				{
					bullet_list[j] = new bullet(5, enemies_list[i]->pos, enemies_list[i]->direction, enemies_list[i]);
					bullet_list[j]->character = '-';
				}
			}
			if (!enemies_list[i]->is_passive)
				count++;
		}
	}
	if (direction->y == 1)
		direction->y = 0;
	return count;
}

void	delete_object(object **object)
{
	(*object)->character = ' ';
	move_obj((*object), 0, 0);
	delete (*object);
	(*object) = NULL;
}

void	move_bullets(spaceship *Player, bullet *bullet_list[MAX_BULLETS], spaceship *enemies_list[MAX_ENEMIES])
{
	int	i = 0;
	while (i < MAX_BULLETS)
	{
		if (bullet_list[i])
		{
			if ((bullet_list[i]->pos.x <= 1 && bullet_list[i]->direction.x < 0) || (bullet_list[i]->pos.x >= screen_size.x - 2 && bullet_list[i]->direction.x > 0) ||
				(bullet_list[i]->pos.y <= 1 && bullet_list[i]->direction.y < 0) || (bullet_list[i]->pos.y >= screen_size.y - 2 && bullet_list[i]->direction.y > 0))
				delete_object((object **)(&bullet_list[i]));
			else if (bullet_list[i]->pos.x == Player->pos.x && bullet_list[i]->pos.y == Player->pos.y)
			{
				Player->health -= bullet_list[i]->damage;
				delete_object((object **)(&bullet_list[i]));
				draw_health(Player->health);
			}	
			else
			{
				for (int j=0; j<MAX_ENEMIES; j++)
				{
					if (!enemies_list[j] || enemies_list[j]->is_passive) continue;
					if (enemies_list[j]->pos.x == bullet_list[i]->pos.x + bullet_list[i]->direction.x && enemies_list[j]->pos.y == bullet_list[i]->pos.y + bullet_list[i]->direction.y)
					{
						enemies_list[j]->health -= bullet_list[i]->damage;
						if (enemies_list[j]->health <= 0)
						{
							Player->score += enemies_list[j]->score;
							delete_object((object **)(&enemies_list[j]));
						}
						delete_object((object **)(&bullet_list[i]));
						break;
					}
				}
				if (bullet_list[i])
					move_obj(bullet_list[i], bullet_list[i]->direction.y, bullet_list[i]->direction.x);
			}
		}
		i++;
	}
}