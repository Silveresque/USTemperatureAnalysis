#ifndef MONTHLY_H
#define MONTHLY_H

#include "Auxiliary.h"
#include "Gumbel.h"
#include "Daily.h"

typedef struct {
	Normal *normals[3][12];
	Normal *stddev[3][12];
} MonthlyNormals;

int monthly_maximum(History *history, int month, int year, int var);
int monthly_minimum(History *history, int month, int year, int var);
RecordMaximum *record_maximum_monthly(History *history, int month, int start_year, int end_year, int var);
RecordMinimum *record_minimum_monthly(History *history, int month, int start_year, int end_year, int var);
int pass_monthly_quality_check(MonthlyNormals *monthly_normals, int month, int var, char standard);
double monthly_mean_minimum(History *history, int month, int start_year, int end_year, int var);
int pctl_monthly(History *history, int month, int start_year, int end_year, int var, int pctl);
int check_bounds_monthly(History *history, int month, int *start_year, int *end_year);
gumbel *dist_monthly_mean_maximum(History *history, int month, int var);
double prob_exceed_val_monthly(gumbel *g, double years, double val);
gumbel *dist_monthly_mean_minimum(History *history, int month, int var);
double prob_under_val_monthly(gumbel *g, double years, double val);
MonthlyNormals *create_monthly_normals(char *WBAN);
MonthlyNormals *destroy_monthly_normals(MonthlyNormals *monthly_normals);
double monthly_mean_maximum(History *history, int month, int start_year, int end_year, int var);
double monthly_avg_dewp(HourlyNormals *hourly_normals, int month, int var);
void print_prob_exeed_val_intervals_monthly(History *history, int month, int var, int val);
void print_prob_under_val_intervals_monthly(History *history, int month, int var, int val);
void simulate_monthly_maximum(History *history, int month, int var);
void simulate_monthly_minimum(History *history, int month, int var);

#endif