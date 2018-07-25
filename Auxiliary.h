#ifndef AUXILIARY_H
#define AUXILIARY_H

typedef struct {
	double value;
	char quality;
} Normal; 

typedef struct {
	int value;
	int year;
} RecordMaximum;

typedef struct {
	int value;
	int year;
} RecordMinimum;

void replace_key(char *dynamic_string, const char *key, const char *str);
void bucket_sort(int *arr, int size);
int is_int(char *str);
int to_int(char *str);
double to_double(char *str);
double avg(int *arr, int size);
double stddev(int *arr, int size);
int max_int(int a, int b);
int min_int(int a, int b);
double max_double(double a, double b);
double min_double(double a, double b);
int is_invalid(double value);
void print_int(int value);
void print_double(double value);
void print_high_or_low(int var);
void print_maximum_or_minimum(int extreme);
RecordMaximum *destroy_record_maximum(RecordMaximum *record_maximum);
RecordMinimum *destroy_record_minimum(RecordMinimum *record_minimum);
void delay(double seconds);

#endif