#include "stack.h"

void initialize_stack(stack* s)
{
	if(!s)
		errno = INVALID_OR_NULL_PTR;
	s->head = NULL;
	s->length = 0;
}

void push(stack* s, void* value)
{
	if(!s || !value){
		errno = INVALID_OR_NULL_PTR;
		return;
	}
	if(s->head == NULL){
		s->head = malloc(sizeof((*s->head)));
		(s->head)->value_ptr = value;
		(s->head)->next = NULL;
	}else{
		node* tmp = malloc(sizeof(*tmp));
		tmp->value_ptr = value;
		tmp->next = s->head;
		s->head = tmp;
	}
	s->length++;
}

void *pop(stack* s)
{
	if(!s){
		errno = INVALID_OR_NULL_PTR;
		return NULL;
	}else if(s->head == NULL){
		errno = STACK_EMPTY;
		return NULL;
	}
	void* tmp_value = (s->head)->value_ptr;
	node* tmp_node = (s->head);	
	s->head = (s->head)->next;
	s->length--;
	free(tmp_node);
	return tmp_value;
}

bool empty(stack* s)
{
	if(!s)
		errno = INVALID_OR_NULL_PTR;
	return s->head == NULL;
}

size_t get_stack_length(stack* s)
{
	return s->length;
}

void reverse_stack(stack* s)
{
	node* tmp_node = NULL;
	node* prev_node = NULL;
	while(s->head){
		tmp_node = s->head;
		s->head = (s->head)->next;
		tmp_node->next = prev_node;
		prev_node = tmp_node;
	}
	s->head = tmp_node;
}

void *get_stack_array(stack* s)
{
	void** tmp = malloc(sizeof(void*) * s->length);
	node* tmp_node = s->head;
	size_t i = 0;
	while(tmp_node){
		tmp[i++] = tmp_node->value_ptr;
		tmp_node = tmp_node->next;
	}
	return tmp;
}

/*
int main(void)
{
	int* j = NULL;
	stack* s = malloc(sizeof(*s));
	initialize_stack(s);
	for(int i = 1; i < 11; i++){
		j = malloc(sizeof(*j));
		*j = i;
		push(s, j);
	}
	reverse_stack(s);
	void** array = get_stack_array(s);
	for(size_t i = 0; i < s->length; i++)
		printf("array[%jd] : %d\n", i, *((int *) array[i]));
	free(array);
	while(!empty(s)){
		j = (int *) pop(s);
		printf("%d\n", *j);
		free(j);
	}
	free(s);
	return 0;
}
*/
