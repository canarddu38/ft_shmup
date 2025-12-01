#ifndef MATHS_HPP
#define MATHS_HPP

#include <time.h>

#define ABS(nb) ((nb < 0)?-nb:nb)
#define MIN(a, b) ((a < b)?a:b)
#define MAX(a, b) ((a > b)?a:b)

int		random_range(int min, int max);
void	maths_init();

#endif