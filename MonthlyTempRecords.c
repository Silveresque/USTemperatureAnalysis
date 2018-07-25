#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <limits.h>
#include "MonthlyTempRecords.h"
#include "Auxiliary.h"
#include "Calendar.h"
#include "Constants.h"

typedef ActualTemps* ActualTempsPointer;

// helper functions for create_history
static void store_year_month(char *line, ActualTempsPointer monthly_temps);
static void copy_temp_value(char *temp, char *component);
static void parse_component(char *line, char *temp);
static void store_temp_values(char *line, ActualTempsPointer monthly_temps);
static ActualTempsPointer create_actual_temps(char *line);
static FILE *open_station_file(char *WBAN);
static ActualTempsPointer *resize_actual_temps_array(ActualTempsPointer *actual_temps_array, int current_capacity);
static ActualTempsPointer *add_to_actual_temps_array(ActualTempsPointer *actual_temps_array,
	ActualTempsPointer actual_temps, int *current_capacity, int *counter);
static void fill_highs_and_lows(char *WBAN, ActualTempsPointer **daily_high, ActualTempsPointer **daily_low,
	int *earliest, int *latest);
static History *initialize_history(int earliest, int latest);
static void missing_fill(int *arr);
static void fill_history_temps(MonthlyRecord *monthly_record, ActualTempsPointer *actual_temps_array,
	int current, int *iterator_high, int var);
static void fill_avg_temps(History *history, int index);

static void store_year_month(char *line, ActualTempsPointer monthly_temps) {
	char year_month_string[7];
	strncpy(year_month_string, line + 11, 6);
	monthly_temps->year_month = to_int(year_month_string);
	strcpy(line, line + 21);
}

// check for bad quality control flag
// if one is present, copy -9999 (the value that corresponds to UNAVAILABLE) into 'temp' instead
static void copy_temp_value(char *temp, char *component) {
	if (component[6] != ' ')
		strcpy(temp, "-9999");
	else
		strncpy(temp, component, 5);
	temp[5] = '\0';
}

static void parse_component(char *line, char *temp) {
	char component[9];
	strncpy(component, line, 8);
	component[8] = '\0';

	copy_temp_value(temp, component);
}

static void store_temp_values(char *line, ActualTempsPointer monthly_temps) {
	int i;
	for (i = 0; i < 31; i++) {
		char *temp = calloc(6, sizeof(char));
		parse_component(line, temp);
		monthly_temps->temps[i] = tenths_celsius_to_fahrenheit(to_double(temp));

		strcpy(line, line + 8);
		free(temp);
		temp = NULL;
	}
}

// processes one line of ghcnd
static ActualTempsPointer create_actual_temps(char *line) {
    ActualTempsPointer monthly_temps = calloc(1, sizeof(ActualTemps));
	store_year_month(line, monthly_temps);
	store_temp_values(line, monthly_temps);
    return monthly_temps;
}

static FILE *open_station_file(char *WBAN) {
	char filename[36];
	strcpy(filename, "hly-inventory-ghcnd/");
	strcat(filename, WBAN);
	strcat(filename, ".txt");
	return fopen(filename, "r");
}

static ActualTempsPointer *resize_actual_temps_array(ActualTempsPointer *actual_temps_array, int current_capacity) {
	ActualTempsPointer *temp = realloc(actual_temps_array, sizeof(ActualTempsPointer) * current_capacity);
	if (!temp) {
		printf("Out of memory!\n");
		exit(EXIT_FAILURE);
	}
	else
		return temp;
}

static ActualTempsPointer *add_to_actual_temps_array(ActualTempsPointer *actual_temps_array, ActualTempsPointer actual_temps, int *current_capacity, int *counter) {
	actual_temps_array[*counter] = actual_temps;
	*counter += 1;

	if (*current_capacity <= *counter) {
		*current_capacity = *current_capacity * 2;
		return resize_actual_temps_array(actual_temps_array, *current_capacity);
	}
	else
		return actual_temps_array;
}

static void fill_highs_and_lows(char *WBAN, ActualTempsPointer **daily_high, ActualTempsPointer **daily_low, 
	int *earliest, int *latest) {
	FILE *ifp = open_station_file(WBAN);

	int status, capacity_high = 12, capacity_low = 12, counter_high = 0, counter_low = 0;
	char line[LINE];

	while (fgets(line, LINE, ifp)) {
		status = (!strncmp(line + 17, "TMAX", 4)) ? 1 : 0;

		ActualTempsPointer actual_temps = create_actual_temps(line);

		*earliest = min_int(actual_temps->year_month, *earliest);
		*latest = max_int(actual_temps->year_month, *latest);

		if (status)
			*daily_high = add_to_actual_temps_array(*daily_high, actual_temps, &capacity_high, &counter_high);
		else
			*daily_low = add_to_actual_temps_array(*daily_low, actual_temps, &capacity_low, &counter_low);
	}

	fclose(ifp);
}

// find the number of months for which records exist and allocate that many pointers for the monthly_records array
static History *initialize_history(int earliest, int latest) {
	History *history = calloc(1, sizeof(History));
	if (!history) {
		printf("Ran out of memory!\n");
		exit(EXIT_FAILURE);
	}

	history->size = calc_num_months(earliest, latest);
	history->start = earliest;
	history->end = latest;
	history->monthly_records = calloc(history->size, sizeof(MonthlyRecord*));
	return history;
}

// fills array with UNAVAILABLE values
static void missing_fill(int *arr) {
	int i;
	for (i = 0; i < 31; i++)
		arr[i] = UNAVAILABLE;
}

static void fill_history_temps(MonthlyRecord *monthly_record, ActualTempsPointer *actual_temps_array, 
	int current, int *iterator_high, int var) {
	// for daily highs portion
	if (actual_temps_array[*iterator_high]->year_month == current) {
		memcpy(monthly_record->temps[var], actual_temps_array[*iterator_high]->temps, 31 * sizeof(int));
		free(actual_temps_array[*iterator_high]);
		actual_temps_array[*iterator_high] = NULL;
		*iterator_high += 1;
	}
	else
		missing_fill(monthly_record->temps[var]);
}

static void fill_avg_temps(History *history, int index) {
	int *highs = history->monthly_records[index]->temps[HIGH];
	int *lows = history->monthly_records[index]->temps[LOW];

	int j;
	for (j = 0; j < 31; j++) 
		if (highs[j] != UNAVAILABLE && lows[j] != UNAVAILABLE)
			history->monthly_records[index]->avg_temps[j] = (highs[j] + lows[j]) / 2.0;
		else
			history->monthly_records[index]->avg_temps[j] = UNAVAILABLE;
	
}

History *create_history(char *WBAN) {
    // two ActualTemps arrays: one for daily highs and one for daily lows
    // start at an initial capacity of 12 and are doubled as needed
    // 'status' used to determine which array to add the next ActualTemps to
    int earliest = INT_MAX, latest = INT_MIN;

    ActualTempsPointer *daily_high = calloc(12, sizeof(ActualTempsPointer));
    ActualTempsPointer *daily_low = calloc(12, sizeof(ActualTempsPointer));
	fill_highs_and_lows(WBAN, &daily_high, &daily_low, &earliest, &latest);

	History *history = initialize_history(earliest, latest);

    // used to fill 'history'
    int i, current = earliest, iterator_high = 0, iterator_low = 0;

    for (i = 0; i < history->size; i++) {
        history->monthly_records[i] = calloc(1, sizeof(MonthlyRecord));
		if (!history->monthly_records[i]) {
			printf("Ran out of memory!\n");
			exit(EXIT_FAILURE);
		}
        history->monthly_records[i]->year_month = current;

		fill_history_temps(history->monthly_records[i], daily_high, current, &iterator_high, HIGH);
		fill_history_temps(history->monthly_records[i], daily_low, current, &iterator_low, LOW);
		fill_avg_temps(history, i);

        // move current to the next year_month;
        current = ((current - current / 100 * 100) < 12) ? current + 1 : 100 * (current / 100 + 1) + 1;
    }

	free(daily_high);
	free(daily_low);

    return history;
}

// function to calculate number of months given a starting year_month and ending year_month
int calc_num_months(int start, int end) {
    int start_year = start / 100, end_year = end / 100;
    int month_diff = end - end_year * 100 - (start - start_year * 100) + 1;
    return 12 * (end_year - start_year) + month_diff;
}

// function to calculate from tenths of degrees Celsius to Fahrenheit
int tenths_celsius_to_fahrenheit(double temp) {
    if (temp == -9999)
        return -9999;

    return (int) round(temp * 0.9 / ((double) 5) + 32);
}

// binary search to a year_month
// returns a pointer to a MonthlyRecord
MonthlyRecord *search_month(History *history, int year_month) {
    int lower = 0, upper = history->size - 1;
    int mid = upper >> 1;

    while (history->monthly_records[mid]->year_month != year_month) {
        if (history->monthly_records[mid]->year_month > year_month)
            upper = mid - 1;
        else
            lower = mid + 1;

        mid = (upper + lower) >> 1;

        if (lower > upper)
            return NULL;
    }

    return history->monthly_records[mid];
}

// simply returns the MonthlyRecord at 'index' in 'history" if it exists, otherwise NULL
MonthlyRecord *get_monthly_record(History *history, int index) {
	if (index < 0 || index >= calc_num_months(history->start, history->end))
		return NULL;
	else
		return history->monthly_records[index];
}

// binary search given a year_month
// returns the index in the MonthlyRecords array in 'history'
int get_month_index(History *history, int year_month) {
    int lower = 0, upper = history->size - 1;
    int mid = upper >> 1;

    while (history->monthly_records[mid]->year_month != year_month) {
        if (history->monthly_records[mid]->year_month > year_month)
            upper = mid - 1;
        else
            lower = mid + 1;

        mid = (upper + lower) >> 1;

        if (lower > upper)
            return -1;
    }

    return mid;
}

// free all dynamically allocated memory
History *destroy_history(History *history) {
	int i;
	for (i = 0; i < history->size; i++) 
		free(history->monthly_records[i]);

	free(history->monthly_records);
	
    free(history);
	return NULL;
}

int first_year(History *history) {
	return year_of_ym(history->start);
}

int last_year(History *history) {
	return year_of_ym(history->end);
}