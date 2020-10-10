#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
	int NO;
	struct watchpoint *next;
	/* TODO: Add more members if necessary */
	bool isused;
	int value;
	char expr[32]; 
	int newvalue;

} WP;

WP* new_wp();
int free_wp(int no);
void PrintW();
int set_watchpoint(char *e);
bool delete_watchpoint(int NO);
WP* scan_watchpoint(void);
#endif
