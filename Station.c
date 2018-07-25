#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "Interactive.h"
#include "StringBST.h"
#include "Auxiliary.h"
#include "Calendar.h"
#include "Constants.h"
#include "Station.h"

// helper functions for select_station()
static void select_state(char *state);
static void format_station_name(char **station_name_string);
static int line_matches_state(char *line, char *state);
static char *create_dynamic_string(int size);
static void capture_WBAN(char *line, char *station_WBAN);
static void capture_staiton_name(char *line, char *station_name_string);
static void add_station(char *line, char *station_WBAN, node **station_bst);
static void compile_station_list(node **station_bst, char *state);
static void station_list_choice(node **station_bst, node **selected_station);


static void select_state(char *state) {
	int i;
	char buffer[BUFFER];

	printf("\nWhat U.S. state are you interested in?  Enter two letters (e.g. \"CT\" or \"ct\" for Connecticut).\n");
	fgets(buffer, BUFFER, stdin);

	while (strlen(buffer) > 3) {
		printf("\nInvalid state.\n");
		printf("\nWhat U.S. state are you interested in?  Enter two letters (e.g. \"CT\" or \"ct\" for Connecticut).\n");
		fgets(buffer, BUFFER, stdin);
	}

	for (i = 0; i < 2; i++)
		state[i] = toupper(buffer[i]);
	state[2] = 0;
}

// formatting station names using replaceStr()
static void format_station_name(char **station_name_string) {
	replace_key(station_name_string, " AAF ", " ARMY AIR FORCE ");
	replace_key(station_name_string, " AFB ", " AIR FORCE BASE ");
	replace_key(station_name_string, " AP ", " AIRPORT ");
	replace_key(station_name_string, " CO ", " COUNTY ");
	replace_key(station_name_string, " EXEC ", " EXECUTIVE ");
	replace_key(station_name_string, " FLD ", " FIELD ");
	replace_key(station_name_string, " INTL ", " INTERNATIONAL ");
	replace_key(station_name_string, " MEM ", " MEMORIAL ");
	replace_key(station_name_string, " MUNI ", " MUNICIPAL ");
	replace_key(station_name_string, " NAS ", " NAVAL AIR STATION ");
	replace_key(station_name_string, " NP ", " NATIONAL PARK ");
	replace_key(station_name_string, " RGNL ", " REGIONAL ");
	replace_key(station_name_string, " STN ", " STATION ");
	replace_key(station_name_string, " WSO ", " WEATHER STATION OFFICE ");
	replace_key(station_name_string, "  ", " ");
}

static int line_matches_state(char *line, char *state) {
	return (line[38] == state[0] && line[39] == state[1]) ? 1 : 0;
}

static char *create_dynamic_string(int size) {
	char *dynamic_string = calloc(size + 1, sizeof(char));
	if (!dynamic_string) {
		printf("no more memory\n");
		exit(EXIT_FAILURE);
	}

	return dynamic_string;
}

static void capture_WBAN(char *line, char *station_WBAN) {
	int i;
	for (i = 0; i < WBAN_LENGTH; i++) {
		station_WBAN[i] = line[i];
	}
	station_WBAN[WBAN_LENGTH] = '\0';
}

static void capture_staiton_name(char *line, char *station_name_string) {
	station_name_string[0] = ' ';
	strncpy(station_name_string + 1, line + 41, 31);
	station_name_string[32] = ' ';
	station_name_string[33] = '\0';
}

static void add_station(char *line, char *station_WBAN, node **station_bst) {
	char *station_name_string = create_dynamic_string(STATION_NAME_LENGTH);
	capture_WBAN(line, station_WBAN);
	capture_staiton_name(line, station_name_string);
	format_station_name(&station_name_string);
	*station_bst = BST_insert(*station_bst, station_name_string, station_WBAN);
	free(station_name_string);
	station_name_string = NULL;
}

static void compile_station_list(node **station_bst, char *state) {
	FILE *ifp = fopen("hly-inventory.txt", "r");
	char line[LINE], station_WBAN[WBAN_LENGTH + 1];

	while (fgets(line, LINE, ifp))
		if (line_matches_state(line, state))
			add_station(line, station_WBAN, station_bst);
}

static void station_list_choice(node **station_bst, node **selected_station) {
	if (!*station_bst)
		return;

	// facilitate selection of station
	char buffer[BUFFER];
	while (*selected_station == NULL) {
		printf("Enter the number that corresponds to your station of interest.\n");
		fgets(buffer, BUFFER, stdin);

		*selected_station = BST_search(*station_bst, to_int(buffer));
		if (*selected_station == NULL)
			printf("\nNot a valid number.\n");
	}
}

int select_station(char *WBAN, char *station_name) {
	char state[3];
	node *station_bst = NULL;

	select_state(state);
	compile_station_list(&station_bst, state);

	// if no entries were found
	if (station_bst == NULL) {
		printf("\nInvalid state.\n");
		station_bst = forest_fire(station_bst);
		return SELECT_STATION_FAILURE;
	}
	// otherwise present a list of all the stations calling the inorder traversal function for the BST
	else
		inorder_recursive(station_bst);

	node *selected_station = NULL;
	station_list_choice(&station_bst, &selected_station);
	if (!selected_station) {
		printf("Ran out of memory!\n");
		exit(EXIT_FAILURE);
	}

	// get the WBAN and name of the selected station
	strcpy(WBAN, selected_station->WBAN);
	strcpy(station_name, selected_station->data + 1);
	station_name[strlen(station_name) - 1] = '\0';

	station_bst = forest_fire(station_bst);

	return SELECT_STATION_SUCCESS;
}