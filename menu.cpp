#include "boxes.hpp"
#include "menu.hpp"
#include <ncurses.h>

using namespace std;

int	get_max_strlen(string str)
{
	int len = 0;
	int temp_len = 0;

	for (int i = 0; str[i]; i++)
	{
		if (str[i] != '\n')
		{
			temp_len++;
			if (temp_len > len)
				len = temp_len;
		}
		else
		{
			temp_len = 0;
		}
	}
	return (len);
}

void	put_ascii_art(string str, int y, int x)
{
	int	max_strlen = get_max_strlen(str);
	int	index = 0;
	for (int i = 0; str[i]; i++)
	{
		if (str[i] != '\n')
		{
			mvaddch(y, x + index - max_strlen / 2, str[i]);
			index++;
		}
		else
		{
			index = 0;
			y++;
		}
	}
}

void	clear_selection(selection twinkle)
{
	mvaddch(twinkle.y + 2, twinkle.x - 3 - twinkle.max_strlen / 2, ' ');
	mvaddch(twinkle.y + 2, twinkle.x + 3 + twinkle.max_strlen / 2, ' ');
}

void	aff_selection(selection twinkle)
{
	if (twinkle.state)
	{
		attron(A_BOLD);
		mvaddch(twinkle.y + 2, twinkle.x - 3 - twinkle.max_strlen / 2, '?');
		mvaddch(twinkle.y + 2, twinkle.x + 3 + twinkle.max_strlen / 2, '<');
		attroff(A_BOLD);
	}
	else
		clear_selection(twinkle);
}

void	draw_death_infos(spaceship *Player, int y, int x)
{
	mvprintw(y - 8, x / 2 - 10, "%-15s : %.0f", "Score", Player->score);
	mvprintw(y - 7, x / 2 - 10, "%-15s : %d", "Time survived", Player->time_survived);
	mvprintw(y - 6, x / 2 - 10, "%-15s : %d", "Waves Defeated", Player->wave);
}

void	game_over(spaceship *Player)
{
	clear ();
	int		pressedKey;
	int		x;
	int		y;
	string	string_game_over = "   _________    __  _________  ____ _    ____________ \n  / ____/   |  /  |/  / ____/ / __ \\ |  / / ____/ __ \\\n / / __/ /| | / /|_/ / __/   / / / / | / / __/ / /_/ /\n/ /_/ / ___ |/ /  / / /___  / /_/ /| |/ / /___/ _, _/ \n\\____/_/  |_/_/  /_/_____/  \\____/ |___/_____/_/ |_|  \n";
	box(stdscr, 0, 0);
	getmaxyx(stdscr, y, x);
	put_ascii_art(string_game_over, y / 2 - 4, x / 2);
	put_ascii_art("press [ESC] to quit", y - 4, x / 2);
	put_ascii_art("press [Enter] to play again", y - 3, x / 2);
	draw_death_infos(Player, y, x);
	refresh();

	pressedKey = getch();
	while (1)
	{
		if (pressedKey == 27)
		{
			endwin();
			delete Player;
			exit(0);
		}
		else if (pressedKey == ENTER)
		{
			endwin();
			system("./ft_shmup");
			delete Player;
			exit(0);
		}
		pressedKey = getch();
	}
}

void	menu()
{
	int				y;
	int				x;
	unsigned int	tick = 0;
	int				prechoice = 0;
	int				pressedKey;
	string			start_game_string = "   ______________    ____  ______  _________    __  _________\n  / ___/_  __/   |  / __ \\/_  __/ / ____/   |  /  |/  / ____/\n  \\__ \\ / / / /| | / /_/ / / /   / / __/ /| | / /|_/ / __/   \n ___/ // / / ___ |/ _, _/ / /   / /_/ / ___ |/ /  / / /___   \n/____//_/ /_/  |_/_/ |_| /_/    \\____/_/  |_/_/  /_/_____/\n";

	keypad(stdscr, TRUE);
	noecho();
	curs_set(0);
	timeout(1000 / 60);

	getmaxyx(stdscr, y, x);
	
	int			start_game_y = y / 2;
	selection	twinkle = {x / 2 , start_game_y - 4, false, get_max_strlen(start_game_string), prechoice};
	
	draw_box(0, 0, y - 1, x - 1);
	put_ascii_art(start_game_string, start_game_y - 4, x / 2);
	put_ascii_art("press [Enter] to select", y - 3, x / 2);

	pressedKey = getch();
	while (1)
	{
		if (pressedKey == 27)
		{
			endwin();
			exit(0);
		}
		else if (pressedKey == ENTER)
		{
			clear();
			break;
		}
		if (prechoice != twinkle.prechoice)
			clear_selection(twinkle);
		else if (prechoice == 0)
			twinkle = {x / 2, start_game_y - 4, twinkle.state, get_max_strlen(start_game_string), prechoice};
		tick++;
		if (!(tick % 60))
		{
			if (twinkle.state)
				twinkle.state = false;
			else
				twinkle.state = true;
			aff_selection(twinkle);
		}
		pressedKey = getch();
	}
}