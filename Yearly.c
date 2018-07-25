#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "Auxiliary.h"
#include "MonthlyTempRecords.h"
#include <math.h>
#include <time.h>
#include <limits.h>
#include "Yearly.h"
#include "Daily.h"
#include "Calendar.h"
#include "Constants.h"

// helper function for record_maximum_yearly()
static void find_record_max(History *history, int end_year, int var, int index, int *max_value, int *max_year);
// helper function for record_minimum_yearly()
static void find_record_min(History *history, int end_year, int var, int index, int *min_value, int *min_year);
// helper function for yearly_mean_maximum()
static void calc_yearly_max_stats(History *history, int end_year, int var, int index, int *maximum_totals, int *counter);
// helper function for yearly_mean_minimum()
static void calc_yearly_min_stats(History *history, int end_year, int var, int index, int *minimum_totals, int *counter);
// helper functiosn for pctl_yearly()
static void get_start_index(History *history, int *index, int start_year);
static int *resize_temps_array(int *temps, int *capacity, int size);
static int *create_pctl_array(History *history, int end_year, int var, int index, int *size);
// helper functions for print_prob_exceed_vals_yearly()
static void print_exceed_val_label(int var, int val);
static void print_exceed_val_output(gumbel *g, int val);
static void print_exceed_expected_frequency(gumbel *g, int val);
// helper functions for print_prob_under_vals_yearly()
static void print_under_val_label(int var, int val);
static void print_under_val_output(gumbel *g, int val);
static void print_under_expected_frequency(gumbel *g, int val);
//helper functions for simulate_yearly_maximum() and simulate_yearly_minimum()
static void print_simulation_label(int extreme, int var);
static void print_simulated_value(gumbel *g, int extreme);
// helper functions for create_record_extrema_table()
static void fill_table_monthly(History *history, RecordExtremaTable *record_extrema_table, int i, int j);
static void fill_table_yearly(History *history, RecordExtremaTable *record_extrema_table, int i, int j);

// returns the yearly maximum temperature of any given year
int yearly_maximum(History *history, int year, int var) {
	if (!check_bounds_yearly(history, &year, &year))
		return UNAVAILABLE;

	int i, j, start_month = 1, index = get_month_index(history, year_month_of(year, start_month));
	int year_maximum = INT_MIN;

	while (index == -1)
		index = get_month_index(history, year_month_of(year, ++start_month));

	for (i = index; i < history->size && year_of_ym(history->monthly_records[i]->year_month) == year; i++) {
		for (j = 0; j < 31; j++) {
			int temp = history->monthly_records[i]->temps[var][j];
			if (!is_invalid(temp))
				year_maximum = max_int(temp, year_maximum);
		}
	}

	return (year_maximum != INT_MIN) ? year_maximum : UNDEFINED;
}

// returns the yearly mean minimum temperature of any given year
int yearly_minimum(History *history, int year, int var) {
	if (!check_bounds_yearly(history, &year, &year))
		return UNAVAILABLE;

	int i, j, start_month = 1, index = get_month_index(history, year_month_of(year, start_month));
	int year_minimum = INT_MAX;

	while (index == -1)
		index = get_month_index(history, year_month_of(year, ++start_month));

	for (i = index; i < history->size && year_of_ym(history->monthly_records[i]->year_month) == year; i++) {
		for (j = 0; j < 31; j++) {
			int temp = history->monthly_records[i]->temps[var][j];
			if (!is_invalid(temp))
				year_minimum = min_int(temp, year_minimum);
		}
	}

	return (year_minimum != INT_MAX) ? year_minimum : UNAVAILABLE;
}

static void find_record_max(History *history, int end_year, int var, int index, int *max_value, int *max_year) {
	int i;
	for (i = index; i < history->size && year_of_ym(history->monthly_records[i]->year_month) <= end_year; i += 12) {

		int yearly_max = yearly_maximum(history, year_of_ym(history->monthly_records[i]->year_month), var);

		if (!is_invalid(yearly_max)) {
			*max_year = (yearly_max > *max_value) ? year_of_ym(history->monthly_records[i]->year_month) : *max_year;
			*max_value = !is_invalid(*max_value) ? max_int(*max_value, yearly_max) : *max_value;
		}

	}
}

// returns a pointer ot a RecordMaximum instance containing the maximum value and its year of occurrence
RecordMaximum *record_maximum_yearly(History *history, int start_year, int end_year, int var) {
	if (!check_bounds_yearly(history, &start_year, &end_year))
		return NULL;

	RecordMaximum *record_maximum = calloc(1, sizeof(RecordMaximum));
	if (!record_maximum) {
		printf("Ran out of memory!\n");
		exit(EXIT_FAILURE);
	}

	int start_month = 1, index = get_month_index(history, year_month_of(start_year, start_month));
	int max_value = INT_MIN;
	int max_year = first_year(history);

	while (index == -1)
		index = get_month_index(history, year_month_of(start_year, ++start_month));

	find_record_max(history, end_year, var, index, &max_value, &max_year);

	record_maximum->value = (max_value != INT_MIN) ? max_value : UNDEFINED;
	record_maximum->year = max_year;

	return record_maximum;
}


static void find_record_min(History *history, int end_year, int var, int index, int *min_value, int *min_year) {
	int i;
	for (i = index; i < history->size && year_of_ym(history->monthly_records[i]->year_month) <= end_year; i += 12) {

		int yearly_min = yearly_minimum(history, year_of_ym(history->monthly_records[i]->year_month), var);

		if (!is_invalid(yearly_min)) {
			*min_year = (yearly_min < *min_value) ? year_of_ym(history->monthly_records[i]->year_month) : *min_year;
			*min_value = !is_invalid(*min_value) ? min_int(*min_value, yearly_min) : *min_value;
		}

	}
}

// returns a pointer ot a RecordMaximum instance containing the minimum value and its year of occurrence
RecordMinimum *record_minimum_yearly(History *history, int start_year, int end_year, int var) {
	if (!check_bounds_yearly(history, &start_year, &end_year))
		return NULL;

	RecordMinimum *record_minimum = calloc(1, sizeof(RecordMinimum));
	if (!record_minimum) {
		printf("Ran out of memory!\n");
		exit(EXIT_FAILURE);
	}

	int start_month = 1, index = get_month_index(history, year_month_of(start_year, start_month));
	int min_value = INT_MAX;
	int min_year = first_year(history);

	while (index == -1)
		index = get_month_index(history, year_month_of(start_year, ++start_month));

	find_record_min(history, end_year, var, index, &min_value, &min_year);

	record_minimum->value = (min_value != INT_MAX) ? min_value : UNDEFINED;
	record_minimum->year = min_year;

	return record_minimum;
}

static void calc_yearly_max_stats(History *history, int end_year, int var, int index, int *maximum_totals, int *counter) {
	int i;
	for (i = index; i < history->size && year_of_ym(history->monthly_records[i]->year_month) <= end_year; i += 12) {

		int year_maximum = yearly_maximum(history, year_of_ym(history->monthly_records[i]->year_month), var);

		if (!is_invalid(year_maximum)) {
			*maximum_totals += year_maximum;
			*counter += 1;
		}
	}
}

// returns the yearly mean maximum temperature for a given time period
double yearly_mean_maximum(History *history, int start_year, int end_year, int var) {

	if (!check_bounds_yearly(history, &start_year, &end_year))
		return UNAVAILABLE;

	int start_month = 1, index = get_month_index(history, year_month_of(start_year, start_month));
	int counter = 0;
	int maximum_totals = 0;

	while (index == -1)
		index = get_month_index(history, year_month_of(start_year, ++start_month));

	calc_yearly_max_stats(history, end_year, var, index, &maximum_totals, &counter);

	return ((double)maximum_totals) / ((double)counter);
}

static void calc_yearly_min_stats(History *history, int end_year, int var, int index, int *minimum_totals, int *counter) {
	int i;
	for (i = index; i < history->size && year_of_ym(history->monthly_records[i]->year_month) <= end_year; i += 12) {

		int year_minimum = yearly_minimum(history, year_of_ym(history->monthly_records[i]->year_month), var);

		if (!is_invalid(year_minimum)) {
			*minimum_totals += year_minimum;
			*counter += 1;
		}
	}
}

// returns the yearly mean maximum temperature for a given time period
double yearly_mean_minimum(History *history, int start_year, int end_year, int var) {
	if (!check_bounds_yearly(history, &start_year, &end_year))
		return UNAVAILABLE;

	int start_month = 1, index = get_month_index(history, year_month_of(start_year, start_month));
	int counter = 0;
	int minimum_totals = 0;

	while (index == -1)
		index = get_month_index(history, year_month_of(start_year, ++start_month));

	calc_yearly_min_stats(history, end_year, var, index, &minimum_totals, &counter);

	return ((double)minimum_totals) / ((double)counter);
}

// general purpose checking to make sure either a year or a range of years
// are valid to access in 'history'
// auto-adjusts start_year and end_year if out of range
int check_bounds_yearly(History *history, int *start_year, int *end_year) {
	if (*start_year > year_of_ym(history->end) || *end_year < year_of_ym(history->start))
		return 0;
	
	if (*start_year == *end_year)
		return 1;
	
	if (*start_year > *end_year)
		return 0;

	if (*start_year < year_of_ym(history->start))
		*start_year = year_of_ym(history->start);

	if (*end_year > year_of_ym(history->end))
		*end_year = year_of_ym(history->end);

	return 1;
}

static void get_start_index(History *history, int *index, int start_year) {
	int start_month = 1;
	*index = get_month_index(history, year_month_of(start_year, start_month));
	while (*index == -1)
		*index = get_month_index(history, year_month_of(start_year, ++start_month));
}

static int *resize_temps_array(int *temps, int *capacity, int size) {
	*capacity = *capacity * 2;
	int *transfer = realloc(temps, sizeof(int) * *capacity);

	if (!transfer) {
		printf("Out of memory!\n");
		exit(EXIT_FAILURE);
	}
	else
		return transfer;
}

static int *create_pctl_array(History *history, int end_year, int var, int index, int *size) {
	int i, j;
	int capacity = 31;
	int *temps = calloc(capacity, sizeof(int));
	if (!temps) {
		printf("Ran out of memory!\n");
		exit(EXIT_FAILURE);
	}

	for (i = index; i < history->size && year_of_ym(history->monthly_records[i]->year_month) <= end_year; i++) {
		for (j = 0; j < 31; j++) {

			int temp = history->monthly_records[i]->temps[var][j];
			if (!is_invalid(temp)) {
				temps[*size] = temp;
				*size += 1;
			}
			else
				continue;

			if (*size == capacity) 
				temps = resize_temps_array(temps, &capacity, *size);
		}
	}

	return temps;
}

// returns the temperature equal to the given percentile in the specified time period
int pctl_yearly(History *history, int start_year, int end_year, int var, int pctl) {
	if (!check_bounds_yearly(history, &start_year, &end_year))
		return UNAVAILABLE;

	if (pctl < 1 || pctl > 99)
		return UNAVAILABLE;

	int index;
	int capacity = 31, size = 0;
	double position;

	get_start_index(history, &index, start_year);

	int retval, *temps = create_pctl_array(history, end_year, var, index, &size);

	bucket_sort(temps, size);
	position = ((double)(pctl * size)) / ((double)100) - 1;
	retval = temps[(int)round(position)];

	free(temps);
	return retval;
}

// returns a pointer to a gumbel distribution for mean maximums
gumbel *dist_yearly_mean_maximum(History *history, int var) {
	int yearly_max[30];
	int i, capacity = 10, size = 0;

	for (i = 0; i < 30; i++) {
		int value = yearly_maximum(history, 1981 + i, var);
		if (value != UNAVAILABLE)
			yearly_max[size++] = value;
	}

	gumbel *g = generate_max_gumbel(yearly_max, size);

	return g;
}

/*returns the probability of at least one occurrence of a value greater than or equal to 'val'
*in a given amount of years combining gumbel and geometric distributions
*/
// cdf_max_gumbel(g, val) is equal to the probability of no occurrence in one year
// pow(cdf_max_gumbel(g, val), years) is equal to the probability of no occurrences in 'years' years
double prob_exceed_val_yearly(gumbel *g, double years, double val) {
	return 1 - pow(cdf_max_gumbel(g, val), years);
}

// gumbel distribution for mean minimums
gumbel *dist_yearly_mean_minimum(History *history, int var) {
	int yearly_min[30];
	int i, capacity = 10, size = 0;

	for (i = 0; i < 30; i++) {
		int value = yearly_minimum(history, 1981 + i, var);
		if (value != UNAVAILABLE)
			yearly_min[size++] = value;
	}

	gumbel *g = generate_min_gumbel(yearly_min, size);

	return g;
}

// same as prob_exeed_val but for mean minimums
double prob_under_val_yearly(gumbel *g, double years, double val) {
	return 1 - pow(1 - cdf_min_gumbel(g, val), years);
}

static void print_exceed_val_label(int var, int val) {
	printf("\n");
	if (!var)
		printf("High temperature at least ");
	else
		printf("Low temperature at least ");

	printf("%d%cF\n", val, 248);
	printf("Probability of occurrence in Whole Year\n");
}

static void print_exceed_val_output(gumbel *g, int val) {
	double real_val = (double)val - 0.5;
	printf("  1 year - %5.1lf%%\n", 100 * prob_exceed_val_yearly(g, 1, real_val));
	printf(" 5 years - %5.1lf%%\n", 100 * prob_exceed_val_yearly(g, 5, real_val));
	printf("10 years - %5.1lf%%\n", 100 * prob_exceed_val_yearly(g, 10, real_val));
	printf("15 years - %5.1lf%%\n", 100 * prob_exceed_val_yearly(g, 15, real_val));
	printf("20 years - %5.1lf%%\n", 100 * prob_exceed_val_yearly(g, 20, real_val));
	printf("25 years - %5.1lf%%\n", 100 * prob_exceed_val_yearly(g, 25, real_val));
}

static void print_exceed_expected_frequency(gumbel *g, int val) {
	if (1.0 / prob_exceed_val_yearly(g, 1, val) < 1.05)
		printf("Expected frequency: every year\n");
	else
		printf("Expected frequency: once every %.1lf years\n", 1.0 / prob_exceed_val_yearly(g, 1, val));
}

// print-out of prob_exceed_val_yearly() for 1-year, 5-year, 10-year, 15-year, 20-year, and 25-year periods
void print_prob_exeed_val_intervals_yearly(History *history, int var, int val) {
	gumbel *g = dist_yearly_mean_maximum(history, var);

	print_exceed_val_label(var, val);
	print_exceed_val_output(g, val);
	print_exceed_expected_frequency(g, val);

	destroy_gumbel(g);
}

static void print_under_val_label(int var, int val) {
	printf("\n");
	if (!var)
		printf("High temperature less than ");
	else
		printf("Low temperature less than ");

	printf("%d%cF\n", val, 248);
	printf("Probability of occurrence in Whole Year\n");
}

static void print_under_val_output(gumbel *g, int val) {
	double real_val = (double)val - 0.5;
	printf("  1 year - %5.1lf%%\n", 100 * prob_under_val_yearly(g, 1, real_val));
	printf(" 5 years - %5.1lf%%\n", 100 * prob_under_val_yearly(g, 5, real_val));
	printf("10 years - %5.1lf%%\n", 100 * prob_under_val_yearly(g, 10, real_val));
	printf("15 years - %5.1lf%%\n", 100 * prob_under_val_yearly(g, 15, real_val));
	printf("20 years - %5.1lf%%\n", 100 * prob_under_val_yearly(g, 20, real_val));
	printf("25 years - %5.1lf%%\n", 100 * prob_under_val_yearly(g, 25, real_val));
}

static void print_under_expected_frequency(gumbel *g, int val) {
	if (1.0 / prob_under_val_yearly(g, 1, val) < 1.05)
		printf("Expected frequency: every year\n");
	else
		printf("Expected frequency: once every %.1lf years\n", 1.0 / prob_under_val_yearly(g, 1, val));
}

// print-out of prob_under_val_yearly() for 1-year, 5-year, 10-year, 15-year, 20-year, and 25-year periods
void print_prob_under_val_intervals_yearly(History * history, int var, int val) {
	gumbel *g = dist_yearly_mean_minimum(history, var);
	print_under_val_label(var, val);
	print_under_val_output(g, val);
	print_under_expected_frequency(g, val);

	destroy_gumbel(g);
}

static void print_simulation_label(int extreme, int var) {
	printf("\nSimulating Yearly");
	extreme == MAX ? printf(" Maximum ") : printf(" Minimum ");
	if (!var)
		printf("High Temperatures for 30 Seconds\n");
	else
		printf("Low Temperatures for 30 Seconds\n");
}

static void print_simulated_value(gumbel *g, int extreme) {
	// simmulate numbers from percentiles 2.0% to 98.0%
	double pctl = ((double)(rand() % 961) + 20) / ((double)1000);
	if (extreme == MAX)
		printf("%4.0lf%cF ", pctl_max_gumbel(g, pctl), 248);
	else
		printf("%4.0lf%cF ", pctl_min_gumbel(g, pctl), 248);
}

void simulate_yearly_maximum(History *history, int var) {
	int i;
	gumbel *g = dist_yearly_mean_maximum(history, var);

	print_simulation_label(MAX, var);

	for (i = 0; i < 50; i++) {
		delay(0.6);
		printf("Year %02d Maximum ", i + 1);
		print_high_or_low(var);
		print_simulated_value(g, MAX);
		printf("\n");
	}

	printf("\nDone simulating.\n");
	destroy_gumbel(g);
}

void simulate_yearly_minimum(History *history, int var) {
	int i;
	gumbel *g = dist_yearly_mean_minimum(history, var);

	print_simulation_label(MIN, var);

	for (i = 0; i < 50; i++) {
		delay(0.6);
		printf("Year %02d Minimum ", i + 1);
		print_high_or_low(var);
		print_simulated_value(g, MIN);
		printf("\n");
	}

	printf("\nDone simulating.\n");
	destroy_gumbel(g);
}

// using 'monthly_normals', calculates and returns the yearly normal
double yearly_normal(MonthlyNormals *monthly_normals, int var) {
	int i;
	int counter = 0;
	double total = 0;
	for (i = 0; i < 12; i++) {
		double value = monthly_normals->normals[var][i]->value;
		if (!is_invalid(value)) {
			total += value;
			counter++;
		}
		else
			return UNDEFINED;
	}
	return total / (double)counter;
}

// returns 1 if all the months meet or exceed the given quality standard, 0 otherwise
int pass_yearly_quality_check(MonthlyNormals *monthly_normals, int var, char standard) {
	int i;
	for (i = 0; i < 12; i++)
		if (!pass_monthly_quality_check(monthly_normals, i + 1, var, standard))
			return 0;
	return 1;
}

// returns yearly average hourly 10th percentile, normal, or 90th percentile dew point value 
double yearly_avg_dewp(HourlyNormals *hourly_normals, int var) {
	int i, j, counter = 0;
	double total = 0;

	for (i = 0; i < 365; i++)
		for (j = 0; j < 24; j++) {
			double value = hourly_normals->dewp[var][i][j]->value;
			total += !is_invalid(value) ? value : 0;
			counter += !is_invalid(value) ? 1 : 0;
		}

	return (counter > 0) ? total / (double)counter : UNDEFINED;
}

static void fill_table_monthly(History *history, RecordExtremaTable *record_extrema_table, int i, int j) {
	RecordMaximum *record_maximum = record_maximum_monthly(history, j + 1, first_year(history), last_year(history), i);
	record_extrema_table->monthly_max[i][j] = record_maximum->value;

	RecordMinimum *record_minimum = record_minimum_monthly(history, j + 1, first_year(history), last_year(history), i);
	record_extrema_table->monthly_min[i][j] = record_minimum->value;

	destroy_record_maximum(record_maximum);
	destroy_record_minimum(record_minimum);
}

static void fill_table_yearly(History *history, RecordExtremaTable *record_extrema_table, int i, int j) {
	RecordMaximum *record_maximum = record_maximum_yearly(history, first_year(history), last_year(history), i);
	record_extrema_table->yearly_max[i] = record_maximum->value;

	RecordMinimum *record_minimum = record_minimum_yearly(history, first_year(history), last_year(history), i);
	record_extrema_table->yearly_min[i] = record_minimum->value;

	destroy_record_maximum(record_maximum);
	destroy_record_minimum(record_minimum);
}

// declares and returns a pointer to a RecordExtremaTable
// containing all monthly and yearly maximum and minimum values and their years of occurrences
RecordExtremaTable *create_record_extrema_table(History *history) {
	RecordExtremaTable *record_extrema_table = calloc(1, sizeof(RecordExtremaTable));
	int i, j;

	for (i = 0; i < 2; i++) {
		for (j = 0; j < 12; j++)
			fill_table_monthly(history, record_extrema_table, i, j);
		fill_table_yearly(history, record_extrema_table, i, j);
	}

	return record_extrema_table;
}

RecordExtremaTable *destroy_record_extrema_table(RecordExtremaTable *record_extrema_table) {
	free(record_extrema_table);
	record_extrema_table = NULL;
	return record_extrema_table;
}