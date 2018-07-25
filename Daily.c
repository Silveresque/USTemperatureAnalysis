#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "MonthlyTempRecords.h"
#include "Monthly.h"
#include "Daily.h"
#include "Calendar.h"
#include "Constants.h"

// helper functions for create_hourly_normals()
static void fill_vars_and_pctl(char vars[2][5], char pctl[3][7]);
static FILE *open_hourly_file(char *WBAN, char *var, char *pctl);
static void get_month_and_day(char *line, int *month, int *day);
static void add_hourly_normals(HourlyNormals *hourly_normals, double val, int month, int day, int i, int j, int k);
// helper functions for create_daily_normals()
static void initialize_daily_normals(DailyNormals *daily_normals);
static FILE *open_daily_file(char *WBAN, int var, int i);
static void enter_stddev(DailyNormals *daily_normals, char *value, int month, int i, int k);
static void parse_line(DailyNormals *daily_normals, char *line, int month, int var, int i, int k);
static void fill_values(DailyNormals *daily_normals, char *WBAN, int var, int i);
static void find_record_max(History *history, int day, int end_year, int var, int index, int *max_value, int *max_year);
static void find_record_min(History *history, int day, int end_year, int var, int index, int *min_value, int *min_year);
static void fill_normals(HourlyNormals *hourly_normals, char *line, char *WBAN, char *var, char *pctl, int i, int j);

static void fill_vars_and_pctl(char vars[2][5], char pctl[3][7]) {
	strcpy(vars[0], "temp");
	strcpy(vars[1], "dewp");
	strcpy(pctl[0], "10pctl");
	strcpy(pctl[1], "normal");
	strcpy(pctl[2], "90pctl");
}

static FILE *open_hourly_file(char *WBAN, char *var, char *pctl) {
	char filename[FILENAME];
	strcpy(filename, "hly-");
	strcat(filename, var);
	strcat(filename, "-");
	strcat(filename, pctl);
	strcat(filename, "/");
	strcat(filename, WBAN);
	strcat(filename, ".txt");
	return fopen(filename, "r");
}

static void get_month_and_day(char *line, int *month, int *day) {
#pragma warning(suppress: 6031)
	sscanf(line, "%d", month);
	strcpy(line, strchr(line, ' '));

#pragma warning(suppress: 6031)
	sscanf(line, "%d", day);
	strcpy(line, strchr(line + 1, ' '));
}

// store values in designated space in 'hourly_normals'
static void add_hourly_normals(HourlyNormals *hourly_normals, double val, int month, int day, int i, int j, int k) {
	if (!hourly_normals)
		return;
	
	if (i == 0) {
		hourly_normals->temp[j][month_day_to_int(month, day) - 1][k] = calloc(1, sizeof(Normal));
		if (!hourly_normals->temp[j][month_day_to_int(month, day) - 1][k]) {
			printf("Ran out of memory!\n");
			exit(EXIT_FAILURE);
		}
		hourly_normals->temp[j][month_day_to_int(month, day) - 1][k]->value = (val != UNAVAILABLE) ? val / 10.0 : UNAVAILABLE;
	}
	else {
		hourly_normals->dewp[j][month_day_to_int(month, day) - 1][k] = calloc(1, sizeof(Normal));
		if (!hourly_normals->dewp[j][month_day_to_int(month, day) - 1][k]) {
			printf("Ran out of memory!\n");
			exit(EXIT_FAILURE);
		}
		hourly_normals->dewp[j][month_day_to_int(month, day) - 1][k]->value = (val != UNAVAILABLE) ? val / 10.0 : UNAVAILABLE;
	}
}

// fill all requested values in 'hourly_normals'
static void fill_normals(HourlyNormals *hourly_normals, char *line, char *WBAN, char *var, char *pctl, int i, int j) {
	int k;
	FILE *ifp = open_hourly_file(WBAN, var, pctl);

	while (fgets(line, LINE, ifp)) {
		int month, day;
		get_month_and_day(line, &month, &day);

		double val;
		for (k = 0; k < 23; k++) {
#pragma warning(suppress: 6031)
			sscanf(line, "%lf", &val);
			add_hourly_normals(hourly_normals, val, month, day, i, j, k);
			strcpy(line, strchr(line + 1, ' '));
		}
#pragma warning(suppress: 6031)
		sscanf(line, "%lf", &val);
		add_hourly_normals(hourly_normals, val, month, day, i, j, k);
	}
}

// declares and returns pointer to a new HourlyNormals instance
HourlyNormals *create_hourly_normals(char *WBAN) {
	int i, j;
	char vars[2][5], pctl[3][7], line[LINE];

	HourlyNormals *hourly_normals = calloc(1, sizeof(HourlyNormals));
	if (!hourly_normals) {
		printf("Ran out of memory!\n");
		exit(EXIT_FAILURE);
	}

	fill_vars_and_pctl(vars, pctl);

	for (i = 0; i < 2; i++)
		for (j = 0; j < 3; j++) 
			fill_normals(hourly_normals, line, WBAN, vars[i], pctl[j], i, j);
			
	return hourly_normals;
}

HourlyNormals *destroy_hourly_normals(HourlyNormals *hourly_normals) {
	int i, j, k;

	for (i = 0; i < 3; i++) 
		for (j = 0; j < 365; j++) 
			for (k = 0; k < 24; k++) {
				free(hourly_normals->temp[i][j][k]);
				hourly_normals->temp[i][j][k] = NULL;

				free(hourly_normals->dewp[i][j][k]);
				hourly_normals->dewp[i][j][k] = NULL;
			}
		
	free(hourly_normals);	
	hourly_normals = NULL;
	return hourly_normals;
}

// initialize arrays in 'daily_normals'
static void initialize_daily_normals(DailyNormals *daily_normals) {
	if (!daily_normals)
		return;

	int i, j, k;

	for (i = 0; i < 3; i++)
		for (j = 0; j < 12; j++)
			for (k = 0; k < 31; k++) {
				daily_normals->normals[i][j][k] = calloc(1, sizeof(Normal));
				if (!daily_normals->normals[i][j][k]) {
					printf("Ran out of memory!\n");
					exit(EXIT_FAILURE);
				}
				daily_normals->normals[i][j][k]->value = UNDEFINED;
				daily_normals->normals[i][j][k]->quality = '\0';

				daily_normals->stddev[i][j][k] = calloc(1, sizeof(Normal));
				if (!daily_normals->stddev[i][j][k]) {
					printf("Ran out of memory!\n");
					exit(EXIT_FAILURE);
				}
				daily_normals->stddev[i][j][k]->value = UNDEFINED;
				daily_normals->stddev[i][j][k]->quality = '\0';
			}
}

static FILE *open_daily_file(char *WBAN, int var, int i) {
	char filename[FILENAME];
	strcpy(filename, !i ? "dly-tmax-" : (i == 1 ? "dly-tmin-" : "dly-tavg-"));
	strcat(filename, (var == NORMAL) ? "normal/" : "stddev/");
	strcat(filename, WBAN);
	strcat(filename, ".txt");
	return fopen(filename, "r");
}

// enter vlaue in designated space in 'daily_normals'
static void enter_normal(DailyNormals *daily_normals, char *value, int month, int i, int k) {
	if (is_int(value)) {
		daily_normals->normals[i][month - 1][k]->value = to_double(value);
		daily_normals->normals[i][month - 1][k]->quality = '\0';
	}
	else {
		daily_normals->normals[i][month - 1][k]->quality = value[strlen(value) - 1];
		value[strlen(value) - 1] = ' ';
		daily_normals->normals[i][month - 1][k]->value = to_double(value) / 10.0;
	}
}

static void enter_stddev(DailyNormals *daily_normals, char *value, int month, int i, int k) {
	if (is_int(value)) {
		daily_normals->stddev[i][month - 1][k]->value = to_double(value);
		daily_normals->stddev[i][month - 1][k]->quality = '\0';
	}
	else {
		daily_normals->stddev[i][month - 1][k]->quality = value[strlen(value) - 1];
		value[strlen(value) - 1] = ' ';
		daily_normals->stddev[i][month - 1][k]->value = to_double(value) / 10.0;
	}
}

static void parse_line(DailyNormals *daily_normals, char *line, int month, int var, int i, int k) {
	char value[6];
#pragma warning(suppress: 6031)
	sscanf(line, "%s", value);

	if (var == NORMAL)
		enter_normal(daily_normals, value, month, i, k);
	else
		enter_stddev(daily_normals, value, month, i, k);

	if (k < 30)
		strcpy(line, strchr(line + 1, ' '));
}

// fill all requested values in 'daily_normals'
static void fill_values(DailyNormals *daily_normals, char *WBAN, int var, int i) {
	int k, month;
	char line[LINE];

	FILE *ifp = open_daily_file(WBAN, var, i);

	while (fgets(line, LINE, ifp)) {
#pragma warning(suppress: 6031)
		sscanf(line, "%d", &month);
		strcpy(line, strchr(line, ' '));

		for (k = 0; k < 31; k++) 
			parse_line(daily_normals, line, month, var, i, k);
	}

	fclose(ifp);
}

// declares and returns pointer to a new DailyNormals instance
DailyNormals *create_daily_normals(char *WBAN) {
	int i;
	DailyNormals *daily_normals = calloc(1, sizeof(DailyNormals));

	initialize_daily_normals(daily_normals);

	for (i = 0; i < 3; i++) {
		fill_values(daily_normals, WBAN, NORMAL, i);
		fill_values(daily_normals, WBAN, STDDEV, i);
	}
	
	return daily_normals;
}

DailyNormals *destroy_daily_normals(DailyNormals *daily_normals) {
	int i, j, k;
	for (i = 0; i < 3; i++) 
		for (j = 0; j < 12; j++)
			for (k = 0; k < 31; k++) {
				free(daily_normals->normals[i][j][k]);
				daily_normals->normals[i][j][k] = NULL;

				free(daily_normals->stddev[i][j][k]);
				daily_normals->stddev[i][j][k] = NULL;
			}

	free(daily_normals);
	daily_normals = NULL;
	return daily_normals;
}

// finds maximum value and its year of ocucrrence
static void find_record_max(History *history, int day, int end_year, int var, int index, int *max_value, int *max_year) {
	int i;
	for (i = index; i < history->size && year_of_ym(history->monthly_records[i]->year_month) <= end_year; i += 12) {
		int value = get_monthly_record(history, i)->temps[var][day - 1];

		if (!is_invalid((double)value)) {
			*max_year = (value > *max_value) ? year_of_ym(history->monthly_records[i]->year_month) : *max_year;
			*max_value = max_int(*max_value, value);
		}
	}
}

// returns the daily record maximum value for either high or low temperature in the requested period
RecordMaximum *record_maximum_daily(History *history, int month, int day, int start_year, int end_year, int var) {
	if (!check_bounds_monthly(history, month, &start_year, &end_year))
		return NULL;

	int index = get_month_index(history, year_month_of(start_year, month));
	int counter = 0;

	RecordMaximum *record_max = calloc(1, sizeof(RecordMaximum));
	if (!record_max) {
		printf("Ran out of memory!\n");
		exit(EXIT_FAILURE);
	}

	int max_value = INT_MIN;
	int max_year = first_year(history);

	find_record_max(history, day, end_year, var, index, &max_value, &max_year);

	record_max->value = (max_value != INT_MIN) ? max_value : UNDEFINED;
	record_max->year = max_year;
	return record_max;
}

// finds minimum value and its year of ocucrrence
static void find_record_min(History *history, int day, int end_year, int var, int index, int *min_value, int *min_year) {
	int i;
	for (i = index; i < history->size && year_of_ym(history->monthly_records[i]->year_month) <= end_year; i += 12) {
		int value = get_monthly_record(history, i)->temps[var][day - 1];

		if (!is_invalid((double)value)) {
			*min_year = (value < *min_value) ? year_of_ym(history->monthly_records[i]->year_month) : *min_year;
			*min_value = min_int(*min_value, value);
		}
	}
}

// returns the daily record minimum value for either high or low temperature in the requested period
RecordMinimum *record_minimum_daily(History *history, int month, int day, int start_year, int end_year, int var) {
	if (!check_bounds_monthly(history, month, &start_year, &end_year))
		return NULL;

	int index = get_month_index(history, year_month_of(start_year, month));
	int counter = 0;

	RecordMinimum *record_min = calloc(1, sizeof(RecordMinimum));
	if (!record_min) {
		printf("Ran out of memory!\n");
		exit(EXIT_FAILURE);
	}

	int min_value = INT_MAX;
	int min_year = first_year(history);

	find_record_min(history, day, end_year, var, index, &min_value, &min_year);

	record_min->value = (min_value != INT_MAX) ? min_value : UNDEFINED;
	record_min->year = min_year;
	return record_min;
}

// returns the average hourly value for 10th percentile, normal, or 90th percentile dew point for the requested day
double daily_avg_dewp(HourlyNormals *hourly_normals, int month, int day, int var) {
	int i, index = month_day_to_int(month, day) - 1;
	
	double total = 0;
	int counter = 0;
	for (i = 0; i < 24; i++) {
		double value = hourly_normals->dewp[var][index][i]->value;
		total += !is_invalid(value) ? value : 0;
		counter += !is_invalid(value) ? 1 : 0;
	}

	return (counter > 0) ? total / (double)counter : UNDEFINED;
}