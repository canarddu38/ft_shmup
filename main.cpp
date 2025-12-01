#include "spaceship.hpp"
#include "boxes.hpp"
#include "config.hpp"
#include "score.hpp"
#include "menu.hpp"
#include "laser.hpp"
#include <signal.h>
#include <thread>

using namespace std;

spaceship *enemies_list[MAX_ENEMIES];
bullet    *bullet_list[MAX_BULLETS];

point  temp_max;
point  screen_size;

unsigned int    tick = 0;

void    move_obj(object *obj, int y, int x)
{
    mvaddch(obj->pos.y, obj->pos.x, ' ');
    obj->pos.y += y;
    obj->pos.x += x;
    obj->pos.x = MAX(1, obj->pos.x);
    obj->pos.y = MAX(1, obj->pos.y);
    obj->pos.x = MIN(obj->pos.x, screen_size.x - 2);
    obj->pos.y = MIN(obj->pos.y, screen_size.y - 2);
    mvaddch(obj->pos.y, obj->pos.x, obj->character);
}

void    intercept_signal(int sig)
{
    (void)sig;
    system("xset r rate 600 25");
    endwin();
    exit(0);
}

void    move_player(int pressedKey, spaceship *Player)
{
	char	bullets_types[TOTAL_AMMO_TYPES] = AMMO_TYPES;
	move_obj(Player, 0, 0);
	switch (pressedKey)
	{
		case KEY_UP:
			if (Player->pos.y > 1)
					move_obj(Player, -1, 0);
			break;
		case KEY_DOWN:
			if (Player->pos.y < (screen_size.y - 2))
				move_obj(Player, 1, 0);
			break;
		case KEY_RIGHT:
			if (Player->pos.x < (screen_size.x - 2))
					move_obj(Player, 0, 1);
			break;
		case KEY_LEFT:
			if (Player->pos.x > 1)
				move_obj(Player, 0, -1);
			break;
		case KEY_SPACE:
			int	i = 0;
			while (i < MAX_BULLETS && bullet_list[i])
				i++;
			if (i < MAX_BULLETS && Player->ammunition > 0 && (Player->last_fire + (TICKRATE / Player->fire_rate) <= tick))
			{
				bullet_list[i] = new bullet(5 * (MIN(Player->shoot_power, TOTAL_AMMO_TYPES - 1) + 1), Player->pos, Player->direction, Player);
				bullet_list[i]->character = bullets_types[Player->shoot_power];
				Player->ammunition--;
				draw_magazine(Player->ammunition);
				Player->last_fire = tick;
			}
			break;
	}
}

void    resize_box(spaceship *Player)
{
    if (temp_max.x != screen_size.x || temp_max.y != screen_size.y)
        {
            while (temp_max.y < screen_size.y)
            {
                move(temp_max.y - 1, 0);
                clrtoeol();
                temp_max.y++;
            }

            if (temp_max.x < screen_size.x)
            {
                for (int i = 0; i < screen_size.y; i++)
                {
                    move(i, temp_max.x - 1);
                    clrtoeol();
                }
            }
            draw_box(0, 0, screen_size.y - 1 , screen_size.x - 1);
            for (int i = screen_size.y; i < screen_size.y + BOX_INFOS_HEIGHT; i++)
            {
                move(i, 0);
                clrtoeol();
            }
            draw_box_infos(Player);
        }
}

struct star {
    int x, y;
};

star stars[MAX_STARS];
int star_count = 0;

void init_stars() {
    star_count = (int)(screen_size.x * screen_size.y * STAR_DENSITY);
    if (star_count < 1 && STAR_DENSITY > 0.0f) star_count = 1;
    if (star_count > MAX_STARS) star_count = MAX_STARS;
    for (int i = 0; i < star_count; ++i) {
        stars[i].x = random_range(1, screen_size.x - 2);
        stars[i].y = random_range(1, screen_size.y - 2);
    }
}

void update_stars() {
    static int scroll_tick = 0;
    scroll_tick++;
    if (scroll_tick < STAR_SCROLL_SPEED) return;
    scroll_tick = 0;
    for (int i = 0; i < star_count; ++i) {
        mvaddch(stars[i].y, stars[i].x, ' ');
        stars[i].x--;
        if (stars[i].x < 1) {
            stars[i].x = screen_size.x - 2;
            stars[i].y = random_range(1, screen_size.y - 2);
        }
    }
}

bool is_enemy_at(int x, int y) {
    for (int i = 0; i < MAX_ENEMIES; ++i) {
        if (enemies_list[i] && enemies_list[i]->pos.x == x && enemies_list[i]->pos.y == y)
            return true;
    }
    return false;
}

void draw_stars() {
    attron(COLOR_PAIR(STAR_COLOR));
    for (int i = 0; i < star_count; ++i) {
        if (!is_enemy_at(stars[i].x, stars[i].y)) {
            mvaddch(stars[i].y, stars[i].x, STAR_CHAR);
        }
    }
    attroff(COLOR_PAIR(STAR_COLOR));
}

int main(int ac, char **av)
{
	(void)av;
	if (ac == 1)
	{
		system("gnome-terminal --maximize -- bash -c \"./ft_shmup 1\"");
		exit(0);
	}
	this_thread::sleep_for(chrono::milliseconds(500));
    signal(SIGHUP, intercept_signal);
    signal(SIGTERM, intercept_signal);
    signal(SIGINT, intercept_signal);

    initscr();

    start_color();
    init_color(COLOR_GRAY, 500, 500, 500);
    init_color(STAR_GRAY_COLOR, 250, 250, 360);
    init_pair(1, COLOR_WHITE, COLOR_RED);
    init_pair(2, COLOR_GRAY, COLOR_BLACK);
    init_pair(STAR_COLOR, STAR_GRAY_COLOR, COLOR_BLACK);
    init_pair(BLUE_ANNOUCE_LASER_COLOR, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(BLUE_FIRE_LASER_COLOR, COLOR_MAGENTA, COLOR_MAGENTA);

	keypad(stdscr, TRUE);
	noecho();
	curs_set(0);
	timeout(1000 / TICKRATE);

	// init rng
	maths_init();

    menu();

    system("xset r rate 1 25");

    getmaxyx(stdscr, screen_size.y, screen_size.x);
    if (screen_size.y >= BOX_INFOS_HEIGHT * 2)
        screen_size.y -= BOX_INFOS_HEIGHT;
    init_stars();

    int     remaining_enemies = 5;
	int		remaining_enemies_overflow = 0;
    point   enemies_dir = {1, 0};
    load_enemies(enemies_list, remaining_enemies, screen_size.x - 2, 4);
    memset(bullet_list, 0, MAX_BULLETS);

    spaceship *Player = new spaceship(screen_size.y / 2, 2, 0, 0, 5, '>', 100, 0.5 * TICKRATE);

    draw_box(0, 0, screen_size.y - 1 , screen_size.x - 1);
    draw_box_infos(Player);

    laser lasers[LASER_NUMBER] = {

		// ---- TIER 1 ----
		{10, 12, 13, random_range(5, screen_size.x - 5), 'v'},
		{10, 12, 13, random_range(5, screen_size.x - 5), 'v'},

		// ---- TIER 2 ----
		{30, 32, 33, random_range(5, screen_size.x - 5), 'v'},
		{30, 32, 33, random_range(5, screen_size.x - 5), 'v'},
		{30, 32, 33, random_range(5, screen_size.x - 5), 'v'},
		{30, 32, 33, random_range(5, screen_size.x - 5), 'v'},

		// ---- TIER 3 ----
		{60, 61, 63, random_range(5, screen_size.x - 5), 'v'},
		{60, 61, 63, random_range(5, screen_size.x - 5), 'v'},
		{60, 61, 63, random_range(5, screen_size.x - 5), 'v'},
		{60, 61, 63, random_range(5, screen_size.x - 5), 'v'},
		{60, 61, 63, random_range(5, screen_size.x - 5), 'v'},
		{60, 61, 63, random_range(5, screen_size.x - 5), 'v'},

		// ---- WAVE 4 (75–76–78) - 6 lasers ----
		{75, 76, 78, random_range(5, screen_size.x - 5), 'v'},
		{75, 76, 78, random_range(5, screen_size.x - 5), 'v'},
		{75, 76, 78, random_range(5, screen_size.x - 5), 'v'},
		{75, 76, 78, random_range(5, screen_size.x - 5), 'v'},
		{75, 76, 78, random_range(5, screen_size.x - 5), 'v'},
		{75, 76, 78, random_range(5, screen_size.x - 5), 'v'},

		// ---- WAVE 5 (90–91–93) - 7 lasers ----
		{90, 91, 93, random_range(5, screen_size.x - 5), 'v'},
		{90, 91, 93, random_range(5, screen_size.x - 5), 'v'},
		{90, 91, 93, random_range(5, screen_size.x - 5), 'v'},
		{90, 91, 93, random_range(5, screen_size.x - 5), 'v'},
		{90, 91, 93, random_range(5, screen_size.x - 5), 'v'},
		{90, 91, 93, random_range(5, screen_size.x - 5), 'v'},
		{90, 91, 93, random_range(5, screen_size.x - 5), 'v'},

		// ---- WAVE 6 (105–106–108) - 8 lasers ----
		{105, 106, 108, random_range(5, screen_size.x - 5), 'v'},
		{105, 106, 108, random_range(5, screen_size.x - 5), 'v'},
		{105, 106, 108, random_range(5, screen_size.x - 5), 'v'},
		{105, 106, 108, random_range(5, screen_size.x - 5), 'v'},
		{105, 106, 108, random_range(5, screen_size.x - 5), 'v'},
		{105, 106, 108, random_range(5, screen_size.x - 5), 'v'},
		{105, 106, 108, random_range(5, screen_size.x - 5), 'v'},
		{105, 106, 108, random_range(5, screen_size.x - 5), 'v'},

		// ---- WAVE 7 (120–121–123) - 9 lasers ----
		{120, 121, 123, random_range(5, screen_size.x - 5), 'v'},
		{120, 121, 123, random_range(5, screen_size.x - 5), 'v'},
		{120, 121, 123, random_range(5, screen_size.x - 5), 'v'},
		{120, 121, 123, random_range(5, screen_size.x - 5), 'v'},
		{120, 121, 123, random_range(5, screen_size.x - 5), 'v'},
		{120, 121, 123, random_range(5, screen_size.x - 5), 'v'},
		{120, 121, 123, random_range(5, screen_size.x - 5), 'v'},
		{120, 121, 123, random_range(5, screen_size.x - 5), 'v'},
		{120, 121, 123, random_range(5, screen_size.x - 5), 'v'},

		// ---- WAVE 8 (135–136–138) - 10 lasers ----
		{135, 136, 138, random_range(5, screen_size.x - 5), 'v'},
		{135, 136, 138, random_range(5, screen_size.x - 5), 'v'},
		{135, 136, 138, random_range(5, screen_size.x - 5), 'v'},
		{135, 136, 138, random_range(5, screen_size.x - 5), 'v'},
		{135, 136, 138, random_range(5, screen_size.x - 5), 'v'},
		{135, 136, 138, random_range(5, screen_size.x - 5), 'v'},
		{135, 136, 138, random_range(5, screen_size.x - 5), 'v'},
		{135, 136, 138, random_range(5, screen_size.x - 5), 'v'},
		{135, 136, 138, random_range(5, screen_size.x - 5), 'v'},
		{135, 136, 138, random_range(5, screen_size.x - 5), 'v'},

		// ---- WAVE 9 (150–151–153) - 6 lasers ----
		{150, 151, 153, random_range(5, screen_size.x - 5), 'v'},
		{150, 151, 153, random_range(5, screen_size.x - 5), 'v'},
		{150, 151, 153, random_range(5, screen_size.x - 5), 'v'},
		{150, 151, 153, random_range(5, screen_size.x - 5), 'v'},
		{150, 151, 153, random_range(5, screen_size.x - 5), 'v'},
		{150, 151, 153, random_range(5, screen_size.x - 5), 'v'},

		// ---- WAVE 10 (165–166–168) - 7 lasers ----
		{165, 166, 168, random_range(5, screen_size.x - 5), 'v'},
		{165, 166, 168, random_range(5, screen_size.x - 5), 'v'},
		{165, 166, 168, random_range(5, screen_size.x - 5), 'v'},
		{165, 166, 168, random_range(5, screen_size.x - 5), 'v'},
		{165, 166, 168, random_range(5, screen_size.x - 5), 'v'},
		{165, 166, 168, random_range(5, screen_size.x - 5), 'v'},
		{165, 166, 168, random_range(5, screen_size.x - 5), 'v'},

		// ---- WAVE 11 (180–181–183) - 8 lasers ----
		{180, 181, 183, random_range(5, screen_size.x - 5), 'v'},
		{180, 181, 183, random_range(5, screen_size.x - 5), 'v'},
		{180, 181, 183, random_range(5, screen_size.x - 5), 'v'},
		{180, 181, 183, random_range(5, screen_size.x - 5), 'v'},
		{180, 181, 183, random_range(5, screen_size.x - 5), 'v'},
		{180, 181, 183, random_range(5, screen_size.x - 5), 'v'},
		{180, 181, 183, random_range(5, screen_size.x - 5), 'v'},
		{180, 181, 183, random_range(5, screen_size.x - 5), 'v'},

		// ---- WAVE 12 (195–196–198) - 9 lasers ----
		{195, 196, 198, random_range(5, screen_size.x - 5), 'v'},
		{195, 196, 198, random_range(5, screen_size.x - 5), 'v'},
		{195, 196, 198, random_range(5, screen_size.x - 5), 'v'},
		{195, 196, 198, random_range(5, screen_size.x - 5), 'v'},
		{195, 196, 198, random_range(5, screen_size.x - 5), 'v'},
		{195, 196, 198, random_range(5, screen_size.x - 5), 'v'},
		{195, 196, 198, random_range(5, screen_size.x - 5), 'v'},
		{195, 196, 198, random_range(5, screen_size.x - 5), 'v'},
		{195, 196, 198, random_range(5, screen_size.x - 5), 'v'},

		// ---- WAVE 13 (210–211–213) - 10 lasers ----
		{210, 211, 213, random_range(5, screen_size.x - 5), 'v'},
		{210, 211, 213, random_range(5, screen_size.x - 5), 'v'},
		{210, 211, 213, random_range(5, screen_size.x - 5), 'v'},
		{210, 211, 213, random_range(5, screen_size.x - 5), 'v'},
		{210, 211, 213, random_range(5, screen_size.x - 5), 'v'},
		{210, 211, 213, random_range(5, screen_size.x - 5), 'v'},
		{210, 211, 213, random_range(5, screen_size.x - 5), 'v'},
		{210, 211, 213, random_range(5, screen_size.x - 5), 'v'},
		{210, 211, 213, random_range(5, screen_size.x - 5), 'v'},
		{210, 211, 213, random_range(5, screen_size.x - 5), 'v'},

		// ---- WAVE 14 (225–226–228) - 6 lasers ----
		{225, 226, 228, random_range(5, screen_size.x - 5), 'v'},
		{225, 226, 228, random_range(5, screen_size.x - 5), 'v'},
		{225, 226, 228, random_range(5, screen_size.x - 5), 'v'},
		{225, 226, 228, random_range(5, screen_size.x - 5), 'v'},
		{225, 226, 228, random_range(5, screen_size.x - 5), 'v'},
		{225, 226, 228, random_range(5, screen_size.x - 5), 'v'},

		// ---- WAVE 15 (240–241–243) - 7 lasers ----
		{240, 241, 243, random_range(5, screen_size.x - 5), 'v'},
		{240, 241, 243, random_range(5, screen_size.x - 5), 'v'},
		{240, 241, 243, random_range(5, screen_size.x - 5), 'v'},
		{240, 241, 243, random_range(5, screen_size.x - 5), 'v'},
		{240, 241, 243, random_range(5, screen_size.x - 5), 'v'},
		{240, 241, 243, random_range(5, screen_size.x - 5), 'v'},
		{240, 241, 243, random_range(5, screen_size.x - 5), 'v'},

		// ---- WAVE 16 (255–256–258) - 8 lasers ----
		{255, 256, 258, random_range(5, screen_size.x - 5), 'v'},
		{255, 256, 258, random_range(5, screen_size.x - 5), 'v'},
		{255, 256, 258, random_range(5, screen_size.x - 5), 'v'},
		{255, 256, 258, random_range(5, screen_size.x - 5), 'v'},
		{255, 256, 258, random_range(5, screen_size.x - 5), 'v'},
		{255, 256, 258, random_range(5, screen_size.x - 5), 'v'},
		{255, 256, 258, random_range(5, screen_size.x - 5), 'v'},
		{255, 256, 258, random_range(5, screen_size.x - 5), 'v'},

		// ---- WAVE 17 (270–271–273) - 9 lasers ----
		{270, 271, 273, random_range(5, screen_size.x - 5), 'v'},
		{270, 271, 273, random_range(5, screen_size.x - 5), 'v'},
		{270, 271, 273, random_range(5, screen_size.x - 5), 'v'},
		{270, 271, 273, random_range(5, screen_size.x - 5), 'v'},
		{270, 271, 273, random_range(5, screen_size.x - 5), 'v'},
		{270, 271, 273, random_range(5, screen_size.x - 5), 'v'},
		{270, 271, 273, random_range(5, screen_size.x - 5), 'v'},
		{270, 271, 273, random_range(5, screen_size.x - 5), 'v'},
		{270, 271, 273, random_range(5, screen_size.x - 5), 'v'},

		// ---- WAVE 18 (285–286–288) - 10 lasers ----
		{285, 286, 288, random_range(5, screen_size.x - 5), 'v'},
		{285, 286, 288, random_range(5, screen_size.x - 5), 'v'},
		{285, 286, 288, random_range(5, screen_size.x - 5), 'v'},
		{285, 286, 288, random_range(5, screen_size.x - 5), 'v'},
		{285, 286, 288, random_range(5, screen_size.x - 5), 'v'},
		{285, 286, 288, random_range(5, screen_size.x - 5), 'v'},
		{285, 286, 288, random_range(5, screen_size.x - 5), 'v'},
		{285, 286, 288, random_range(5, screen_size.x - 5), 'v'},
		{285, 286, 288, random_range(5, screen_size.x - 5), 'v'},
		{285, 286, 288, random_range(5, screen_size.x - 5), 'v'},

		// ---- WAVE 19 (300–301–303) - 11 lasers ----
		{300, 301, 303, random_range(5, screen_size.x - 5), 'v'},
		{300, 301, 303, random_range(5, screen_size.x - 5), 'v'},
		{300, 301, 303, random_range(5, screen_size.x - 5), 'v'},
		{300, 301, 303, random_range(5, screen_size.x - 5), 'v'},
		{300, 301, 303, random_range(5, screen_size.x - 5), 'v'},
		{300, 301, 303, random_range(5, screen_size.x - 5), 'v'},
		{300, 301, 303, random_range(5, screen_size.x - 5), 'v'},
		{300, 301, 303, random_range(5, screen_size.x - 5), 'v'},
		{300, 301, 303, random_range(5, screen_size.x - 5), 'v'},
		{300, 301, 303, random_range(5, screen_size.x - 5), 'v'},
		{300, 301, 303, random_range(5, screen_size.x - 5), 'v'},

		// ---- WAVE 20 (315–316–318) - 12 lasers ----
		{315, 316, 318, random_range(5, screen_size.x - 5), 'v'},
		{315, 316, 318, random_range(5, screen_size.x - 5), 'v'},
		{315, 316, 318, random_range(5, screen_size.x - 5), 'v'},
		{315, 316, 318, random_range(5, screen_size.x - 5), 'v'},
		{315, 316, 318, random_range(5, screen_size.x - 5), 'v'},
		{315, 316, 318, random_range(5, screen_size.x - 5), 'v'},
		{315, 316, 318, random_range(5, screen_size.x - 5), 'v'},
		{315, 316, 318, random_range(5, screen_size.x - 5), 'v'},
		{315, 316, 318, random_range(5, screen_size.x - 5), 'v'},
		{315, 316, 318, random_range(5, screen_size.x - 5), 'v'},
		{315, 316, 318, random_range(5, screen_size.x - 5), 'v'},
		{315, 316, 318, random_range(5, screen_size.x - 5), 'v'},

		// ---- WAVE 21 (330–331–333) - 12 lasers ----
		{330, 331, 333, random_range(5, screen_size.x - 5), 'v'},
		{330, 331, 333, random_range(5, screen_size.x - 5), 'v'},
		{330, 331, 333, random_range(5, screen_size.x - 5), 'v'},
		{330, 331, 333, random_range(5, screen_size.x - 5), 'v'},
		{330, 331, 333, random_range(5, screen_size.x - 5), 'v'},
		{330, 331, 333, random_range(5, screen_size.x - 5), 'v'},
		{330, 331, 333, random_range(5, screen_size.x - 5), 'v'},
		{330, 331, 333, random_range(5, screen_size.x - 5), 'v'},
		{330, 331, 333, random_range(5, screen_size.x - 5), 'v'},
		{330, 331, 333, random_range(5, screen_size.x - 5), 'v'},
		{330, 331, 333, random_range(5, screen_size.x - 5), 'v'},
		{330, 331, 333, random_range(5, screen_size.x - 5), 'v'},

		// ---- WAVE 22 (345–346–348) - 12 lasers ----
		{345, 346, 348, random_range(5, screen_size.x - 5), 'v'},
		{345, 346, 348, random_range(5, screen_size.x - 5), 'v'},
		{345, 346, 348, random_range(5, screen_size.x - 5), 'v'},
		{345, 346, 348, random_range(5, screen_size.x - 5), 'v'},
		{345, 346, 348, random_range(5, screen_size.x - 5), 'v'},
		{345, 346, 348, random_range(5, screen_size.x - 5), 'v'},
		{345, 346, 348, random_range(5, screen_size.x - 5), 'v'},
		{345, 346, 348, random_range(5, screen_size.x - 5), 'v'},
		{345, 346, 348, random_range(5, screen_size.x - 5), 'v'},
		{345, 346, 348, random_range(5, screen_size.x - 5), 'v'},
		{345, 346, 348, random_range(5, screen_size.x - 5), 'v'},
		{345, 346, 348, random_range(5, screen_size.x - 5), 'v'},

		// ---- WAVE 23 (360–361–363) - 12 lasers ----
		{360, 361, 363, random_range(5, screen_size.x - 5), 'v'},
		{360, 361, 363, random_range(5, screen_size.x - 5), 'v'},
		{360, 361, 363, random_range(5, screen_size.x - 5), 'v'},
		{360, 361, 363, random_range(5, screen_size.x - 5), 'v'},
		{360, 361, 363, random_range(5, screen_size.x - 5), 'v'},
		{360, 361, 363, random_range(5, screen_size.x - 5), 'v'},
		{360, 361, 363, random_range(5, screen_size.x - 5), 'v'},
		{360, 361, 363, random_range(5, screen_size.x - 5), 'v'},
		{360, 361, 363, random_range(5, screen_size.x - 5), 'v'},
		{360, 361, 363, random_range(5, screen_size.x - 5), 'v'},
		{360, 361, 363, random_range(5, screen_size.x - 5), 'v'},
		{360, 361, 363, random_range(5, screen_size.x - 5), 'v'},

		// ---- WAVE 24 (375–376–378) - 12 lasers ----
		{375, 376, 378, random_range(5, screen_size.x - 5), 'v'},
		{375, 376, 378, random_range(5, screen_size.x - 5), 'v'},
		{375, 376, 378, random_range(5, screen_size.x - 5), 'v'},
		{375, 376, 378, random_range(5, screen_size.x - 5), 'v'},
		{375, 376, 378, random_range(5, screen_size.x - 5), 'v'},
		{375, 376, 378, random_range(5, screen_size.x - 5), 'v'},
		{375, 376, 378, random_range(5, screen_size.x - 5), 'v'},
		{375, 376, 378, random_range(5, screen_size.x - 5), 'v'},
		{375, 376, 378, random_range(5, screen_size.x - 5), 'v'},
		{375, 376, 378, random_range(5, screen_size.x - 5), 'v'},
		{375, 376, 378, random_range(5, screen_size.x - 5), 'v'},
		{375, 376, 378, random_range(5, screen_size.x - 5), 'v'},

		// ---- WAVE 25 (390–391–393) - 12 lasers ----
		{390, 391, 393, random_range(5, screen_size.x - 5), 'v'},
		{390, 391, 393, random_range(5, screen_size.x - 5), 'v'},
		{390, 391, 393, random_range(5, screen_size.x - 5), 'v'},
		{390, 391, 393, random_range(5, screen_size.x - 5), 'v'},
		{390, 391, 393, random_range(5, screen_size.x - 5), 'v'},
		{390, 391, 393, random_range(5, screen_size.x - 5), 'v'},
		{390, 391, 393, random_range(5, screen_size.x - 5), 'v'},
		{390, 391, 393, random_range(5, screen_size.x - 5), 'v'},
		{390, 391, 393, random_range(5, screen_size.x - 5), 'v'},
		{390, 391, 393, random_range(5, screen_size.x - 5), 'v'},
		{390, 391, 393, random_range(5, screen_size.x - 5), 'v'},
		{390, 391, 393, random_range(5, screen_size.x - 5), 'v'},

		// ---- WAVE 26 (405–406–408) - 12 lasers ----
		{405, 406, 408, random_range(5, screen_size.x - 5), 'v'},
		{405, 406, 408, random_range(5, screen_size.x - 5), 'v'},
		{405, 406, 408, random_range(5, screen_size.x - 5), 'v'},
		{405, 406, 408, random_range(5, screen_size.x - 5), 'v'},
		{405, 406, 408, random_range(5, screen_size.x - 5), 'v'},
		{405, 406, 408, random_range(5, screen_size.x - 5), 'v'},
		{405, 406, 408, random_range(5, screen_size.x - 5), 'v'},
		{405, 406, 408, random_range(5, screen_size.x - 5), 'v'},
		{405, 406, 408, random_range(5, screen_size.x - 5), 'v'},
		{405, 406, 408, random_range(5, screen_size.x - 5), 'v'},
		{405, 406, 408, random_range(5, screen_size.x - 5), 'v'},
		{405, 406, 408, random_range(5, screen_size.x - 5), 'v'},

		// ---- WAVE 27 (420–421–423) - 12 lasers ----
		{420, 421, 423, random_range(5, screen_size.x - 5), 'v'},
		{420, 421, 423, random_range(5, screen_size.x - 5), 'v'},
		{420, 421, 423, random_range(5, screen_size.x - 5), 'v'},
		{420, 421, 423, random_range(5, screen_size.x - 5), 'v'},
		{420, 421, 423, random_range(5, screen_size.x - 5), 'v'},
		{420, 421, 423, random_range(5, screen_size.x - 5), 'v'},
		{420, 421, 423, random_range(5, screen_size.x - 5), 'v'},
		{420, 421, 423, random_range(5, screen_size.x - 5), 'v'},
		{420, 421, 423, random_range(5, screen_size.x - 5), 'v'},
		{420, 421, 423, random_range(5, screen_size.x - 5), 'v'},
		{420, 421, 423, random_range(5, screen_size.x - 5), 'v'},
		{420, 421, 423, random_range(5, screen_size.x - 5), 'v'},

		// ---- WAVE 28 (435–436–438) - 12 lasers ----
		{435, 436, 438, random_range(5, screen_size.x - 5), 'v'},
		{435, 436, 438, random_range(5, screen_size.x - 5), 'v'},
		{435, 436, 438, random_range(5, screen_size.x - 5), 'v'},
		{435, 436, 438, random_range(5, screen_size.x - 5), 'v'},
		{435, 436, 438, random_range(5, screen_size.x - 5), 'v'},
		{435, 436, 438, random_range(5, screen_size.x - 5), 'v'},
		{435, 436, 438, random_range(5, screen_size.x - 5), 'v'},
		{435, 436, 438, random_range(5, screen_size.x - 5), 'v'},
		{435, 436, 438, random_range(5, screen_size.x - 5), 'v'},
		{435, 436, 438, random_range(5, screen_size.x - 5), 'v'},
		{435, 436, 438, random_range(5, screen_size.x - 5), 'v'},
		{435, 436, 438, random_range(5, screen_size.x - 5), 'v'},

		// ---- WAVE 29 (450–451–453) - 12 lasers ----
		{450, 451, 453, random_range(5, screen_size.x - 5), 'v'},
		{450, 451, 453, random_range(5, screen_size.x - 5), 'v'},
		{450, 451, 453, random_range(5, screen_size.x - 5), 'v'},
		{450, 451, 453, random_range(5, screen_size.x - 5), 'v'},
		{450, 451, 453, random_range(5, screen_size.x - 5), 'v'},
		{450, 451, 453, random_range(5, screen_size.x - 5), 'v'},
		{450, 451, 453, random_range(5, screen_size.x - 5), 'v'},
		{450, 451, 453, random_range(5, screen_size.x - 5), 'v'},
		{450, 451, 453, random_range(5, screen_size.x - 5), 'v'},
		{450, 451, 453, random_range(5, screen_size.x - 5), 'v'},
		{450, 451, 453, random_range(5, screen_size.x - 5), 'v'},
		{450, 451, 453, random_range(5, screen_size.x - 5), 'v'},

		// ---- WAVE 30 (465–466–468) - 12 lasers ----
		{465, 466, 468, random_range(5, screen_size.x - 5), 'v'},
		{465, 466, 468, random_range(5, screen_size.x - 5), 'v'},
		{465, 466, 468, random_range(5, screen_size.x - 5), 'v'},
		{465, 466, 468, random_range(5, screen_size.x - 5), 'v'},
		{465, 466, 468, random_range(5, screen_size.x - 5), 'v'},
		{465, 466, 468, random_range(5, screen_size.x - 5), 'v'},
		{465, 466, 468, random_range(5, screen_size.x - 5), 'v'},
		{465, 466, 468, random_range(5, screen_size.x - 5), 'v'},
		{465, 466, 468, random_range(5, screen_size.x - 5), 'v'},
		{465, 466, 468, random_range(5, screen_size.x - 5), 'v'},
		{465, 466, 468, random_range(5, screen_size.x - 5), 'v'},
		{465, 466, 468, random_range(5, screen_size.x - 5), 'v'},

		// ---- WAVE 31 (480–481–483) - 12 lasers ----
		{480, 481, 483, random_range(5, screen_size.x - 5), 'v'},
		{480, 481, 483, random_range(5, screen_size.x - 5), 'v'},
		{480, 481, 483, random_range(5, screen_size.x - 5), 'v'},
		{480, 481, 483, random_range(5, screen_size.x - 5), 'v'},
		{480, 481, 483, random_range(5, screen_size.x - 5), 'v'},
		{480, 481, 483, random_range(5, screen_size.x - 5), 'v'},
		{480, 481, 483, random_range(5, screen_size.x - 5), 'v'},
		{480, 481, 483, random_range(5, screen_size.x - 5), 'v'},
		{480, 481, 483, random_range(5, screen_size.x - 5), 'v'},
		{480, 481, 483, random_range(5, screen_size.x - 5), 'v'},
		{480, 481, 483, random_range(5, screen_size.x - 5), 'v'},
		{480, 481, 483, random_range(5, screen_size.x - 5), 'v'},

		// ---- WAVE 32 (495–496–498) - 12 lasers ----
		{495, 496, 498, random_range(5, screen_size.x - 5), 'v'},
		{495, 496, 498, random_range(5, screen_size.x - 5), 'v'},
		{495, 496, 498, random_range(5, screen_size.x - 5), 'v'},
		{495, 496, 498, random_range(5, screen_size.x - 5), 'v'},
		{495, 496, 498, random_range(5, screen_size.x - 5), 'v'},
		{495, 496, 498, random_range(5, screen_size.x - 5), 'v'},
		{495, 496, 498, random_range(5, screen_size.x - 5), 'v'},
		{495, 496, 498, random_range(5, screen_size.x - 5), 'v'},
		{495, 496, 498, random_range(5, screen_size.x - 5), 'v'},
		{495, 496, 498, random_range(5, screen_size.x - 5), 'v'},
		{495, 496, 498, random_range(5, screen_size.x - 5), 'v'},
		{495, 496, 498, random_range(5, screen_size.x - 5), 'v'},

		// ---- WAVE 33 (510–511–513) - 12 lasers ----
		{510, 511, 513, random_range(5, screen_size.x - 5), 'v'},
		{510, 511, 513, random_range(5, screen_size.x - 5), 'v'},
		{510, 511, 513, random_range(5, screen_size.x - 5), 'v'},
		{510, 511, 513, random_range(5, screen_size.x - 5), 'v'},
		{510, 511, 513, random_range(5, screen_size.x - 5), 'v'},
		{510, 511, 513, random_range(5, screen_size.x - 5), 'v'},
		{510, 511, 513, random_range(5, screen_size.x - 5), 'v'},
		{510, 511, 513, random_range(5, screen_size.x - 5), 'v'},
		{510, 511, 513, random_range(5, screen_size.x - 5), 'v'},
		{510, 511, 513, random_range(5, screen_size.x - 5), 'v'},
		{510, 511, 513, random_range(5, screen_size.x - 5), 'v'},
		{510, 511, 513, random_range(5, screen_size.x - 5), 'v'},

		// ---- WAVE 34 (525–526–528) - 12 lasers ----
		{525, 526, 528, random_range(5, screen_size.x - 5), 'v'},
		{525, 526, 528, random_range(5, screen_size.x - 5), 'v'},
		{525, 526, 528, random_range(5, screen_size.x - 5), 'v'},
		{525, 526, 528, random_range(5, screen_size.x - 5), 'v'},
		{525, 526, 528, random_range(5, screen_size.x - 5), 'v'},
		{525, 526, 528, random_range(5, screen_size.x - 5), 'v'},
		{525, 526, 528, random_range(5, screen_size.x - 5), 'v'},
		{525, 526, 528, random_range(5, screen_size.x - 5), 'v'},
		{525, 526, 528, random_range(5, screen_size.x - 5), 'v'},
		{525, 526, 528, random_range(5, screen_size.x - 5), 'v'},
		{525, 526, 528, random_range(5, screen_size.x - 5), 'v'},
		{525, 526, 528, random_range(5, screen_size.x - 5), 'v'},

		// ---- WAVE 35 (540–541–543) - 12 lasers ----
		{540, 541, 543, random_range(5, screen_size.x - 5), 'v'},
		{540, 541, 543, random_range(5, screen_size.x - 5), 'v'},
		{540, 541, 543, random_range(5, screen_size.x - 5), 'v'},
		{540, 541, 543, random_range(5, screen_size.x - 5), 'v'},
		{540, 541, 543, random_range(5, screen_size.x - 5), 'v'},
		{540, 541, 543, random_range(5, screen_size.x - 5), 'v'},
		{540, 541, 543, random_range(5, screen_size.x - 5), 'v'},
		{540, 541, 543, random_range(5, screen_size.x - 5), 'v'},
		{540, 541, 543, random_range(5, screen_size.x - 5), 'v'},
		{540, 541, 543, random_range(5, screen_size.x - 5), 'v'},
		{540, 541, 543, random_range(5, screen_size.x - 5), 'v'},
		{540, 541, 543, random_range(5, screen_size.x - 5), 'v'},

		// ---- WAVE 36 (555–556–558) - 12 lasers ----
		{555, 556, 558, random_range(5, screen_size.x - 5), 'v'},
		{555, 556, 558, random_range(5, screen_size.x - 5), 'v'},
		{555, 556, 558, random_range(5, screen_size.x - 5), 'v'},
		{555, 556, 558, random_range(5, screen_size.x - 5), 'v'},
		{555, 556, 558, random_range(5, screen_size.x - 5), 'v'},
		{555, 556, 558, random_range(5, screen_size.x - 5), 'v'},
		{555, 556, 558, random_range(5, screen_size.x - 5), 'v'},
		{555, 556, 558, random_range(5, screen_size.x - 5), 'v'},
		{555, 556, 558, random_range(5, screen_size.x - 5), 'v'},
		{555, 556, 558, random_range(5, screen_size.x - 5), 'v'},
		{555, 556, 558, random_range(5, screen_size.x - 5), 'v'},
		{555, 556, 558, random_range(5, screen_size.x - 5), 'v'},

		// ---- WAVE 37 (570–571–573) - 12 lasers ----
		{570, 571, 573, random_range(5, screen_size.x - 5), 'v'},
		{570, 571, 573, random_range(5, screen_size.x - 5), 'v'},
		{570, 571, 573, random_range(5, screen_size.x - 5), 'v'},
		{570, 571, 573, random_range(5, screen_size.x - 5), 'v'},
		{570, 571, 573, random_range(5, screen_size.x - 5), 'v'},
		{570, 571, 573, random_range(5, screen_size.x - 5), 'v'},
		{570, 571, 573, random_range(5, screen_size.x - 5), 'v'},
		{570, 571, 573, random_range(5, screen_size.x - 5), 'v'},
		{570, 571, 573, random_range(5, screen_size.x - 5), 'v'},
		{570, 571, 573, random_range(5, screen_size.x - 5), 'v'},
		{570, 571, 573, random_range(5, screen_size.x - 5), 'v'},
		{570, 571, 573, random_range(5, screen_size.x - 5), 'v'},

		// ---- WAVE 38 (585–586–588) - 12 lasers ----
		{585, 586, 588, random_range(5, screen_size.x - 5), 'v'},
		{585, 586, 588, random_range(5, screen_size.x - 5), 'v'},
		{585, 586, 588, random_range(5, screen_size.x - 5), 'v'},
		{585, 586, 588, random_range(5, screen_size.x - 5), 'v'},
		{585, 586, 588, random_range(5, screen_size.x - 5), 'v'},
		{585, 586, 588, random_range(5, screen_size.x - 5), 'v'},
		{585, 586, 588, random_range(5, screen_size.x - 5), 'v'},
		{585, 586, 588, random_range(5, screen_size.x - 5), 'v'},
		{585, 586, 588, random_range(5, screen_size.x - 5), 'v'},
		{585, 586, 588, random_range(5, screen_size.x - 5), 'v'},
		{585, 586, 588, random_range(5, screen_size.x - 5), 'v'},
		{585, 586, 588, random_range(5, screen_size.x - 5), 'v'},

		// ---- WAVE 39 (600–601–603) - 12 lasers ----
		{600, 601, 603, random_range(5, screen_size.x - 5), 'v'},
		{600, 601, 603, random_range(5, screen_size.x - 5), 'v'},
		{600, 601, 603, random_range(5, screen_size.x - 5), 'v'},
		{600, 601, 603, random_range(5, screen_size.x - 5), 'v'},
		{600, 601, 603, random_range(5, screen_size.x - 5), 'v'},
		{600, 601, 603, random_range(5, screen_size.x - 5), 'v'},
		{600, 601, 603, random_range(5, screen_size.x - 5), 'v'},
		{600, 601, 603, random_range(5, screen_size.x - 5), 'v'},
		{600, 601, 603, random_range(5, screen_size.x - 5), 'v'},
		{600, 601, 603, random_range(5, screen_size.x - 5), 'v'},
		{600, 601, 603, random_range(5, screen_size.x - 5), 'v'},
		{600, 601, 603, random_range(5, screen_size.x - 5), 'v'},

		// ---- WAVE 40 (615–616–618) - 12 lasers ----
		{615, 616, 618, random_range(5, screen_size.x - 5), 'v'},
		{615, 616, 618, random_range(5, screen_size.x - 5), 'v'},
		{615, 616, 618, random_range(5, screen_size.x - 5), 'v'},
		{615, 616, 618, random_range(5, screen_size.x - 5), 'v'},
		{615, 616, 618, random_range(5, screen_size.x - 5), 'v'},
		{615, 616, 618, random_range(5, screen_size.x - 5), 'v'},
		{615, 616, 618, random_range(5, screen_size.x - 5), 'v'},
		{615, 616, 618, random_range(5, screen_size.x - 5), 'v'},
		{615, 616, 618, random_range(5, screen_size.x - 5), 'v'},
		{615, 616, 618, random_range(5, screen_size.x - 5), 'v'},
		{615, 616, 618, random_range(5, screen_size.x - 5), 'v'},
		{615, 616, 618, random_range(5, screen_size.x - 5), 'v'}
	};

    int next_announcment = get_next_announcment(lasers, tick);
    int next_start = get_next_start(lasers, tick);
    int next_end = get_next_end(lasers, tick);

    int pressedKey = getch();
    while (pressedKey != 27)
    {
        move_player(pressedKey, Player);
        temp_max.x = screen_size.x;
        temp_max.y = screen_size.y;
        getmaxyx(stdscr, screen_size.y, screen_size.x);
        if (screen_size.y >= BOX_INFOS_HEIGHT * 2)
            screen_size.y -= BOX_INFOS_HEIGHT;
        resize_box(Player);

		update_stars();
        draw_stars();
		refresh();

		if (!(tick % (TICKRATE / 3)))
		{
			remaining_enemies = move_enemies(tick, Player, enemies_list, bullet_list, &enemies_dir);
			// enemies spawn mechanics
			if (remaining_enemies > 0)
			{
				int to_display = MIN(remaining_enemies_overflow, screen_size.y);
				remaining_enemies_overflow = MAX(remaining_enemies_overflow - screen_size.y, 0);
				load_enemies(enemies_list, to_display, screen_size.x - 2, 4);
			}
		}

		if (random_range(0, 3000) <= POWERUP_SPAWN_PROBABILITY)
			spawn_powerup(enemies_list, screen_size.x - 2, 3);

		
		if (remaining_enemies <= 0)
		{
			Player->score_multiplier++;
			Player->wave++;
			draw_score(Player);
			remaining_enemies = Player->wave * 5;
			remaining_enemies_overflow = remaining_enemies - screen_size.y;
			load_enemies(enemies_list, MIN(remaining_enemies, screen_size.y), screen_size.x - 2, 4);
		}
		
		move_bullets(Player, bullet_list, enemies_list);

		for (int i=0; i < MAX_ENEMIES; i++) // handle collisions
		{
			if (enemies_list[i] &&   enemies_list[i]->pos.x == Player->pos.x && enemies_list[i]->pos.y == Player->pos.y)
			{
				if (enemies_list[i]->is_passive)
				{
					if (Player->shoot_power < TOTAL_AMMO_TYPES - 1)
						Player->shoot_power++;
					else
					{
						Player->health = MIN(100, Player->health + 10);
						draw_health(Player->health);
					}
				}
				else if (!enemies_list[i]->is_passive)
					Player->health = 0;
				enemies_list[i]->character = ' ';
				move_obj(enemies_list[i], 0, 0);
				delete_object((object **)&enemies_list[i]);
			}
		}
		
		if (!(tick % TICKRATE))
		{
			Player->time_survived++;
			if (Player->time_survived == TIME_TIER_1)
				Player->score_multiplier += MULTIPLIER_TIER_1;
			Player->score += Player->score_multiplier;
			draw_score(Player);
		}
		if (!(tick % Player->reload_delay) && (Player->last_fire + (TICKRATE / Player->fire_rate) <= tick))
		{
			if (Player->ammunition < MAX_AMMUNITION)
			{
				Player->ammunition++;
				draw_magazine(Player->ammunition);
			}
		}
		if (Player->time_survived >= next_announcment && Player->time_survived < next_start)
		{
			do_laser(lasers, next_announcment, announce_vertical_laser);
		}
		else if (Player->time_survived >= next_start && Player->time_survived < next_end)
		{
			do_laser(lasers, next_announcment, fire_vertical_laser);
			for (int i = 0; i < LASER_NUMBER; i++)
			{
				if (lasers[i].announcment == next_announcment)
				{
					if (lasers[i].type == 'v')
						is_in_vertical_laser(Player, lasers[i].pos, tick);
				}
			}
		}
		else if (Player->time_survived == next_end)
		{
			temp_max.x++;
			resize_box(Player);
			do_laser(lasers, next_announcment, clear_vertical_laser);
			next_announcment = get_next_announcment(lasers, tick);
			next_start = get_next_start(lasers, tick);
			next_end = get_next_end(lasers, Player->time_survived);
		}

		if (Player->health <= 0)
			break;

		refresh();
		pressedKey = getch();
		tick++;
	}
	system("xset r rate 600 25");
	for (int i=0; i<MAX_ENEMIES;i++)
		if (enemies_list[i]) delete enemies_list[i];
	for (int i=0; i<MAX_BULLETS;i++)
		if (bullet_list[i]) delete bullet_list[i];
	game_over(Player);
	endwin();
	return (0);
}