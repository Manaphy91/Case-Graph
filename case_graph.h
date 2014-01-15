#ifndef CASE_GRAPH_H
#define CASE_GRAPH_H

#include "stack.h"
#include <limits.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <gvc.h>

typedef struct{
	char ID;
	unsigned int pre_condition;
	unsigned int pre_post_compl;
	unsigned int post_condition;
} event;

typedef struct{
	unsigned int global_state;
	stack* event_node_stack;		
} graph_node;

typedef struct{
	char event_ID;
	unsigned int global_state;
} event_node;

void bit_print(unsigned int value);

void initialize_event(event* ev, char ID, unsigned int pre, unsigned int post);

stack* generate_graph(unsigned int global_state, event* vector, size_t length, unsigned short int number_of_states);

char *configuration_print(unsigned int global_state, unsigned short int number_of_states);

void print_graph(stack* s, unsigned short int number_of_states, Agraph_t* graph);

#endif
