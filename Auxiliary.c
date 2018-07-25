#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <ctype.h>
#include "Auxiliary.h"
#include "Constants.h"

// helper functions for replace_key()
static int key_match(char *dynamic_string, const char *key, int key_length);
static int locate_key(char *dynamic_string, const char *key, int dyamic_string_length, int key_length);
static void grow_dynamic_string(char **dynamic_string, int dynamic_string_length, int diff);
static void make_room(char **dynamic_string, int dynamic_string_length, int key_location, int key_length, int diff);
static void replace(char *dynamic_string, const char *str, int str_length, int key_location);
static void gap_close(char *dynamic_string, int dynamic_string_length, int key_location, int str_length, int diff);
// helper functions for bucket_sort()
static int *create_bucket(int *arr, int size, int range_length, int min);
static void refill_arr(int *arr, int *bucket, int range_length, int min);

// returs 1 if the prefix of 'dynamic_string' is the same as 'key', 0 otherwise
static int key_match(char *dynamic_string, const char *key, int key_length) {
	int i;
	for (i = 0; i < key_length; i++)
		if (dynamic_string[i] != key[i])
			return 0;
	
	return 1;
}

// returns the index at which 'key' starts in dynamic_string, -1 if nowhere
static int locate_key(char *dynamic_string, const char *key, int dynamic_string_length, int key_length) {
	if (key_length <= 0)
		return -1;
	
	int i;
	for (i = 0; i < dynamic_string_length; i++)
		if (key_match(dynamic_string + i, key, key_length)) 
			return i;

	return -1;
}

// increases the size of 'dynamic_string' by 'diff'
static void grow_dynamic_string(char **dynamic_string, int dynamic_string_length, int diff) {
	char *temp = realloc(*dynamic_string, sizeof(char) * (dynamic_string_length + diff + 1));
	if (!temp) {
		free(*dynamic_string);
		printf("Ran out of memory!\n");
		exit(EXIT_FAILURE);
	}

	*dynamic_string = temp;
}

// shifts characters from 'key_location' to the right by 'key_legnth' 
static void make_room(char **dynamic_string, int dynamic_string_length, int key_location, int key_length, int diff) {
	if (diff <= 0)
		return;

	grow_dynamic_string(dynamic_string, dynamic_string_length, diff);

	int i;
	for (i = dynamic_string_length - 1; i >= key_location + key_length; i--)
		*(*dynamic_string + i + diff) = *(*dynamic_string + i);

	*(*dynamic_string + dynamic_string_length + diff) = '\0';
}

// plants 'str' at 'key_location' in 'dynamic_string'
static void replace(char *dynamic_string, const char *str, int str_length, int key_location) {
	if (str == NULL || str_length <= 0)
		return;
	
	int i;
	for (i = 0; i < str_length; i++)
		*(dynamic_string + key_location + i) = *(str + i);
}

// shortens 'dynamic_string' by diff
static void gap_close(char *dynamic_string, int dynamic_string_length, int key_location, int str_length, int diff) {
	if (diff >= 0)
		return;
	
	int i;
	for (i = key_location + str_length; i < dynamic_string_length; i++)
		*(dynamic_string + i) = *(dynamic_string + i - diff);

	*(dynamic_string + dynamic_string_length + diff) = '\0';	
}

// replaces all occurrences of 'key' in 'dynamic_string' with 'str'
void replace_key(char *dynamic_string, const char *key, const char *str) {
    if (dynamic_string == NULL || key == NULL || str == NULL)
        return;

    // key_location set to -1 by default - represents no occurrences of 'key' in 'dynamic_string'
    int key_location = -1, key_length = strlen(key);
    int str_length = strlen(str);
    int dynamic_string_length = strlen(dynamic_string), diff = str_length - key_length;

    // inspect from each character for an occurrence of 'key'
	key_location = locate_key(dynamic_string, key, dynamic_string_length, key_length);
	if (key_location == -1)
		return;

	// key replacement process
	make_room(&dynamic_string, dynamic_string_length, key_location, key_length, diff);
	replace(dynamic_string, str, str_length, key_location);
	gap_close(dynamic_string, dynamic_string_length, key_location, str_length, diff);

    // recursively call until all occurrences of 'key' have been replaced with 'str'
    replace_key(dynamic_string, key, str);
}

// dynamically allocates an integer array 'bucket' and returns its pointer
static int *create_bucket(int *arr, int size, int range_length, int min) {
	int i, *bucket = calloc(range_length, sizeof(int));
	if (!bucket) {
		free(bucket);
		printf("Ran out of memory!\n");
		exit(EXIT_FAILURE);
	}

	for (i = 0; i < size; i++)
		bucket[arr[i] - min]++;

	return bucket;
}

// refills 'arr' using 'bucket'
static void refill_arr(int *arr, int *bucket, int range_length, int min) {
	int i, index = 0;
	for (i = 0; i < range_length; i++)
		while (bucket[i]-- > 0)
			arr[index++] = i + min;
}

void bucket_sort(int *arr, int size) {
	if (size < 2)
		return;

	// find max and min values in array
	int i, min = INT_MAX, max = INT_MIN;
	for (i = 0; i < size; i++) {
		max = max_int(max, arr[i]);
		min = min_int(min, arr[i]);
	}

	int range_length = max - min + 1;
	int *bucket = create_bucket(arr, size, range_length, min);
	refill_arr(arr, bucket, range_length, min);

	free(bucket);
}

// returns 1 if the 'str' could have been an integer, 0 otherwise
int is_int(char *str) {
    if (!str || !*str)
        return 0;

    while (*str != '\0') {
        if(!isdigit(*str) && *str != '-' && *str != '\n')
            return 0;
        str++;
    }

    return 1;
}

int to_int(char *str) {
    char *ptr = NULL;
    return (int)strtol(str, &ptr, 10);
}

double to_double(char *str) {
    char *ptr = NULL;
	return strtod(str, &ptr);
}

double avg(int *arr, int size) {
	int i;
	double sum = 0;

	for (i = 0; i < size; i++)
		sum += (double)arr[i];

	return ((double)sum) / ((double)size);
}

double stddev(int *arr, int size) {
	int i;
	double mean = avg(arr, size), sum = 0;

	for (i = 0; i < size; i++)
		sum += pow(((double)arr[i] - mean), 2);

	sum /= ((double)(size - 1));
	sum = pow(sum, 0.5);

	return sum;
}

int max_int(int a, int b) {
    return (a > b) ? a : b;
}

int min_int(int a, int b) {
    return (a < b) ? a : b;
}

double max_double(double a, double b) {
	return (a > b) ? a : b;
}

double min_double(double a, double b) {
	return (a < b) ? a : b;
}

int is_invalid(double value) {
	return (value == UNAVAILABLE || value == UNDEFINED) ? 1 : 0;
}

void print_int(int value) {
	(!is_invalid(value)) ? printf("%3d", value) : printf(" --");
}

void print_double(double value) {
	(!is_invalid(value)) ? printf("%5.1lf", value) : printf(" ----");
}

void print_high_or_low(int var) {
	!var ? printf("High") : printf("Low ");
}

void print_maximum_or_minimum(int extreme) {
	(extreme == MAX) ? printf("Maximum") : printf("Minimum");
}

RecordMaximum *destroy_record_maximum(RecordMaximum *record_maximum) {
	free(record_maximum);
	record_maximum = NULL;
	return record_maximum;
}

RecordMinimum *destroy_record_minimum(RecordMinimum *record_minimum) {
	free(record_minimum);
	record_minimum = NULL;
	return record_minimum;
}

void delay(double seconds) {
	clock_t start_time = clock();
	while (clock() < start_time + 1000 * seconds);
}