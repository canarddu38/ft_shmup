#ifndef MENU_HPP
#define MENU_HPP

typedef struct selection
{
	int		x;
	int		y;
	bool	state;
	int		max_strlen;
	int		prechoice;
}	selection;

void	menu();
void	game_over(spaceship *Player);

#endif