#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include <time.h>
#include "MonthlyTempRecords.h"
#include "Monthly.h"
#include "Calendar.h"
#include "Constants.h"

// helper function for record_maximum_monthly()
static void find_record_max(History *history, int month, int end_year, int var, int index, int *max_value, int *max_year);
// helper function for record_minimum_monthly()
static void find_record_min(History *history, int month, int end_year, int var, int index, int *min_value, int *min_year);
// helper function for monthly_mean_maximum()
static void calc_monthly_max_stats(History *history, int month, int end_year, int var, int index,
	int *maximum_totals, int *counter);
// helper function for monthly_mean_minimum()
static void calc_monthly_min_stats(History *history, int month, int end_year, int var, int index,
	int *minimum_totals, int *counter);
// helper functions for pctl_monthly()
static int *resize_temps_array(int *temps, int *capacity, int size);
static int *create_pctl_array(History *history, int month, int end_year, int var, int index, int *size);
// helper functions for check_bounds_monthly()
static void adjust_start_year(History *history, int *start_year, int month);
static void adjust_end_year(History *history, int *end_year, int month);
// helper functions for create_monthly_normals()
static void initialize_monthly_normals(MonthlyNormals *monthly_normals);
static FILE *open_monthly_file(char *WBAN, int var, int i);
static void enter_normal(MonthlyNormals *monthly_normals, char *value, int i, int j);
static void enter_stddev(MonthlyNormals *monthly_normals, char *value, int i, int j);
static void parse_line(MonthlyNormals *monthly_normals, char *line, int var, int i, int j);
static void fill_values(MonthlyNormals *monthly_normals, char *WBAN, int var, int i);
// helper functions for print_prob_exceed_vals_intervals_monthly()
static void print_exceed_val_label(int month, int var, int val);
static void print_exceed_val_output(gumbel *g, int val);
static void print_exceed_expected_frequency(gumbel *g, int val);
// helper functions for print_prob_under_val_intervals_monthly()
static void print_under_val_label(int month, int var, int val);
static void print_under_val_output(gumbel *g, int val);
static void print_under_expected_frequency(gumbel *g, int val);
// helper functions for simulate_monthly_maximum() and simulate_monthly_minimum()
static void print_simulation_label(int month, int extreme, int var);
static void print_simulated_value(gumbel *g, int extreme);

// returns the mean maximum of a given month
int monthly_maximum(History *history, int month, int year, int var) {
	if (!check_bounds_monthly(history, month, &year, &year))
		return UNAVAILABLE;

	int i, index = get_month_index(history, year_month_of(year, month));
	int month_maximum = INT_MIN;

	for (i = 0; i < 31; i++) {
		int temp = history->monthly_records[index]->temps[var][i];
		if (!is_invalid(temp))
			month_maximum = max_int(temp, month_maximum);
	}

	return (month_maximum != INT_MIN) ? month_maximum : UNDEFINED;
}

// returns the mean minimum of one month
int monthly_minimum(History *history, int month, int year, int var) {
	if (!check_bounds_monthly(history, month, &year, &year))
		return UNAVAILABLE;

	int i, index = get_month_index(history, year_month_of(year, month));
	int month_minimum = INT_MAX;

	for (i = 0; i < 31; i++) {
		int temp = history->monthly_records[index]->temps[var][i];
		if (!is_invalid(temp))
			month_minimum = min_int(temp, month_minimum);
	}

	return (month_minimum != INT_MAX) ? month_minimum : UNDEFINED;
}

static void find_record_max(History *history, int month, int end_year, int var, int index, int *max_value, int *max_year) {
	int i;
	for (i = index; i < history->size && year_of_ym(history->monthly_records[i]->year_month) <= end_year; i += 12) {
		int monthly_max = monthly_maximum(history, month, year_of_ym(history->monthly_records[i]->year_month), var);

		if (!is_invalid(monthly_max)) {
			*max_year = (monthly_max > *max_value) ? year_of_ym(history->monthly_records[i]->year_month) : *max_year;
			*max_value = max_int(*max_value, monthly_max);
		}
	}
}

// returns a pointer to new RecordMaximum instance containing the value and its year of occurrence
RecordMaximum *record_maximum_monthly(History *history, int month, int start_year, int end_year, int var) {
	if (!check_bounds_monthly(history, month, &start_year, &end_year))
		return NULL;

	int index = get_month_index(history, year_month_of(start_year, month));
	int max_value = INT_MIN;
	int max_year = start_year;
	RecordMaximum *record_maximum = calloc(1, sizeof(RecordMaximum));
	if (!record_maximum) {
		printf("Ran out of memory!\n");
		exit(EXIT_FAILURE);
	}

	find_record_max(history, month, end_year, var, index, &max_value, &max_year);

	record_maximum->value = (max_value != INT_MIN) ? max_value : UNDEFINED;
	record_maximum->year = max_year;

	return record_maximum;
}

static void find_record_min(History *history, int month, int end_year, int var, int index, int *min_value, int *min_year) {
	int i;
	for (i = index; i < history->size && year_of_ym(history->monthly_records[i]->year_month) <= end_year; i += 12) {
		int monthly_min = monthly_minimum(history, month, year_of_ym(history->monthly_records[i]->year_month), var);

		if (!is_invalid(monthly_min)) {
			*min_year = (monthly_min < *min_value) ? year_of_ym(history->monthly_records[i]->year_month) : *min_year;
			*min_value = min_int(*min_value, monthly_min);
		}
	}
}

// returns a pointer to new RecordMinimum instance containing the value and its year of occurrence
RecordMinimum *record_minimum_monthly(History *history, int month, int start_year, int end_year, int var) {
	if (!check_bounds_monthly(history, month, &start_year, &end_year))
		return NULL;

	int index = get_month_index(history, year_month_of(start_year, month));
	int min_value = INT_MAX;
	int min_year = start_year;
	RecordMinimum *record_minimum = calloc(1, sizeof(RecordMinimum));
	if (!record_minimum) {
		printf("Ran out of memory!\n");
		exit(EXIT_FAILURE);
	}

	find_record_min(history, month, end_year, var, index, &min_value, &min_year);

	record_minimum->value = (min_value != INT_MAX) ? min_value : UNDEFINED;
	record_minimum->year = min_year;

	return record_minimum;
}

static void calc_monthly_max_stats(History *history,int month, int end_year, int var, int index, 
	int *maximum_totals, int *counter) {
	int i;
	for (i = index; i < history->size && year_of_ym(history->monthly_records[i]->year_month) <= end_year; i += 12) {
		int month_maximum = monthly_maximum(history, month, year_of_ym(history->monthly_records[i]->year_month), var);

		if (month_maximum != UNAVAILABLE) {
			*maximum_totals += month_maximum;
			*counter += 1;
		}
	}
}

// returns the mean maximum temperature of a given month over a specified time period
double monthly_mean_maximum(History *history, int month, int start_year, int end_year, int var) {
	if (!check_bounds_monthly(history, month, &start_year, &end_year))
		return UNAVAILABLE;

	int index = get_month_index(history, year_month_of(start_year, month));
	int counter = 0;
	int maximum_totals = 0;

	calc_monthly_max_stats(history, month, end_year, var, index, &maximum_totals, &counter);

	return (counter) ? ((double)maximum_totals) / ((double)counter) : UNDEFINED;
}

static void calc_monthly_min_stats(History *history, int month, int end_year, int var, int index,
	int *minimum_totals, int *counter) {
	int i;
	for (i = index; i < history->size && year_of_ym(history->monthly_records[i]->year_month) <= end_year; i += 12) {
		int month_minimum = monthly_minimum(history, month, year_of_ym(history->monthly_records[i]->year_month), var);

		if (month_minimum != UNAVAILABLE) {
			*minimum_totals += month_minimum;
			*counter += 1;
		}
	}
}

// returns the mean minimum temperature of a given month over a specified time period
double monthly_mean_minimum(History *history, int month, int start_year, int end_year, int var) {
	if (!check_bounds_monthly(history, month, &start_year, &end_year))
		return UNAVAILABLE;

	int index = get_month_index(history, year_month_of(start_year, month));
	int counter = 0;
	int minimum_totals = 0;

	calc_monthly_min_stats(history, month, end_year, var, index, &minimum_totals, &counter);

	return (counter) ? ((double)minimum_totals) / ((double)counter) : UNDEFINED;
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

static int *create_pctl_array(History *history, int month, int end_year, int var, int index, int *size) {
	int i, j;
	int capacity = 31;
	int *temps = calloc(capacity, sizeof(int));
	if (!temps) {
		printf("Ran out of memory!\n");
		exit(EXIT_FAILURE);
	}

	for (i = index; i < history->size && year_of_ym(history->monthly_records[i]->year_month) <= end_year; i += 12) {
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
int pctl_monthly(History *history, int month, int start_year, int end_year, int var, int pctl) {
	if (!check_bounds_monthly(history, month, &start_year, &end_year) || (pctl < 1 || pctl > 99))
		return UNAVAILABLE;

	int index = get_month_index(history, year_month_of(start_year, month));
	int capacity = 31, size = 0;
	double position;
	int retval, *temps = create_pctl_array(history, month, end_year, var, index, &size);
	
	bucket_sort(temps, size);
	position = ((double)(pctl * size)) / ((double)100) - 1;
	retval = temps[(int)round(position)];

	free(temps);
	return retval;
}

static void adjust_start_year(History *history, int *start_year, int month) {
	*start_year = year_of_ym(history->start);
	if (year_month_of(*start_year, month) < history->start)
		*start_year += 1;
}

static void adjust_end_year(History *history, int *end_year, int month) {
	*end_year = year_of_ym(history->end);
	if (year_month_of(*end_year, month) > history->end)
		*end_year -= 1;
}

// general purpose bounds checking of years passed to other functions
// can both be used to check appropriate bounds for both one year alone and a range of years
int check_bounds_monthly(History *history, int month, int *start_year, int *end_year) {
	if (year_month_of(*start_year, month) > history->end || year_month_of(*end_year, month) < history->start)
		return 0;

	// if only checking for one year, we're done
	if (*start_year == *end_year)
		return 1;

	if (*start_year > *end_year)
		return 0;

	if (year_month_of(*start_year, month) < history->start)
		adjust_start_year(history, start_year, month);

	if (year_month_of(*end_year, month) > history->end)
		adjust_end_year(history, end_year, month);

	return 1;
}

// returns a pointer to a gumbel distribution for mean maximums
gumbel *dist_monthly_mean_maximum(History *history, int month, int var) {
	int monthly_max[30];
	int i, capacity = 10, size = 0;

	for (i = 0; i < 30; i++) {
		int value = monthly_maximum(history, month, 1981 + i, var);
		if (value != UNAVAILABLE)
			monthly_max[size++] = value;
	}
	
	gumbel *g = generate_max_gumbel(monthly_max, size);
	
	return g;
}

/*
 *returns the probability of at least one occurrence of a value greater than or equal to 'val'
 *in a given amount of years combining gumbel and geometric distributions
 */
// cdf_max_gumbel(g, val) is equal to the probability of no occurrence in one year
// pow(cdf_max_gumbel(g, val), years) is equal to the probability of no occurrences in 'years' years
double prob_exceed_val_monthly(gumbel *g, double years, double val) {
	return 1 - pow(cdf_max_gumbel(g, val), years);
}

// gumbel distribution for mean minimums
gumbel *dist_monthly_mean_minimum(History *history, int month, int var) {
	int monthly_min[30];
	int i, capacity = 10, size = 0;

	for (i = 0; i < 30; i++) {
		int value = monthly_minimum(history, month, 1981 + i, var);
		if (value != UNAVAILABLE)
			monthly_min[size++] = value;
	}

	gumbel *g = generate_min_gumbel(monthly_min, size);

	return g;
}

// same as prob_exeed_val but for mean minimums
double prob_under_val_monthly(gumbel *g, double years, double val) {
	return 1 - pow(1 - cdf_min_gumbel(g, val), years);
}

// initializes 'monthly_normals' arrays
static void initialize_monthly_normals(MonthlyNormals *monthly_normals) {
	if (!monthly_normals)
		return;

	int i, j;

	for (i = 0; i < 3; i++)
		for (j = 0; j < 12; j++) {
			monthly_normals->normals[i][j] = calloc(1, sizeof(Normal));
			if (!monthly_normals->normals[i][j]) {
				printf("Ran out of memory!\n");
				exit(EXIT_FAILURE);
			}
			monthly_normals->normals[i][j]->value = UNAVAILABLE;
			monthly_normals->normals[i][j]->quality = '\0';

			monthly_normals->stddev[i][j] = calloc(1, sizeof(Normal));
			if (!monthly_normals->stddev[i][j]) {
				printf("Ran out of memory!\n");
				exit(EXIT_FAILURE);
			}
			monthly_normals->stddev[i][j]->value = UNAVAILABLE;
			monthly_normals->stddev[i][j]->quality = '\0';
		}
}

static FILE *open_monthly_file(char *WBAN, int var, int i) {
	char filename[FILENAME];
	strcpy(filename, !i ? "mly-tmax-" : (i == 1 ? "mly-tmin-" : "mly-tavg-"));
	strcat(filename, (var == NORMAL) ? "normal/" : "stddev/");
	strcat(filename, WBAN);
	strcat(filename, ".txt");
	return fopen(filename, "r");
}

// enters the value in the designated space in 'monthly'normals'
static void enter_normal(MonthlyNormals *monthly_normals, char *value, int i, int j) {
	if (is_int(value)) {
		monthly_normals->normals[i][j]->value = to_double(value);
		monthly_normals->normals[i][j]->quality = '\0';
	}
	else {
		monthly_normals->normals[i][j]->quality = value[strlen(value) - 1];
		value[strlen(value) - 1] = ' ';
		monthly_normals->normals[i][j]->value = to_double(value) / 10.0;
	}
}

static void enter_stddev(MonthlyNormals *monthly_normals, char *value, int i, int j) {
	if (is_int(value)) {
		monthly_normals->stddev[i][j]->value = to_double(value);
		monthly_normals->stddev[i][j]->quality = '\0';
	}
	else {
		monthly_normals->stddev[i][j]->quality = value[strlen(value) - 1];
		value[strlen(value) - 1] = ' ';
		monthly_normals->stddev[i][j]->value = to_double(value) / 10.0;
	}
}

static void parse_line(MonthlyNormals *monthly_normals, char *line, int var, int i, int j) {
	char value[6];
#pragma warning(suppress: 6031)
	sscanf(line, "%s", value);

	if (var == NORMAL)
		enter_normal(monthly_normals, value, i, j);
	else
		enter_stddev(monthly_normals, value, i, j);

	if (j < 11)
		strcpy(line, strchr(line + 1, ' '));
}

// fills all the arrays in 'monthly_normals'
static void fill_values(MonthlyNormals *monthly_normals, char *WBAN, int var, int i) {
	int j;
	char line[LINE];

	FILE *ifp = open_monthly_file(WBAN, var, i);
	
	if (!fgets(line, LINE, ifp))
		return;

	for (j = 0; j < 12; j++)
		parse_line(monthly_normals, line, var, i, j);

	fclose(ifp);
}

// declares and returns a pointer to a new MonthlyNormals instance
MonthlyNormals *create_monthly_normals(char *WBAN) {
	MonthlyNormals *monthly_normals = calloc(1, sizeof(MonthlyNormals));
	if (!monthly_normals) {
		printf("Ran out of memory!\n");
		exit(EXIT_FAILURE);
	}

	initialize_monthly_normals(monthly_normals);

	int i;
	for (i = 0; i < 3; i++) {
		fill_values(monthly_normals, WBAN, NORMAL, i);
		fill_values(monthly_normals, WBAN, STDDEV, i);
	}

	return monthly_normals;
}

MonthlyNormals *destroy_monthly_normals(MonthlyNormals *monthly_normals) {
	int i, j;
	for (i = 0; i < 3; i++)
		for (j = 0; j < 12; j++) {
			free(monthly_normals->normals[i][j]);
			monthly_normals->normals[i][j] = NULL;

			free(monthly_normals->stddev[i][j]);
			monthly_normals->stddev[i][j] = NULL;
		}
	free(monthly_normals);
	monthly_normals = NULL;
	return monthly_normals;
}

// returns 1 if quality meets or exceeds the given standard, 0 otherwise
int pass_monthly_quality_check(MonthlyNormals *monthly_normals, int month, int var, char standard) {
	switch (standard) {
	case '\0':
		if (monthly_normals->normals[var][month - 1]->quality == '\0')
			return 1;
	case 'Q':
		if (monthly_normals->normals[var][month - 1]->quality == 'Q')
			return 1;
	case 'P':
		if (monthly_normals->normals[var][month - 1]->quality == 'P')
			return 1;
	case 'R':
		if (monthly_normals->normals[var][month - 1]->quality == 'R')
			return 1;
	case 'S':
		if (monthly_normals->normals[var][month - 1]->quality == 'S')
			return 1;
	case 'C':
		if (monthly_normals->normals[var][month - 1]->quality == 'C')
			return 1;
	}
	return 0;
}

// returns average 10th percentile, normal, or 90th percentile dew point value in the month
double monthly_avg_dewp(HourlyNormals *hourly_normals, int month, int var) {
	int i, j, counter = 0;
	double total = 0;

	int start_index = month_day_to_int(month, 1) - 1;
	int end_index = month_day_to_int(month, calc_num_days(month)) - 1;

	for (i = start_index; i <= end_index; i++)
		for (j = 0; j < 24; j++) {
			double value = hourly_normals->dewp[var][i][j]->value;
			total += !is_invalid(value) ? value : 0;
			counter += !is_invalid(value) ? 1 : 0;
		}
	
	return (counter > 0) ? total / (double)counter : UNDEFINED;
}

static void print_exceed_val_label(int month, int var, int val) {
	printf("\n");
	if (!var)
		printf("High temperature at least ");
	else
		printf("Low temperature at least ");

	printf("%d%cF\n", val, 248);
	printf("Probability of occurrence in ");
	print_month_from_int(month);
	printf("\n");
}

static void print_exceed_val_output(gumbel *g, int val) {
	double real_val = (double)val - 0.5;
	printf("  1 year - %5.1lf%%\n", 100 * prob_exceed_val_monthly(g, 1, real_val));
	printf(" 5 years - %5.1lf%%\n", 100 * prob_exceed_val_monthly(g, 5, real_val));
	printf("10 years - %5.1lf%%\n", 100 * prob_exceed_val_monthly(g, 10, real_val));
	printf("15 years - %5.1lf%%\n", 100 * prob_exceed_val_monthly(g, 15, real_val));
	printf("20 years - %5.1lf%%\n", 100 * prob_exceed_val_monthly(g, 20, real_val));
	printf("25 years - %5.1lf%%\n", 100 * prob_exceed_val_monthly(g, 25, real_val));
}

static void print_exceed_expected_frequency(gumbel *g, int val) {
	if (1.0 / prob_exceed_val_monthly(g, 1, val) < 1.05)
		printf("Expected frequency: every year");
	else
		printf("Expected frequency: once every %.1lf years\n", 1.0 / prob_exceed_val_monthly(g, 1, val));
}

// print-out of prob_exceed_val_monthly() for 1-year, 5-year, 10-year, 15-year, 20-year, and 25-year periods
void print_prob_exeed_val_intervals_monthly(History *history, int month, int var, int val) {
	gumbel *g = dist_monthly_mean_maximum(history, month, var);

	print_exceed_val_label(month, var, val);
	print_exceed_val_output(g, val);
	print_exceed_expected_frequency(g, val);

	destroy_gumbel(g);
}

static void print_under_val_label(int month, int var, int val) {
	printf("\n");
	if (!var)
		printf("High temperature less than ");
	else
		printf("Low temperature less than ");

	printf("%d%cF\n", val, 248);
	printf("Probability of occurrence in ");
	print_month_from_int(month);
	printf("\n");
}

static void print_under_val_output(gumbel *g, int val) {
	double real_val = (double)val - 0.5;
	printf("  1 year - %5.1lf%%\n", 100 * prob_under_val_monthly(g, 1, real_val));
	printf(" 5 years - %5.1lf%%\n", 100 * prob_under_val_monthly(g, 5, real_val));
	printf("10 years - %5.1lf%%\n", 100 * prob_under_val_monthly(g, 10, real_val));
	printf("15 years - %5.1lf%%\n", 100 * prob_under_val_monthly(g, 15, real_val));
	printf("20 years - %5.1lf%%\n", 100 * prob_under_val_monthly(g, 20, real_val));
	printf("25 years - %5.1lf%%\n", 100 * prob_under_val_monthly(g, 25, real_val));
}

static void print_under_expected_frequency(gumbel *g, int val) {
	if (1.0 / prob_under_val_monthly(g, 1, val) < 1.05)
		printf("Expected frequency: every year");
	else
		printf("Expected frequency: once every %.1lf years\n", 1.0 / prob_under_val_monthly(g, 1, val));
}

// print-out of prob_under_val_monthly() for 1-year, 5-year, 10-year, 15-year, 20-year, and 25-year periods
void print_prob_under_val_intervals_monthly(History *history, int month, int var, int val) {
	gumbel *g = dist_monthly_mean_minimum(history, month, var);

	print_under_val_label(month, var, val);
	print_under_val_output(g, val);
	print_under_expected_frequency(g, val);

	destroy_gumbel(g);
}

static void print_simulation_label(int month, int extreme, int var) {
	printf("\nSimulating ");
	print_month_from_int(month);
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

void simulate_monthly_maximum(History *history, int month, int var) {
	int i;
	gumbel *g = dist_monthly_mean_maximum(history, month, var);

	print_simulation_label(month, MAX, var);

	for (i = 0; i < 50; i++) {
		delay(0.6);
		print_month_from_int(month);
		printf(" Year %02d Maximum ", i + 1);
		print_high_or_low(var);
		print_simulated_value(g, MAX);
		printf("\n");
	}

	printf("\nDone simulating.\n");
	destroy_gumbel(g);
}

void simulate_monthly_minimum(History *history, int month, int var) {
	int i;
	gumbel *g = dist_monthly_mean_minimum(history, month, var);

	print_simulation_label(month, MIN, var);

	for (i = 0; i < 50; i++) {
		delay(0.6);
		print_month_from_int(month);
		printf(" Year %02d Minimum ", i + 1);
		print_high_or_low(var);
		print_simulated_value(g, MIN);
		printf("\n");
	}

	printf("\nDone simulating.\n");
	destroy_gumbel(g);
}