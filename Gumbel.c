#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include "Gumbel.h"
#include "Auxiliary.h"

#define PI 3.14159265358979323846264338327950288419716939937510
#define EULER 0.5772156649015328606065120900824024310421

// new max gumbel distribution
gumbel *generate_max_gumbel(int *arr, int size) {
	gumbel *g = calloc(1, sizeof(gumbel));
	if (!g) {
		printf("Ran out of memory!\n");
		exit(EXIT_FAILURE);
	}

	g->beta = stddev(arr, size);
	g->beta *= pow(6, 0.5);
	g->beta /= PI;

	g->mu = avg(arr, size) - g->beta * EULER;

	return g;
}

// new min gumbel distribution
gumbel *generate_min_gumbel(int *arr, int size) {
	gumbel *g = calloc(1, sizeof(gumbel));
	if (!g) {
		printf("Ran out of memory!\n");
		exit(EXIT_FAILURE);
	}

	g->beta = stddev(arr, size);
	g->beta *= pow(6, 0.5);
	g->beta /= PI;

	g->mu = avg(arr, size) + g->beta * EULER;

	return g;
}

// returns cdf of max gumbel evaluated at 'val'
double cdf_max_gumbel(gumbel *g, double val) {
	return exp(-1 * exp(-1 * (val - g->mu) / g->beta));
}

// returns cdf of min gumbel evaluated at 'val'
double cdf_min_gumbel(gumbel *g, double val) {
	return 1 - exp(-1 * exp((val - g->mu) / g->beta));
}

// returns value that corresponds to the given 'pctl' in max gumbel
double pctl_max_gumbel(gumbel *g, double pctl) {
	// pctl = exp(-1 * exp(-1 * (val - g->mu) / g->beta))
	// log(pctl) = -1 * exp(-1 * (val - g->mu) / g->beta)
	// -1 * log(pctl) = exp(-1 * (val - g->mu) / g->beta)
	// log(-1 * log(pctl)) = -1 * (val - g->mu) / g->beta
	// -1 * log(-1 * log(pctl)) = (val - g->mu) / g->beta
	// -1 * log(-1 * log(pctl)) * g->beta = val - g->mu
	// -1 * log(-1 * log(pctl)) * g->beta + g->mu = val
	return -1 * log(-1 * log(pctl)) * g->beta + g->mu;
}

// returns value that corresponds to the given 'pctl' in min gumbel
double pctl_min_gumbel(gumbel *g, double pctl) {
	// pct = 1 - exp(-1 * exp((val - g->mu) / g->beta))
	// pct - 1 = -1 * exp(-1 * exp((val - g->mu) / g->beta))
	// 1 - pctl = exp(-1 * exp((val - g->mu) / g->beta))
	// log(1 - pctl) = -1 * exp((val - g->mu) / g->beta)
	// -1 * log(1 - pctl) = exp((val - g->mu) / g->beta)
	// log(-1 * log(1 - pctl)) = (val - g->mu) / g->beta
	// g->beta * (log(-1 * log(1 - pctl))) = val - g->mu
	// g->beta * (log(-1 * log(1 - pctl))) + g->mu = val
	return g->beta * (log(-1 * log(1 - pctl))) + g->mu;
}

gumbel *destroy_gumbel(gumbel *g) {
	free(g);
	return NULL;
}