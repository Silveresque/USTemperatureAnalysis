#ifndef CALENDAR_H
#define CALENDAR_H

int year_month_of(int year, int month);
int year_of_ym(int year_month);
int month_of_ym(int year_month);
int month_day_of(int month, int day);
int month_of_md(int month_day);
int day_of_md(int month_day);
int calc_num_days(int year_month);
void print_formatted_month_year(int year_month);
void print_month_from_int(int month);
int month_day_to_int(int month, int day);

#endif