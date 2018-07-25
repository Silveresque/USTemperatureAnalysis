#ifndef YEARLY_H
#define YEARLY_H

#include "Monthly.h"
#include "Gumbel.h"

typedef struct {
	int monthly_max[2][12];
	int yearly_max[2];

	int monthly_min[2][12];
	int yearly_min[2];
} RecordExtremaTable;

typedef struct {
	double monthly_mean_max[2][12];
	double yearly_mean_max[2];

	double monthly_mean_min[2][12];
	double yearly_mean_min[2];
} MeanExtremaTable;

int yearly_maximum(History *history, int year, int var);
int yearly_minimum(History *history, int year, int var);
RecordMaximum *record_maximum_yearly(History *history, int start_year, int end_year, int var);
RecordMinimum *record_minimum_yearly(History *history, int start_year, int end_year, int var);
double yearly_mean_maximum(History *history, int start_year, int end_year, int var);
double yearly_mean_minimum(History *history, int start_year, int end_year, int var);
int check_bounds_yearly(History *history, int *start_year, int *end_year);
int pctl_yearly(History *history, int start_year, int end_year, int var, int pctl);
gumbel *dist_yearly_mean_maximum(History *history, int var);
double prob_exceed_val_yearly(gumbel *g, double years, double val);
gumbel *dist_yearly_mean_minimum(History *history, int var);
double prob_under_val_yearly(gumbel *g, double years, double val);
void print_prob_exeed_val_intervals_yearly(History *history, int var, int val);
void print_prob_under_val_intervals_yearly(History * history, int var, int val);
void simulate_yearly_maximum(History *history, int var);
void simulate_yearly_minimum(History *history, int var);
double yearly_normal(MonthlyNormals *monthly_normals, int var);
int pass_yearly_quality_check(MonthlyNormals *monthly_normals, int var, char standard);
double yearly_avg_dewp(HourlyNormals *hourly_normals, int var);
RecordExtremaTable *create_record_extrema_table(History *history);
RecordExtremaTable *destroy_record_extrema_table(RecordExtremaTable *record_extrema_table);

#endif