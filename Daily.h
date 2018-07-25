#ifndef DAILY_H
#define DAILY_H

#include "Auxiliary.h"

typedef struct {
	Normal *temp[3][365][24];
	Normal *dewp[3][365][24];
} HourlyNormals;

typedef struct {
	Normal *normals[3][12][31];
	Normal *stddev[3][12][31];
} DailyNormals;

HourlyNormals *create_hourly_normals(char *WBAN);
HourlyNormals *destroy_hourly_normals(HourlyNormals *hourly_normals);
DailyNormals *create_daily_normals(char *WBAN);
DailyNormals *destroy_daily_normals(DailyNormals *daily_normals);
RecordMaximum *record_maximum_daily(History *history, int month, int day, int start_year, int end_year, int var);
RecordMinimum *record_minimum_daily(History *history, int month, int day, int start_year, int end_year, int var);
double daily_avg_dewp(HourlyNormals *hourly_normals, int month, int day, int var);

#endif