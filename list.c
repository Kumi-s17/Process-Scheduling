#include "list.h"
#include "process.h"
#include "processor.h"
#include <stdlib.h>
#include <assert.h>

//create a new list
//adapted from Alsitar Moffat's book "Programming, problem solving, and Abstraction with C" (page 173)
struct list *create_empty_list(){
	struct list *list;
	list = malloc(sizeof(*list));
	assert(list);
	list -> head = list -> foot = NULL;
	return list;
}                                                              

                      
//Add a new process to queue when it arrives
//adapted from Alsitar Moffat's book "Programming, problem solving, and Abstraction with C" (page 173)
struct list *insert_node(struct list *list, struct process *new_process){
	assert(list);

	//if it is the first process
	if(list -> head == NULL){
		list -> head = list -> foot = new_process;
	}
	else{
		list -> foot -> next = new_process;
		new_process -> previous = list -> foot;
		list -> foot = new_process;
	}
	return list;
}

//removes a process that has finished from queue of the cpu
struct processor *delete_node(struct processor *cpu_running, int current_time){
	//if the process that has finished is the last one in the list
	if(cpu_running -> processes -> head == cpu_running -> processes -> foot){
		cpu_running -> processes -> head = NULL;
		cpu_running -> processes -> foot = NULL;
	}
	//if the finished process is not the last one in the list
	else{
		//if the process that has been completed is the first one in the list
		if(cpu_running -> current_process == cpu_running -> processes -> head){
			cpu_running -> processes -> head = cpu_running -> current_process -> next;
		}
		//if the completed process is the last one in the list
		else if(cpu_running -> current_process == cpu_running -> processes -> foot){
			cpu_running -> processes -> foot = cpu_running -> processes -> foot -> previous;
			cpu_running -> processes -> foot -> next = NULL;
		}
		else{
			struct process *previous_process = cpu_running -> current_process -> previous;
			struct process *next_process = cpu_running -> current_process -> next;
			previous_process -> next = next_process;
			next_process -> previous = previous_process;
		}
	}


	return cpu_running;
}

//removes the processes that have arrived from the processes that have yet to arrive
struct list *remove_added_processes(struct list *processes_not_arrived, struct list *incoming_processes){

	//if there are still processes remaining that will arrive at a later time
	if(processes_not_arrived -> foot != incoming_processes -> foot){
		processes_not_arrived -> head = incoming_processes -> foot -> next;
	}
	//if all processes have been allocated to their processors
	else{
		processes_not_arrived -> head = NULL;
	}
	incoming_processes -> foot -> next = NULL;

	return processes_not_arrived;
}

//frees the list of processes
//adapted from Alsitar Moffat's book "Programming, problem solving, and Abstraction with C" (page 173)
void free_list(struct list* deleted_nodes){
	struct process *current, *prev;
	assert(deleted_nodes);
	current = deleted_nodes -> head;
	while(current != NULL){
		prev = current;
		current = current -> next;
		free(prev -> dataptr);
		free(prev);
	}
	free(deleted_nodes);	
} 



   
