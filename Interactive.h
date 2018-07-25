#include "MonthlyTempRecords.h"
#include "Yearly.h"
#include "Monthly.h"
#include "Daily.h"

void main_menu(void);
void print_main_menu(void);
void execute_main_menu_selection(int selection, char *WBAN, char *station_name, History *history, MonthlyNormals *monthly_normals,
	DailyNormals *daily_normals, HourlyNormals *hourly_normals);
void destroy_info(History *history, MonthlyNormals *monthly_normals, DailyNormals *daily_normals,
	HourlyNormals *hourly_normals);
void year_lookup(History *history, MonthlyNormals *monthly_normals);
void month_lookup(History *history, MonthlyNormals *monthly_normals);
void temp_annual_table(History *history, MonthlyNormals *monthly_normals);
void pctl_table(History *history, MonthlyNormals *monthly_normals);
void daily_normals_in_month(History *history, MonthlyNormals *monthly_normals, DailyNormals *daily_normals);
void hourly_normals_in_day(History *history, HourlyNormals *hourly_normals);
void probs_extreme_temp(History *history, MonthlyNormals *monthly_normals);
void simulate_extreme_temps(History *history, MonthlyNormals *monthly_normals);
void daily_dewp_details(HourlyNormals *hourly_normals);
void monthly_dewp_details(HourlyNormals *hourly_normals);
void print_dewp_annual_table(HourlyNormals *hourly_normals);
void print_works_cited(void);
int retract_or_redo(void);
int high_or_low(void);
int yearly_or_monthly(void);
int max_or_min(void);
int enter_temp_value(void);
int greater_or_less_than(void);
int select_month(void);
int select_day(int month);
void print_maximum_or_minimum(int extreme);
int select_year(void);