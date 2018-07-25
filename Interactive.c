#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "Interactive.h"
#include "StringBST.h"
#include "Auxiliary.h"
#include "Calendar.h"
#include "Gumbel.h"
#include "Station.h"
#include "Constants.h"

// helper functions for year_lookup() and month_lookup()
static void print_monthly_record_header(int year_month);
static void print_day_numbers(int num_days);
static void print_temps(MonthlyRecord *record, int num_days, int var);
static void print_record_table(MonthlyRecord *record, int num_days);
static void calc_departure_stats(MonthlyRecord *record, double *total_avg, int *avg_counter, int num_days);
static void print_month(MonthlyRecord *record, MonthlyNormals *monthly_normals);
// helper functions for temp_annual_table()
static void print_record_extrema(RecordExtremaTable *record_extrema_table, int extreme, int var);
static void print_mean_extrema(History *history, MonthlyNormals *monthly_normals, int extreme, int var);
static void print_monthly_normals(MonthlyNormals *monthly_normals, int var);
static void print_annual_temp_table_values(History *history, MonthlyNormals *monthly_normals);
// helper function for print_annual_pctl()
static void print_annual_pctl(History *history, MonthlyNormals *monthly_normals, int var, int pctl);
// helper functions for daily_normals_in_month()
static void print_daily_normals(DailyNormals *daily_normals, int month, int num_days);
static void print_monthly_record_extrema(History *history, int month, int extreme, int var);
// helper functions for hourly_normals_in_day()
static void print_daily_record_extrema(History *history, int month, int day, int extreme, int var);
static void print_date(int month, int day);
static void print_hour_range(int hour);
static void print_hourly_value(HourlyNormals *hourly_normals, int month, int day, int var, int i, int j);
static void print_hourly_summary(HourlyNormals *hourly_normals, int var, int month, int day);
// helper functions for probs_extreem_temp and simulate_extreme_temps()
static int monthly_quality_check(MonthlyNormals *monthly_normals, int var, int month);
static int yearly_quality_check(MonthlyNormals *monthly_normals, int var);
static int ensure_quality(MonthlyNormals *monthly_normals, int yearly, int month, int var);
// helper function for print_dewp_annual_table
static void print_annual_dewp_line(HourlyNormals *hourly_normals, int var);
// helper function for select_month()
void print_month_list(void);

// run the main menu
void main_menu(void) {
	char WBAN[WBAN_LENGTH + 1], station_name[STATION_NAME_LENGTH];
	while (select_station(WBAN, station_name) == SELECT_STATION_FAILURE);
	
	History *history = create_history(WBAN);
	MonthlyNormals *monthly_normals = create_monthly_normals(WBAN);
	DailyNormals *daily_normals = create_daily_normals(WBAN);
	HourlyNormals *hourly_normals = create_hourly_normals(WBAN);

	char selection[BUFFER];
	while (TRUE) {
		print_main_menu();
		fgets(selection, BUFFER, stdin);
		if (!is_int(selection)) {
			printf("\nInvalid input.");
			continue;
		}
		execute_main_menu_selection(to_int(selection), WBAN, station_name, history, monthly_normals, daily_normals, hourly_normals);
	}
}

void print_main_menu(void) {
	printf("\nPlease enter an integer from 1 to 15 that corresponds to a choice in the menu.\n\n");
	printf("  1 - Specific Year's Temperatures Lookup\n");
	printf("  2 - Specific Month's Temperatures Lookup\n");
	printf("  3 - Temperature Averages and Records Annual Table\n");
	printf("  4 - Temperature Percentiles Annual Table\n");
	printf("  5 - Daily Normals in a Month\n");
	printf("  6 - Hourly Normals in a Day\n");
	printf("  7 - Probability of Extreme Temperatures\n");
	printf("  8 - Simulation of Monthly and Yearly Extreme Temperatures\n");
	printf("  9 - Dew Point Summarized Annual Table\n");
	printf(" 10 - Monthly Dew Point Details\n");
	printf(" 11 - Daily Dew Point Details\n");
	printf(" 12 - Change Station\n");
	printf(" 13 - Works Cited\n");
	printf(" 14 - Quit\n");
}

void execute_main_menu_selection(int selection, char *WBAN, char *station_name, History *history, MonthlyNormals *monthly_normals, 
	DailyNormals *daily_normals, HourlyNormals *hourly_normals) {
	switch (selection) {
	case 1:
		year_lookup(history, monthly_normals);
		break;
	case 2:
		month_lookup(history, monthly_normals);
		break;
	case 3:
		temp_annual_table(history, monthly_normals);
		break;
	case 4:
		pctl_table(history, monthly_normals);
		break;
	case 5:
		daily_normals_in_month(history, monthly_normals, daily_normals);
		break;
	case 6:
		hourly_normals_in_day(history, hourly_normals);
		break;
	case 7:
		probs_extreme_temp(history, monthly_normals);
		break;
	case 8:
		simulate_extreme_temps(history, monthly_normals);
		break;
	case 9:
		print_dewp_annual_table(hourly_normals);
		break;
	case 10:
		monthly_dewp_details(hourly_normals);
		break;
	case 11:
		daily_dewp_details(hourly_normals);
		break;
	case 12:
		destroy_info(history, monthly_normals, daily_normals, hourly_normals);
		main_menu();
		return;
	case 13:
		print_works_cited();
		break;
	case 14:
		destroy_info(history, monthly_normals, daily_normals, hourly_normals);
		exit(EXIT_SUCCESS);
		break;
	default:
		printf("\nInvalid input.");
		return;
	}
	
	if (retract_or_redo() == 1)
		execute_main_menu_selection(selection, WBAN, station_name, history, monthly_normals, daily_normals, hourly_normals);
	else
		return;
}

void destroy_info(History *history, MonthlyNormals *monthly_normals, DailyNormals *daily_normals,
	HourlyNormals *hourly_normals) {
	history = destroy_history(history);
	monthly_normals = destroy_monthly_normals(monthly_normals);
	daily_normals = destroy_daily_normals(daily_normals);
	hourly_normals = destroy_hourly_normals(hourly_normals);
}

static void print_monthly_record_header(int year_month) {
	printf("\n");
	print_formatted_month_year(year_month);
	printf(" Daily Temperature (%cF)", 248);
	printf("\n");
}

static void print_day_numbers(int num_days) {
	int i;
	printf("Date ");
	for (i = 0; i < num_days; i++)
		printf(" %3d ", i + 1);
	printf("\n");
}

static void print_temps(MonthlyRecord *record, int num_days, int var) {
	double total_high = 0;
	int i, num_valid_temps = 0;

	print_high_or_low(var);
	printf(" ");

	for (i = 0; i < num_days; i++) {
		printf(" ");
		print_int(record->temps[var][i]);
		printf(" ");
	}
	printf("\n");
}

// using above helper functions, print MonthlyRecord in a tabular form
static void print_record_table(MonthlyRecord *record, int num_days) {
	print_monthly_record_header(record->year_month);
	print_day_numbers(num_days);
	print_temps(record, num_days, HIGH);
	print_temps(record, num_days, LOW);
}

// calculate necessary statistics to determine the monthly departure
static void calc_departure_stats(MonthlyRecord *record, double *total_avg, int *avg_counter, int num_days) {
	int i;
	for (i = 0; i < num_days; i++) {
		if (!is_invalid(record->avg_temps[i])) {
			*total_avg += record->avg_temps[i];
			*avg_counter += 1;
		}
	}
}

// prints one monthly record
static void print_month(MonthlyRecord *record, MonthlyNormals *monthly_normals) {
	if (!record)
		return;

	int month = month_of_ym(record->year_month);
	int num_days = calc_num_days(record->year_month);

	print_record_table(record, num_days);

	double total_avg = 0;
	int avg_counter = 0;
	calc_departure_stats(record, &total_avg, &avg_counter, num_days);

	if (avg_counter > 0 && !is_invalid(monthly_normals->normals[AVG][month - 1]->value)) {
		double mean_avg = total_avg / (double)avg_counter;
		printf("Departure From Normal: %.1lf\n", mean_avg - monthly_normals->normals[AVG][month - 1]->value);
	}
	else
		printf("Departure From Normal: Unavailable\n");
}

// prints all monthly records available in a requested year
void year_lookup(History *history, MonthlyNormals *monthly_normals) {
	int year = select_year();

	if (!check_bounds_yearly(history, &year, &year)) {
		printf("\nYear out of range.\n");
		printf("Station coverage is from %d to %d.\n", first_year(history), last_year(history));
		return;
	}

	int i;
	for (i = 0; i < 12; i++)
		print_month(search_month(history, year_month_of(year, 1 + i)), monthly_normals);
}

// prints one requested monthly record
void month_lookup(History *history, MonthlyNormals *monthly_normals) {
	int year = select_year(), month = select_month();

	if (!check_bounds_monthly(history, month, &year, &year)) {
		printf("\nYear and month combination out of range.\n");
		printf("Station coverage is from ");
		print_month_from_int(month_of_ym(history->start));
		printf(" %d to ", first_year(history));
		print_month_from_int(month_of_ym(history->end));
		printf(" %d.\n", last_year(history));
		return;
	}

	MonthlyRecord *monthly_record = search_month(history, year_month_of(year, month));
	if (!monthly_record)
		printf("\nYear and month combination out of range.\n");
	else
		print_month(monthly_record, monthly_normals);
}

static void print_record_extrema(RecordExtremaTable *record_extrema_table, int extreme, int var) {
	printf("Record ");
	print_maximum_or_minimum(extreme);
	printf(" ");
	print_high_or_low(var);
	printf(" ");

	int i;
	for (i = 0; i < 12; i++)
		if (extreme == MAX) {
			printf("    ");
			print_int(record_extrema_table->monthly_max[var][i]);
			printf(" ");
		}
		else {
			printf("    ");
			print_int(record_extrema_table->monthly_min[var][i]);
			printf(" ");
		}

	if (extreme == MAX) {
		printf("     ");
		print_int(record_extrema_table->yearly_max[var]);
		printf("\n\n");
	}
	else {
		printf("     ");
		print_int(record_extrema_table->yearly_min[var]);
		printf("\n\n");
	}
}

static void print_mean_extrema(History *history, MonthlyNormals *monthly_normals, int extreme, int var) {
	int i;
	
	printf("Mean ");
	print_maximum_or_minimum(extreme);
	printf(" ");
	print_high_or_low(var);
	printf("   ");

	for (i = 0; i < 12; i++)
		if (extreme == MAX) {
			printf("  ");
			double tentative_value = monthly_mean_maximum(history, i + 1, 1981, 2010, var);
			print_double(pass_monthly_quality_check(monthly_normals, i + 1, var, 'P') ? tentative_value : UNDEFINED);
			printf(" ");
		}
		else {
			printf("  ");
			double tentative_value = monthly_mean_minimum(history, i + 1, 1981, 2010, var);
			print_double(pass_monthly_quality_check(monthly_normals, i + 1, var, 'P') ? tentative_value : UNDEFINED);
			printf(" ");
		}

	if (extreme == MAX) {
		printf("   ");
		double tentative_value = yearly_mean_maximum(history, 1981, 2010, var);
		print_double(pass_yearly_quality_check(monthly_normals, var, 'P') ? tentative_value : UNDEFINED);
		printf("\n\n");
	}
	else {
		printf("   ");
		double tentative_value = yearly_mean_minimum(history, 1981, 2010, var);
		print_double(pass_yearly_quality_check(monthly_normals, var, 'P') ? tentative_value : UNDEFINED);
		printf("\n\n");
	}
}

static void print_monthly_normals(MonthlyNormals *monthly_normals, int var) {
	int i;

	switch (var) {
	case HIGH:
		printf("Average High        ");
		break;
	case LOW:
		printf("Average Low         ");
		break;
	case AVG:
		printf("Daily Mean          ");
		break;
	}

	for (i = 0; i < 12; i++) {
		printf("  ");
		print_double(monthly_normals->normals[var][i]->value);
		printf(" ");
	}
	
	printf("   ");
	print_double(yearly_normal(monthly_normals, var));
	printf(" \n\n");
}

static void print_annual_temp_table_values(History *history, MonthlyNormals *monthly_normals) {
	RecordExtremaTable *record_extrema_table = create_record_extrema_table(history);

	print_record_extrema(record_extrema_table, MAX, HIGH);
	print_record_extrema(record_extrema_table, MAX, LOW);
	print_mean_extrema(history, monthly_normals, MAX, HIGH);
	print_mean_extrema(history, monthly_normals, MAX, LOW);
	print_monthly_normals(monthly_normals, HIGH);
	print_monthly_normals(monthly_normals, AVG);
	print_monthly_normals(monthly_normals, LOW);
	print_mean_extrema(history, monthly_normals, MIN, HIGH);
	print_mean_extrema(history, monthly_normals, MIN, LOW);
	print_record_extrema(record_extrema_table, MIN, HIGH);
	print_record_extrema(record_extrema_table, MIN, LOW);

	destroy_record_extrema_table(record_extrema_table);
}

// using helper functions above, prints normals, mean extrema, and record extrema in a table
void temp_annual_table(History *history, MonthlyNormals *monthly_normals) {
	printf("\nMonth                   ");
	printf("Jan     Feb     Mar     Apr     May     Jun     Jul     Aug     Sep     Oct     Nov     Dec     Year\n\n");
	
	print_annual_temp_table_values(history, monthly_normals);

	printf("Averages and Means: 1981 to 2010 | Records: %d to %d\n", first_year(history), last_year(history));
}

static void print_annual_pctl(History *history, MonthlyNormals *monthly_normals, int var, int pctl) {
	int i;

	printf("%2dth Percentile ", pctl);
	print_high_or_low(var);

	for (i = 0; i < 12; i++) {
		printf("    ");
		int tentative_value = pctl_monthly(history, i + 1, 1981, 2010, var, pctl);
		print_int(pass_monthly_quality_check(monthly_normals, i + 1, var, 'R') ? tentative_value : UNDEFINED);
		printf(" ");
	}

	printf("     ");
	int tentative_value = pctl_yearly(history, 1981, 2010, var, pctl);
	print_int(pass_yearly_quality_check(monthly_normals, var, 'R') ? tentative_value : UNDEFINED);
	printf(" \n\n");
}

// using helper functions above, prints the percentile values in a table for either high or low temperature
void pctl_table(History *history, MonthlyNormals *monthly_normals) {
	int var = high_or_low();

	printf("\nMonth                   Jan     Feb     Mar     Apr     May     Jun     Jul     Aug     Sep     Oct     Nov     Dec     Year\n\n");
	
	print_annual_pctl(history, monthly_normals, var, 99);
	print_annual_pctl(history, monthly_normals, var, 95);
	print_annual_pctl(history, monthly_normals, var, 90);
	print_annual_pctl(history, monthly_normals, var, 75);
	print_annual_pctl(history, monthly_normals, var, 50);
	print_annual_pctl(history, monthly_normals, var, 25);
	print_annual_pctl(history, monthly_normals, var, 10);
	print_annual_pctl(history, monthly_normals, var, 5);
	print_annual_pctl(history, monthly_normals, var, 1);
	printf("Percentiles: 1981 to 2010 | Degrees Fahrenheit\n");
}

static void print_daily_normals(DailyNormals *daily_normals, int month, int num_days) {
	printf("Day     High      Avg      Low\n");

	int i;
	for (i = 0; i < num_days; i++) {
		printf("%3d", i + 1);
		printf("    ");
		print_double(daily_normals->normals[HIGH][month - 1][i]->value);
		printf("    ");
		print_double(daily_normals->normals[AVG][month - 1][i]->value);
		printf("    ");
		print_double(daily_normals->normals[LOW][month - 1][i]->value);
		printf("\n");
	}
	printf("\n");
}

static void print_monthly_record_extrema(History *history, int month, int extreme, int var) {
	if (extreme == MAX) {
		RecordMaximum *record_max = record_maximum_monthly(history, month, first_year(history), last_year(history), var);
		
		printf("Record Maximum ");
		print_high_or_low(var);
		printf(" ");

		print_int(record_max->value);
		printf(" (%d)\n", record_max->year);

		destroy_record_maximum(record_max);
	}
	else {
		RecordMinimum *record_min = record_minimum_monthly(history, month, first_year(history), last_year(history), var);

		printf("Record Minimum ");
		print_high_or_low(var);
		printf(" ");

		print_int(record_min->value);
		printf(" (%d)\n", record_min->year);

		destroy_record_minimum(record_min);
	}
}

// prints the day-by-day average low, average, and high temperatures followed by record monthly extrema values
void daily_normals_in_month(History *history, MonthlyNormals *monthly_normals, DailyNormals *daily_normals) {
	int month = select_month();
	print_month_from_int(month);

	printf(" Daily Temperature Normals (30-Year Climatological Averages)");
	printf("\n");

	print_daily_normals(daily_normals, month, calc_num_days(year_month_of(LEAP_YEAR, month)));

	print_monthly_record_extrema(history, month, MAX, HIGH);
	print_monthly_record_extrema(history, month, MAX, LOW);
	print_monthly_record_extrema(history, month, MIN, HIGH);
	print_monthly_record_extrema(history, month, MIN, LOW);

	printf("\nNormals: 1981 to 2010 | Records: %d to %d | Degrees Fahrenheit\n", first_year(history), last_year(history));
}

static void print_daily_record_extrema(History *history, int month, int day, int extreme, int var) {
	if (extreme == MAX) {
		RecordMaximum *record_max = record_maximum_daily(history, month, day, first_year(history), last_year(history), var);

		printf("Record Maximum ");
		print_high_or_low(var);

		print_int(record_max->value);
		printf(" (%d)\n", record_max->year);

		destroy_record_maximum(record_max);
	}
	else {
		RecordMinimum *record_min = record_minimum_daily(history, month, day, first_year(history), last_year(history), var);

		printf("Record Minimum ");
		print_high_or_low(var);

		print_int(record_min->value);
		printf(" (%d)\n", record_min->year);

		destroy_record_minimum(record_min);
	}
}

static void print_date(int month, int day) {
	print_month_from_int(month);
	printf(" %d", day);
}

static void print_hour_range(int hour) {
	char start_o_clock[6], end_o_clock[6];
	int pm = hour / 12;

	if (!(hour % 12))
		sprintf(start_o_clock, "%02d", 12);
	else
		sprintf(start_o_clock, "%02d", hour % 12);
	strcat(start_o_clock, ":00");

	strncpy(end_o_clock, start_o_clock, 3);
	end_o_clock[3] = '\0';
	strcat(end_o_clock, "59");

	printf("%s-%s", start_o_clock, end_o_clock);
	printf(" %s", pm ? "PM" : "AM");
}

static void print_hourly_value(HourlyNormals *hourly_normals, int month, int day, int var, int i, int j) {
	double val;
	if (var == TEMP)
		val = hourly_normals->temp[j][month_day_to_int(month, day) - 1][i]->value;
	else
		val = hourly_normals->dewp[j][month_day_to_int(month, day) - 1][i]->value;

	printf("\t ");
	print_double(val);
}

static void print_hourly_summary(HourlyNormals *hourly_normals, int var, int month, int day) {
	int i, j;

	printf("Hour\t\t10pctl\tNormal\t90pctl\n");

	for (i = 0; i < 24; i++) {
		print_hour_range(i);
		for (j = 0; j < 3; j++)
			print_hourly_value(hourly_normals, month, day, var, i, j);
		printf("\n");
	}
}

// using helper functions above, prints hourly 10th percentile, normal, and 90th percentile temperature values
// followed by daily record extrema
void hourly_normals_in_day(History *history, HourlyNormals *hourly_normals) {
	int month = select_month(), day = select_day(month);

	printf("\n");
	print_date(month, day);
	printf(" Hourly Temperature Normals (30-Year Climatological Averages)\n");
	printf("10th Percentile to 90th Percentile Intervals\n\n");

	print_hourly_summary(hourly_normals, TEMP, month, day);

	print_daily_record_extrema(history, month, day, MAX, HIGH);
	print_daily_record_extrema(history, month, day, MAX, LOW);
	print_daily_record_extrema(history, month, day, MIN, HIGH);
	print_daily_record_extrema(history, month, day, MIN, LOW);

	printf("\nNormals: 1981 to 2010 | Records: %d to %d | Degrees Fahrenheit\n", first_year(history), last_year(history));
}

// must be enough data in the month for its normal to at least be "standard" (marked as 'S')
static int monthly_quality_check(MonthlyNormals *monthly_normals, int var, int month) {
	return pass_monthly_quality_check(monthly_normals, month, var, 'S');
}

// must be enough data in the year for its normal to at least be "standard" (marked as 'S')
static int yearly_quality_check(MonthlyNormals *monthly_normals, int var) {
	return pass_yearly_quality_check(monthly_normals, var, 'S');
}

// ensuring high quality of data for more involved statistical analyses
static int ensure_quality(MonthlyNormals *monthly_normals, int yearly, int month, int var) {
	if (!yearly) {
		if (!monthly_quality_check(monthly_normals, var, month)) {
			printf("\nNot enough station data to execute this option for ");
			print_month_from_int(month);
			printf(" ");
			print_high_or_low(var);
			printf("\n");
			return 0;
		}
	}
	else {
		if (!yearly_quality_check(monthly_normals, var)) {
			printf("\nNot enough station data to execute this option for Whole Year ");
			print_high_or_low(var);
			printf("\n");
			return 0;
		}
	}
	
	return 1;
}

// prints probabilities of occurrences in increasing time periods for a given extreme temperature value
void probs_extreme_temp(History *history, MonthlyNormals *monthly_normals) {
	int month = 0, yearly = yearly_or_monthly();
	if (!yearly)
		month = select_month();

	int var = high_or_low();

	if (!ensure_quality(monthly_normals, yearly, month, var))
		return;

	int less_than = greater_or_less_than();
	int val = enter_temp_value();
	
	if (!yearly) 
		if (!less_than)
			print_prob_exeed_val_intervals_monthly(history, month, var, val);
		else
			print_prob_under_val_intervals_monthly(history, month, var, val);
	else 
		if (!less_than)
			print_prob_exeed_val_intervals_yearly(history, var, val);
		else
			print_prob_under_val_intervals_yearly(history, var, val);
}

// simulates mmaximum and minimum monthly high or low temperatures
void simulate_extreme_temps(History *history, MonthlyNormals *monthly_normals) {
	int month = 0, yearly = yearly_or_monthly();
	if (!yearly)
		month = select_month();

	int var = high_or_low();

	if (!ensure_quality(monthly_normals, yearly, month, var))
		return;

	int extremum = max_or_min();

	if (!yearly)
		if (extremum == MAX)
			simulate_monthly_maximum(history, month, var);
		else
			simulate_monthly_minimum(history, month, var);
	else
		if (extremum == MAX)
			simulate_yearly_maximum(history, var);
		else
			simulate_yearly_minimum(history, var);
}

// prints hour-by-hour average dew point values in the requested day
void daily_dewp_details(HourlyNormals *hourly_normals) {
	int month = select_month(), day = select_day(month);
	printf("\n");
	print_date(month, day);
	printf(" Hourly Dew Point Normals (30-Year Climatological Averages)\n");
	printf("10th Percentile to 90th Percentile Intervals\n\n");
	print_hourly_summary(hourly_normals, DEWP, month, day);
	printf("\n");
	printf("Normals: 1981 to 2010 | Degrees Fahrenheit\n");
}

// prints day-by-day average dew point values in the requested month
void monthly_dewp_details(HourlyNormals *hourly_normals) {
	int i;
	int month = select_month();
	print_month_from_int(month);
	printf(" Daily Dew Point Normals (30-Year Climatological Averages)");
	printf("\n");

	int num_days = calc_num_days(year_month_of(NON_LEAP_YEAR, month));

	printf("Day   10pctl   Normal   90pctl\n");
	for (i = 0; i < num_days; i++) {
		printf("%3d", i + 1);
		printf("    ");
		print_double(daily_avg_dewp(hourly_normals, month, i + 1, TEN_PCTL));
		printf("    ");
		print_double(daily_avg_dewp(hourly_normals, month, i + 1, NORMAL_HLY));
		printf("    ");
		print_double(daily_avg_dewp(hourly_normals, month, i + 1, NINETY_PCTL));
		printf("\n");
	}

	printf("\nNormals: 1981 to 2010 | Degrees Fahrenheit\n");
}

static void print_annual_dewp_line(HourlyNormals *hourly_normals, int var) {
	printf("Hourly Mean ");
	switch (var) {
	case NINETY_PCTL:
		printf("90pctl  ");
		break;
	case NORMAL_HLY:
		printf("Normal  ");
		break;
	case TEN_PCTL:
		printf("10pctl  ");
		break;
	default:
		break;
	}

	int i;
	for (i = 0; i < 12; i++)
		printf("  %5.1lf ", monthly_avg_dewp(hourly_normals, i + 1, var));
	printf("   %5.1lf \n\n", yearly_avg_dewp(hourly_normals, var));
}

// prints an annual table summarizing dew point values
void print_dewp_annual_table(HourlyNormals *hourly_normals) {
	printf("\nMonth                   Jan     Feb     Mar     Apr     May     Jun     Jul     Aug     Sep     Oct     Nov     Dec     Year\n\n");
	print_annual_dewp_line(hourly_normals, NINETY_PCTL);
	print_annual_dewp_line(hourly_normals, NORMAL_HLY);
	print_annual_dewp_line(hourly_normals, TEN_PCTL);
	printf("Normals: 1981 to 2010 | Degrees Fahrenheit\n");
}

void print_works_cited(void) {
	printf("\n");
	printf("Menne, M.J., I. Durre, R.S. Vose, B.E. Gleason, and T.G. Houston, 2012:  An overview\n");
	printf("of the Global Historical Climatology Network - Daily Database.  Journal of Atmospheric\n");
	printf("and Oceanic Technology, 29, 897 - 910, doi:10.1175 / JTECH - D - 11 - 00103.1.\n");
	printf("\n");

	printf("Menne, M.J., I.Durre, B.Korzeniewski, S.McNeal, K.Thomas, X.Yin, S.Anthony, R.Ray,\n");
	printf("R.S.Vose, B.E.Gleason, and T.G.Houston, 2012: Global Historical Climatology Network -\n");
	printf("Daily(GHCN - Daily), Version 3.25\n");
	printf("NOAA National Climatic Data Center.http ://doi.org/10.7289/V5D21VHZ 6 Jun. 2018.\n");
	printf("\n");

	printf("Anthony Arguez, Imke Durre, Scott Applequist, Mike Squires, Russell Vose, Xungang Yin, and Rocky Bilotta(2010).\n");
	printf("NOAA's U.S. Climate Normals (1981-2010). Hourly.\n");
	printf("NOAA National Centers for Environmental Information. DOI:10.7289/V5PN93JP 27 Dec. 2017.\n");
	printf("\n");

	printf("Anthony Arguez, Imke Durre, Scott Applequist, Mike Squires, Russell Vose, Xungang Yin, and Rocky Bilotta(2010).\n");
	printf("NOAA's U.S. Climate Normals (1981-2010). Temperature.\n");
	printf("NOAA National Centers for Environmental Information. DOI:10.7289/V5PN93JP 9 Jul. 2018.\n");
}

// go back to main menu or repeat the same menu selection
int retract_or_redo(void) {
	char buffer[BUFFER];

	printf("\nEnter '1' to repeat the same option or '2' to go back to the main menu.\n");
	fgets(buffer, BUFFER, stdin);

	while (!is_int(buffer) || (to_int(buffer) != 1 && to_int(buffer) != 2)) {
		printf("\nImproper input.");
		printf("\nEnter '1' to repeat the same option or '2' to go back to the main menu.\n");
		fgets(buffer, BUFFER, stdin);
	}

	return to_int(buffer);
}

// user prompt for high or low temperature
int high_or_low(void) {
	char buffer[BUFFER];

	printf("\n1 - High Temperature\n2 - Low Temperature\n");
	fgets(buffer, BUFFER, stdin);

	while (!is_int(buffer) || (to_int(buffer) != 1 && to_int(buffer) != 2)) {
		printf("\nImproper input.");
		printf("\n1 - High Temperature\n2 - Low Temperature\n");
		fgets(buffer, BUFFER, stdin);
	}

	return to_int(buffer) - 1;
}

// user prompt for yearly or monthly time period
int yearly_or_monthly(void) {
	char buffer[BUFFER];

	printf("\n1 - Monthly\n2 - Yearly\n");
	fgets(buffer, BUFFER, stdin);

	while (!is_int(buffer) || (to_int(buffer) != 1 && to_int(buffer) != 2)) {
		printf("\nImproper input.");
		printf("\n1 - Monthly\n2 - Yearly\n");
		fgets(buffer, BUFFER, stdin);
	}

	return to_int(buffer) - 1;
}

// user prompt for maximum or minimum
int max_or_min(void) {
	char buffer[BUFFER];

	printf("\n1 - Maximum\n2 - Minimum\n");
	fgets(buffer, BUFFER, stdin);

	while (!is_int(buffer) || (to_int(buffer) != 1 && to_int(buffer) != 2)) {
		printf("\nImproper input.");
		printf("\n1 - Maximum\n2 - Minimum\n");
		fgets(buffer, BUFFER, stdin);
	}

	return to_int(buffer) - 1;
}

// user prompt for temperature value
int enter_temp_value(void) {
	char buffer[BUFFER];

	printf("Enter an integer for the temperature value: ");
	fgets(buffer, BUFFER, stdin);

	while (!is_int(buffer)) {
		printf("Invalid input.  Enter an integer for the temperature value: ");
		fgets(buffer, BUFFER, stdin);
	}

	return to_int(buffer);
}

// user prompt for analyzing greater than or equal, or less than, probabilities
int greater_or_less_than(void) {
	char buffer[BUFFER];

	printf("\n1 - Greather Than or Equal\n2 - Less Than\n");
	fgets(buffer, BUFFER, stdin);

	while (!is_int(buffer) || (to_int(buffer) != 1 && to_int(buffer) != 2)) {
		printf("\nImproper input.");
		printf("\n1 - Greather Than or Equal\n2 - Less Than\n");
		fgets(buffer, BUFFER, stdin);
	}

	printf("\n");
	return to_int(buffer) - 1;
}

static void print_month_list(void) {
	int i;

	printf("\n");
	for (i = 0; i < 12; i++) {
		printf("%2d - ", i + 1);
		print_month_from_int(i + 1);
		printf("\n");
	}
}

// user prompt for month selection
int select_month(void) {
	char buffer[BUFFER];

	print_month_list();
	printf("Enter the number that corresponds to your choice of month: ");
	fgets(buffer, BUFFER, stdin);

	while (!is_int(buffer) || (to_int(buffer) < 1 || to_int(buffer) > 12)) {
		print_month_list();
		printf("Improper input.  Enter a number from 1 to 12.\n");
		printf("Enter the number that corresponds to your choice of month: ");
		fgets(buffer, BUFFER, stdin);
	}

	printf("\n");
	return to_int(buffer);
}

// user prompt for day selection
int select_day(int month) {
	char buffer[BUFFER];
	int max_days = calc_num_days(year_month_of(NON_LEAP_YEAR, month));

	printf("Enter an integer from 1 to %d the day to search: ", max_days);
	fgets(buffer, BUFFER, stdin);

	while (!is_int(buffer) || to_int(buffer) < 1 || to_int(buffer) > max_days) {
		printf("Invalid input.  Enter an integer from 1 to %d for the day to search: ", max_days);
		fgets(buffer, BUFFER, stdin);
	}

	return to_int(buffer);
}

// user prompt for year selection
int select_year(void) {
	char buffer[BUFFER];

	printf("\nEnter the year to search: ");
	fgets(buffer, BUFFER, stdin);

	while (!is_int(buffer)) {
		printf("Invalid input.  Enter an integer for the year to search: ");
		fgets(buffer, BUFFER, stdin);
	}

	return to_int(buffer);
}