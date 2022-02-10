#ifndef LIST_H
#define LIST_H

#include <stdio.h>

struct list{
	struct process *head;
	struct process *foot; 
};        

struct list *create_empty_list();
struct list *insert_node(struct list *list, struct process *new_process);
struct processor *delete_node(struct processor *cpu_running, int current_time);
struct list *remove_added_processes(struct list *processes_not_arrived, struct list *incoming_processes);
void free_list(struct list* deleted_nodes);

#endif
