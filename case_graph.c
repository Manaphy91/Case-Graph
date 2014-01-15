#include "case_graph.h"
#include <string.h>
#include <math.h>

void bit_print(unsigned int value)
{
	size_t length = sizeof(value) * 8;
	for(size_t i = 0; i < length; i++)
		printf("%d", (((value >> (length - (i + 1))) & 1) == 1));
	printf("\n");
}

void initialize_event(event* ev, char ID, unsigned int pre, unsigned int post)
{
	ev->ID = ID;
	ev->pre_condition = pre;
	ev->post_condition = post;
	ev->pre_post_compl = (pre | post) ^ UINT_MAX;
}

stack* generate_graph(unsigned int global_state, event* vector, size_t length, unsigned short int number_of_states)
{
	stack* return_stack = malloc(sizeof(*return_stack));
	initialize_stack(return_stack);
	stack* tmp_stack = malloc(sizeof(*tmp_stack));
	initialize_stack(tmp_stack);
	graph_node* n = malloc(sizeof(*n));
	n->global_state = global_state;
	n->event_node_stack = malloc(sizeof(*(n->event_node_stack)));
	initialize_stack(n->event_node_stack);
	push(tmp_stack, n);
	unsigned int bit_pow = (unsigned int) pow(2, number_of_states);
	bool* generated = malloc(sizeof(*generated) * bit_pow);
	for(size_t i = 0; i < bit_pow; i++)
		generated[i] = false;
	generated[global_state] = true;
	do{
		n = (graph_node *) pop(tmp_stack);
		unsigned int s = n->global_state;
		for(size_t i = 0; i < length; i++){
			if(((s & vector[i].pre_condition) == vector[i].pre_condition) && 
				((s & vector[i].post_condition) == 0)){
				unsigned int tmp = s & vector[i].pre_post_compl;
				tmp = tmp | vector[i].post_condition;

				if(!generated[tmp]){
					graph_node* tmp_node = malloc(sizeof(*tmp_node));
					tmp_node->global_state = tmp;
					tmp_node->event_node_stack = malloc(sizeof(*(tmp_node->event_node_stack)));
					initialize_stack(tmp_node->event_node_stack);
					push(tmp_stack, tmp_node);
					generated[tmp] = true;
				}
				event_node* e = malloc(sizeof(*e));
				e->event_ID = vector[i].ID;
				e->global_state = tmp;
				push(n->event_node_stack, e);
			}
		}
		reverse_stack(n->event_node_stack);
		push(return_stack, n);
	}while(!empty(tmp_stack));
	free(tmp_stack);
	free(generated);
	return return_stack;
}

char *configuration_print(unsigned int global_state, unsigned short int number_of_states)
{
	char tmp[160] = { '\0' };
	sprintf(tmp, "{ ");
	int c = 2;
	for(int i = 0; i < number_of_states; i++){
		if(((global_state >> (number_of_states - (i + 1))) & 1)){
			c += sprintf(tmp + c, "p%d ", i + 1);
		}
	}
	tmp[c] = '}';
	char* str = calloc(strlen(tmp) + 1, sizeof(*str));
	strcpy(str, tmp);
	return str;
}

void print_graph(stack* s, unsigned short int number_of_states, Agraph_t *graph)
{
	size_t length = s->length;
	size_t link = 0;
	void** tmp_vector = get_stack_array(s);
	for(size_t i = 0; i < length; i++)
		link += (((graph_node *)tmp_vector[i])->event_node_stack)->length;
	free(tmp_vector);
	graph_node* vector[length];
	printf("Sono stati generati:\n");
	printf("\t%jd stati;\n", length);
	printf("\t%jd archi.\n", link);
	printf("---------------grafo--------------\n");
	stack* deadlock = malloc(sizeof(*deadlock));
	initialize_stack(deadlock);
	size_t i = 0;
	size_t c = 0;
	char* str;
	char link_str[6] = { '\0' };
	while(!empty(s)){
		graph_node* node = (graph_node *) pop(s);
		vector[i++] = node;
		str = configuration_print(node->global_state, number_of_states);
		printf("%s\n", str);
		Agnode_t *t_node;
		if(agnode(graph, str, 0)){
			t_node = agnode(graph, str, 0);
		} else {
			t_node = agnode(graph, str, 1);
		}
		free(str);
		stack* ev_stack = node->event_node_stack;
		if(empty(ev_stack)){
			push(deadlock, node);
			agset(t_node, "fontcolor", "red1");
		}
		Agedge_t *edge;
		while(!empty(ev_stack)){
			event_node* en = (event_node *) pop(ev_stack);
			str = configuration_print(en->global_state, number_of_states);
			Agnode_t *h_node; 
			if(agnode(graph, str, 0)){
				h_node = agnode(graph,str, 0);
			} else {
				h_node = agnode(graph, str, 1);
			}
			printf("|--t%c--> ", en->event_ID);
			sprintf(link_str, "t%c", en->event_ID);	
			edge = agedge(graph, t_node, h_node, link_str, 1);
			agset(edge, "label", link_str);
			printf("%s\n", str);
			free(str);
			free(en);
		}
		free(ev_stack);
	}
	printf("-------------deadlock-------------\n");
	if(!empty(deadlock)){
		printf("Possibili deadlock sono stati individuati:\n");
		reverse_stack(deadlock);
		size_t d_num = deadlock->length;
		do{
			graph_node* deadlock_node = (graph_node *) pop(deadlock);
			printf("\t");
			str = configuration_print(deadlock_node->global_state, number_of_states);
			printf("%s\n", str);
			free(str);
		}while(!empty(deadlock));
		printf("Totale deadlock individuati: %jd\n", d_num);
	}else{
		printf("Nessun deadlock individuato.\n");
	}
	free(deadlock);
	for(size_t j = 0; j < i; j++){
		free(vector[j]);
	}
}

int main(void)
{
	/*
	 s e' lo stato globale iniziale e viene cosi' inizializzato:
	 - prendo una stringa di bit con numero maggiore del numero di stati/condizioni
	   (in questo caso un unsigned int);
	 - ogni bit viene settato a 1 o a 0 rispettivamente se la corrispondente condizione,
	   o asserzione, e' vera o falsa; 
	 - il bit meno significativo sara' l'ultimo stato(se suppongo di avere 6 stati
	   esso corrispondera' al sesto) e mano a mano proseguendo verso il bit piu
	   significativo saranno rappresentati gli stati precedenti.
	 Esempio:
	  se di una rete ho vere le condizioni p1, p2 e p4 e le possibili condizioni sono
	  sei allora lo stato globale sara' cosi' rappresentato:

		     1 2 3 4 5 6
	  	|0|0|1|1|0|1|0|0|

	  ed in esadecimale corrispondera' a

      	0x34

	Per gli eventi funziona nel medesimo modo ed il tutto e' funzionale al fatto
	che per verificare pre e post condizioni vengono effettuate delle operazioni
	con &(and), |(or) ed ^(xor) binari.

	*/	
	/*
	unsigned int s = 0x64;
	event* array = malloc(sizeof(*array) * 3);
	initialize_event(&array[0], '1', 0x60, 0x18);
	initialize_event(&array[1], '2', 0xc, 0x2);
	initialize_event(&array[2], '3', 0x12, 0x1);
	stack* st = generate_graph(s, array, 3);
	free(array);
	print_graph(st,7);
	free(st);
	*/

	/*
	unsigned int s = 0x1c0;
	event* array = malloc(sizeof(*array) * 4);
	initialize_event(&array[0], '1', 0x1c0, 0x38);
	initialize_event(&array[1], '2', 0x30, 0x4);
	initialize_event(&array[2], '3', 0x8, 0x2);
	initialize_event(&array[3], '4', 0x6, 0x1);
	stack* st = generate_graph(s, array, 4, 9);
	free(array);
	print_graph(st, 9);
	free(st);
	*/
	
	/*
	unsigned int s = 0x21;
	event* array = malloc(sizeof(*array) * 5);
	initialize_event(&array[0], '1', 0x20, 0x10);
	initialize_event(&array[1], '2', 0x10, 0x28);
	initialize_event(&array[2], '3', 0x8, 0x4);
	initialize_event(&array[3], '4', 0x5, 0x2);
	initialize_event(&array[4], '5', 0x2, 0x1);
	stack* st = generate_graph(s, array, 5, 6);
	free(array);
	reverse_stack(st);
	print_graph(st, 6);
	free(st);
	*/

	/*
	unsigned int s = 0xc2;
	event* array = malloc(sizeof(*array) * 6);
	initialize_event(&array[0], '1', 0x80, 0x20);
	initialize_event(&array[1], '2', 0x11, 0x82);
	initialize_event(&array[2], '3', 0x9, 0x42);
	initialize_event(&array[3], '4', 0x40, 0x4);
	initialize_event(&array[4], '5', 0x22, 0x11);
	initialize_event(&array[5], '6', 0x6, 0x9);
	stack* st = generate_graph(s, array, 6, 8);
	free(array);
	reverse_stack(st);
	print_graph(st, 8);
	free(st);
	*/

	/*
	unsigned int s = 0x61;
	event* array = malloc(sizeof(*array) * 6);
	initialize_event(&array[0], '1', 0x40, 0x10);
	initialize_event(&array[1], '2', 0x8, 0x41);
	initialize_event(&array[2], '3', 0x4, 0x21);
	initialize_event(&array[3], '4', 0x20, 0x2);
	initialize_event(&array[4], '5', 0x11, 0x8);
	initialize_event(&array[5], '6', 0x3, 0x4);
	stack* st = generate_graph(s, array, 6, 7);
	free(array);
	reverse_stack(st);
	print_graph(st, 7);
	free(st);
	*/

	/*
	// due filosofi a cena con deadlock
	unsigned int s = 0xc03;
	event* array = malloc(sizeof(*array) * 8);
	initialize_event(&array[0], '1', 0x800, 0x300);
	initialize_event(&array[1], '2', 0x400, 0xc0);
	initialize_event(&array[2], '3', 0x201, 0x20);
	initialize_event(&array[3], '4', 0x102, 0x10);
	initialize_event(&array[4], '5', 0x82, 0x8);
	initialize_event(&array[5], '6', 0x41, 0x4);
	initialize_event(&array[6], '7', 0x30, 0x803);
	initialize_event(&array[7], '8', 0xc, 0x403);
	stack* st = generate_graph(s, array, 8, 12);
	free(array);
	reverse_stack(st);
	print_graph(st, 12);
	free(st);
	*/

	/*
	// due filosofi a cena senza deadlock
	unsigned int s = 0x33c;
	event* array = malloc(sizeof(*array) * 6);
	initialize_event(&array[0], '1', 0x228, 0x80);
	initialize_event(&array[1], '2', 0x114, 0x40);
	initialize_event(&array[2], '3', 0xc0, 0x33c);
	initialize_event(&array[3], '4', 0x218, 0x2);
	initialize_event(&array[4], '5', 0x124, 0x1);
	initialize_event(&array[5], '6', 0x3, 0x33c);
	stack* st = generate_graph(s, array, 6, 10);
	free(array);
	reverse_stack(st);
	print_graph(st, 10);
	free(st);
	*/

	/*
	// due filosofi a cena senza deadlock v2
	unsigned int s = 0x33c;
	event* array = malloc(sizeof(*array) * 6);
	initialize_event(&array[0], '1', 0x228, 0x80);
	initialize_event(&array[1], '2', 0x110, 0x40);
	initialize_event(&array[2], '3', 0xc0, 0x338);
	initialize_event(&array[3], '4', 0x18, 0x2);
	initialize_event(&array[4], '5', 0x124, 0x1);
	initialize_event(&array[5], '6', 0x3, 0x13c);
	stack* st = generate_graph(s, array, 6, 10);
	free(array);
	reverse_stack(st);
	print_graph(st, 10);
	free(st);
	*/

	/*
	unsigned int s = 0x4;
	event* array = malloc(sizeof(*array) * 3);
	initialize_event(&array[0], '1', 0x4, 0x3);
	initialize_event(&array[1], '2', 0x5, 0x2);
	initialize_event(&array[2], '3', 0x2, 0x4);
	stack* st = generate_graph(s, array, 3, 3);
	free(array);
	reverse_stack(st);
	print_graph(st, 3);
	free(st);
	*/

	GVC_t* gvc = gvContext();
	Agraph_t* g = agopen("Case Graph", Agdirected, 0);
	agattr(g, AGRAPH, "rankdir", "TB");
	agattr(g, AGNODE, "shape", "none");
	agattr(g, AGNODE, "fontcolor", "black");
	agattr(g, AGEDGE, "label", "");
	FILE* output = fopen("prova.svg", "w");
	if(!output)
		perror("fopen");
	unsigned int s = 0x24;
	event* array = malloc(sizeof(*array) * 5);
	initialize_event(&array[0], '1', 0x20, 0x10);
	initialize_event(&array[1], '2', 0x10, 0xa);
	initialize_event(&array[2], '3', 0x8, 0x24);
	initialize_event(&array[3], '4', 0x4, 0x2);
	initialize_event(&array[4], '5', 0x2, 0x1);
	stack* st = generate_graph(s, array, 5, 6);
	free(array);
	reverse_stack(st);
	print_graph(st, 6, g);
	free(st);
	gvLayout(gvc, g, "dot");
	gvRender(gvc, g, "svg", output);
	gvFreeLayout(gvc, g);
	agclose(g);
	if(fclose(output))
		perror("fclose");
	gvFreeContext(gvc);
	return 0;

	/*
	GVC_t* gvc = gvContext();
	Agraph_t* g = agopen("Case Graph", Agdirected, 0);
	agattr(g, AGRAPH, "rankdir", "LR");
	agattr(g, AGNODE, "fontcolor", "red");
	agattr(g, AGNODE, "shape", "none");
	FILE* output = fopen("prova.svg", "w");
	if(!output)
		perror("fopen");
	Agnode_t* a = agnode(g, "A", 1);
	Agnode_t* b = agnode(g, "B", 1);
	Agnode_t* c = agnode(g, "C", 1);
	Agnode_t* d = agnode(g, "D", 1);
	agedge(g, a, b, "c1", 1);
	agedge(g, b, c, "c1", 1);
	agedge(g, c, d, "c1", 1);
	agedge(g, d, a, "c1", 1);
	Agedge_t* e = agedge(g, a, c, "c1", 1);
	agedge(g, b, d, "c1", 1);
	gvLayout(gvc, g, "dot");
	gvRender(gvc, g, "svg", output);
	gvFreeLayout(gvc, g);
	agclose(g);
	if(fclose(output))
		perror("fclose");
	gvFreeContext(gvc);
	return 0;
	*/
}
