#include <stdio.h>
#include "Calendar.h""

// returns the year and month joint as an integer (i.e. February 1998 as '199802')
int year_month_of(int year, int month) {
	return year * 100 + month;
}

// returns the year portion of a year_month ('1998' is returned from parameter '199802')
int year_of_ym(int year_month) {
	return year_month / 100;
}

// returns the month portion of a year_month ('2' is returned from parameter '199802')
int month_of_ym(int year_month) {
	return year_month - year_month / 100 * 100;
}

// returns the month and day joint as an integer (i.e. October 24 as '1024')
int month_day_of(int month, int day) {
	return month * 100 + day;
}

// returns the month portion of a month_day ('10' is returned from parameter '1024')
int month_of_md(int month_day) {
	return month_day / month_day;
}

// returns the day portion of a month_day ('24' is returned from parameter '1024')
int day_of_md(int month_day) {
	return month_day - month_day / 100 * 100;
}

// function to calculate the number of days given a year_month
int calc_num_days(int year_month) {
	int year = year_month / 100;
	int month = year_month - year * 100;

	if (month == 1 || month == 3 || month == 5 || month == 7 || month == 8 || month == 10 || month == 12)
		return 31;

	else if (month == 2) {
		if (!(year % 4))
			return 29;
		else return 28;
	}

	else
		return 30;
	return 31;
}

// print year_month in a formatted way
void print_formatted_month_year(int year_month) {
	int year = year_month / 100;
	int month = year_month - year * 100;
	switch (month) {

	case 1:
		printf("January ");
		break;
	case 2:
		printf("February ");
		break;
	case 3:
		printf("March ");
		break;
	case 4:
		printf("April ");
		break;
	case 5:
		printf("May ");
		break;
	case 6:
		printf("June ");
		break;
	case 7:
		printf("July ");
		break;
	case 8:
		printf("August ");
		break;
	case 9:
		printf("September ");
		break;
	case 10:
		printf("October ");
		break;
	case 11:
		printf("November ");
		break;
	case 12:
		printf("December ");
		break;
	default:
		break;
	}

	printf("%d", year);
}

// print just the month name given an integer for the month
void print_month_from_int(int month) {

	switch (month) {

	case 1:
		printf("January");
		break;
	case 2:
		printf("February");
		break;
	case 3:
		printf("March");
		break;
	case 4:
		printf("April");
		break;
	case 5:
		printf("May");
		break;
	case 6:
		printf("June");
		break;
	case 7:
		printf("July");
		break;
	case 8:
		printf("August");
		break;
	case 9:
		printf("September");
		break;
	case 10:
		printf("October");
		break;
	case 11:
		printf("November");
		break;
	case 12:
		printf("December");
		break;
	default:
		break;
	}
}

// returns an integer n where n is the nth day of the year based on month and day
int month_day_to_int(int month, int day) {
	int i, retval = 0;

	for (i = 1; i < month; i++)
		switch (i) {
		case 1:
			retval += 31;
			break;
		case 2:
			retval += 28;
			break;
		case 3:
			retval += 31;
			break;
		case 4:
			retval += 30;
			break;
		case 5:
			retval += 31;
			break;
		case 6:
			retval += 30;
			break;
		case 7:
			retval += 31;
			break;
		case 8:
			retval += 31;
			break;
		case 9:
			retval += 30;
			break;
		case 10:
			retval += 31;
			break;
		case 11:
			retval += 30;
			break;
		case 12:
			retval += 31;
			break;
		default:
			break;
		}

	retval += day;
	return retval;
}