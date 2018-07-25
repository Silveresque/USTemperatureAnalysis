#ifndef GUMBEL_H
#define GUMBEL_H

typedef struct {
	double beta;
	double mu;
} gumbel;

gumbel *generate_max_gumbel(int *arr, int size);
gumbel *generate_min_gumbel(int *arr, int size);
double cdf_max_gumbel(gumbel *g, double val);
double cdf_min_gumbel(gumbel *g, double val);
double pctl_max_gumbel(gumbel *g, double pctl);
double pctl_min_gumbel(gumbel *g, double pctl);
gumbel *destroy_gumbel(gumbel *g);

#endif