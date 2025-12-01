#include "maths.hpp"
#include <chrono>

uint64_t rng_state;

uint64_t splitmix64()
{
    uint64_t z = (rng_state += 0x9E3779B97F4A7C15ULL);
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
    return z ^ (z >> 31);
}


int random_range(int min, int max)
{
	int r = splitmix64();
	long long range = ABS(max - min);
    return min + ABS(r % range);
}

int	index_of(char c, char *str)
{
	int	i = 0;
	while (str[i] && str[i] != c)
		i++;
	if (str[i] == c)
		return i;
	return -1;
}

void	maths_init()
{
	rng_state = std::chrono::high_resolution_clock::now().time_since_epoch().count();
}