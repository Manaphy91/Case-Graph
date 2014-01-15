#ifndef STACK_H
#define STACK_H

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

#define INVALID_OR_NULL_PTR 1
#define STACK_EMPTY 2

typedef struct node{
	void* value_ptr;
	struct node* next;
} node;

typedef struct stack{
	node* head;
	size_t length;
} stack;

void initialize_stack(stack* s);

void push(stack* s, void* value);

void *pop(stack* s);

bool empty(stack* s);

size_t get_stack_length(stack* s);

void reverse_stack(stack* s);

void *get_stack_array(stack* s);

#endif
