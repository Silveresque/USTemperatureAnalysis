#ifndef MONTHLY_TEMP_RECORDS
#define MONTHLY_TEMP_RECORDS

typedef struct {
	int year_month;
    int temps[31];
} ActualTemps;

typedef struct {
    int year_month;
	int temps[2][31];
	double avg_temps[31];
}  MonthlyRecord;

typedef struct {
    MonthlyRecord **monthly_records;
    int size;
	int start;
	int end;
} History;

History *create_history(char *WBAN);
int calc_num_months(int start, int end);
int tenths_celsius_to_fahrenheit(double temp);
MonthlyRecord *search_month(History *history, int year_month);
MonthlyRecord *get_monthly_record(History *history, int index);
int get_month_index(History *history, int year_month);
History *destroy_history(History *history);
int first_year(History *history);
int last_year(History *history);

#endif