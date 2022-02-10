#include "process.h"
#include "list.h"
#include "processor.h"
#include "statistics.h"
#include "optimization.h"
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include <math.h>


#include <stdio.h>
#include <string.h>


//Reads in information about the processes from the input file
struct list *read_in(FILE *fp, struct list * list, int *num_diff_subprocesses, int *num_of_process){
	int time_arrived, id, execution_time;
	char parallelisable;

	//While there are processes to be read
	while(fscanf(fp, "%d %d %d %c\n", &time_arrived, &id, &execution_time, &parallelisable) == 4){
        //if execution time is 1, treat that process as non-parallelisable
        if(execution_time == 1){
            parallelisable = NON_PARALLELISABLE;
        }
        if(parallelisable == PARALLELISABLE){
            *num_diff_subprocesses += 1;
        }
		struct process *new_process = create_new_process(time_arrived, id, execution_time, parallelisable);
		insert_node(list, new_process);
        *num_of_process += 1;
    }

	return list;
			
}

//initializes the array storing information about the subprocesses
struct subprocess *initialize_subprocess(int num_diff_subprocesses, struct list *processes_not_arrived){
    struct subprocess *subprocesses = malloc(sizeof(*subprocesses)*num_diff_subprocesses);
    struct process *ptr = processes_not_arrived -> head;
    struct subprocess *subprocess = subprocesses;

    while(ptr){
        if(ptr -> dataptr -> parallelisable == PARALLELISABLE){
            subprocess -> parent_id = ptr -> dataptr -> process_id;
            subprocess -> num_completed = 0;
            subprocesses -> num_processors_allocated = 0;
            subprocess ++;
        }
        ptr = ptr -> next;
    }
    return subprocesses;
}

//insert the data read in into the node of the linked list
struct process *create_new_process(int time_arrived, int id, int execution_time, char parallelisable){
	struct process *new_process;
	new_process = malloc (sizeof(*new_process));
	assert(new_process);
    

    new_process -> dataptr = malloc(sizeof(struct data));
    assert(new_process -> dataptr);
	new_process -> dataptr -> time_arrived = time_arrived;
	new_process -> dataptr -> process_id = id;
	new_process -> dataptr -> initial_execution_time = execution_time;
    new_process -> dataptr -> remaining_time = execution_time;
    new_process -> dataptr -> parallelisable = parallelisable;

    new_process -> next = NULL;
    new_process -> previous = NULL;

	return new_process;
}

//check for every cpu if the previous process has run to completion and if it has, remove it
struct processor **remove_completed_process(struct processor **cpu, int num_of_processor, int current_time, int *num_process_remaining, struct subprocess *subprocesses, int *num_diff_subprocesses, struct statistics *statistics){
    int i;
    //stores all the processes that were finished at that time 
    struct list *deleted_nodes = create_empty_list();
    for(i = 0; i < num_of_processor; i++){
        int finished_process = FALSE;
        //if there was a process running which has come to completion
        if(cpu[i] -> current_process && cpu[i] -> current_process -> dataptr -> remaining_time == 0){
            
            //If the subprocess has finished increment the number of subprocesses that have finished
            if(cpu[i] -> current_process -> dataptr -> parallelisable == PARALLELISABLE){
                subprocesses = complete_subprocess(num_diff_subprocesses, subprocesses, cpu[i]);
            }

            //if the process finished is non-parallelisable or if it is parallelisable and all processes have finished
            if(cpu[i] -> current_process -> dataptr -> parallelisable == NON_PARALLELISABLE || (cpu[i] -> current_process -> dataptr -> parallelisable == PARALLELISABLE && num_subprocesses_complete(num_diff_subprocesses, subprocesses, cpu[i]) == subprocesses[get_subprocess_index(subprocesses, *num_diff_subprocesses, cpu[i] -> current_process -> dataptr -> process_id)].num_processors_allocated)){
                finished_process = TRUE;
                *num_process_remaining -= 1;

                //calculations for the performance statistics
                double duration = (current_time - (cpu[i] -> current_process -> dataptr -> time_arrived));
                statistics -> turnaround_time += duration;
                double current_time_overhead = duration / cpu[i] -> current_process -> dataptr -> initial_execution_time;
                statistics -> average_time_overhead += current_time_overhead;
                //checks if the time overhead for the current process is the maximum and stores it if it is
                if(current_time_overhead > statistics -> max_time_overhead){
                    statistics -> max_time_overhead = current_time_overhead;
                }
            }
            cpu[i] = delete_node(cpu[i], current_time); 
            if(finished_process){
                deleted_nodes = insert_node(deleted_nodes, cpu[i] -> current_process);
                deleted_nodes -> foot -> next = NULL;
            }
            cpu[i] -> current_process = NULL;
        }
    }
    if(deleted_nodes -> head){
        print_finished(deleted_nodes, current_time, num_process_remaining);
    }

    free_list(deleted_nodes);
    
    return cpu;
}

//increments the number of subprocesses that have run to completion for the parent process id
struct subprocess *complete_subprocess(int *num_diff_subprocesses, struct subprocess *subprocesses, struct processor *target_cpu){
    struct subprocess *ptr = subprocesses;
    int i;
    for(i = 0; i < *num_diff_subprocesses; i++){
        //get the information for that subprocess
        if(ptr -> parent_id == (int) target_cpu -> current_process -> dataptr -> process_id){
            ptr -> num_completed += 1;
        }
        ptr ++;
    }
    return subprocesses;

}

//returns the number of subprocesses of that process id that have been completed
int num_subprocesses_complete(int *num_diff_subprocesses, struct subprocess *subprocesses, struct processor *target_cpu){
    struct subprocess *ptr = subprocesses;
    int number_completed, i;
    for(i = 0; i < *num_diff_subprocesses; i++){
        //check if all subprocesses of the same process has ended
        if(ptr -> parent_id == (int) target_cpu -> current_process -> dataptr -> process_id){
            number_completed = ptr -> num_completed;
        }
        ptr ++;
    }
    return number_completed;
}

//prints the output when a process has finished
void print_finished(struct list *deleted_node, int current_time, int *num_process_remaining){
    struct process *ptr = deleted_node -> head;
    while(ptr != deleted_node -> foot){
        printf("%d,FINISHED,pid=%d,proc_remaining=%d\n", current_time, (int) ptr -> dataptr -> process_id, *num_process_remaining);
        ptr = ptr -> next;
    } 
    printf("%d,FINISHED,pid=%d,proc_remaining=%d\n", current_time, (int) ptr -> dataptr -> process_id, *num_process_remaining);  
}

//frees memory for the process
void free_process(struct process * process){
    free(process -> dataptr);
    free(process);
}


//if there is no process currently running, find if there is one that can be run. If there is a process that can be run, decrement its remining time.
struct processor **run_processes(struct processor **cpu, int num_of_processor, int current_time){
    int i;
    //checking for each cpu
    for(i = 0; i < num_of_processor; i++){
        //If the previous process has just ended or there is no process currently run
        if(!(cpu[i] -> current_process)){
            cpu[i] -> current_process = get_next_process(cpu[i] -> processes, current_time, i);
        }

        //If there is currently a process to be run, run it
        if(cpu[i] -> current_process){
            cpu[i] -> current_process -> dataptr -> remaining_time -= 1;
            cpu[i] -> remaining_time -= 1;
        }  
         
    }
    return cpu;
}

//gets the next process to be run when the previous process has been executed to completion
struct process *get_next_process(struct list *processes, int current_time, int index){
    struct process *ptr = processes -> head;
    struct process *current = NULL;
    
    //if there are any processes left that can be run at the moment, find the one that should be executed next
    if(ptr && ptr -> dataptr -> time_arrived <= current_time){
        current = ptr;
        ptr = ptr -> next;
        
        while (ptr != processes -> foot -> next){
            if(ptr -> dataptr -> remaining_time < current -> dataptr -> remaining_time){
                current = ptr;
            }
            else if((ptr -> dataptr -> remaining_time) == (current -> dataptr -> remaining_time) && ptr -> dataptr -> process_id < current -> dataptr -> process_id){
                current = ptr;
            }
            ptr = ptr -> next;
        }
    }

    //if there is a new process or subprocess running, print to output
    if(current){
        if(current -> dataptr -> parallelisable == PARALLELISABLE){
            printf("%d,RUNNING,pid=%.1f,remaining_time=%d,cpu=%d\n", current_time, current -> dataptr -> process_id, current -> dataptr -> remaining_time, index);
            }
        else{
            printf("%d,RUNNING,pid=%d,remaining_time=%d,cpu=%d\n", current_time, (int) current -> dataptr -> process_id, current -> dataptr -> remaining_time, index);
        }
    }
    return current;
}

//sort processes in ascending order of execution time
struct list *sort_ascending(struct list *incoming_processes){
    struct process *sorting = incoming_processes -> head;
    //if there is more than one incoming process, then sort it
    if(incoming_processes -> head != incoming_processes -> foot){
        while(sorting != incoming_processes -> foot){
            struct process *ptr = sorting -> next;

            while(ptr){
                //if the process at ptr has a smaller execution time, swap it with the process being sorted and if there is a tie, check the process id
                if(ptr -> dataptr -> remaining_time < sorting -> dataptr -> remaining_time || 
                      ((ptr -> dataptr -> remaining_time == sorting -> dataptr -> remaining_time) && (ptr -> dataptr -> process_id < sorting -> dataptr -> process_id))){
                    struct data *temp = sorting -> dataptr;
                    sorting -> dataptr = ptr -> dataptr;
                    ptr -> dataptr = temp;
                }
                ptr = ptr -> next; 
            }
            sorting = sorting -> next;
        }
        
    }
    return incoming_processes;

}



//get a list of processes that arrive at that time after execution
struct list *get_incoming_processes(struct list *processes_not_arrived, int current_time){
    struct list *incoming_processes = create_empty_list();
    struct process *ptr = processes_not_arrived -> head;
    while(ptr){
        if(ptr -> dataptr -> time_arrived == current_time){
            incoming_processes = insert_node(incoming_processes, ptr);
        }
        ptr = ptr -> next;
    }
    
    return incoming_processes;
}


//calculates and updates the number of processors the subprocesses are allocated to and their remaining time
struct list *adjust_subprocess_remaining_time(struct list *incoming_processes, int num_of_processor, struct subprocess *subprocesses, int *num_diff_subprocesses){
    struct process *ptr = incoming_processes -> head;
    while(ptr){
        //if the process is parallelisable, add split the subprocess into 
        if(ptr -> dataptr -> parallelisable == PARALLELISABLE){
            int processors_allocated = 1;
            //find the max number of processors the process is parallelisable to 
            while((ptr -> dataptr -> initial_execution_time)/processors_allocated >= 1){
                processors_allocated ++;
            }
            processors_allocated --;

            while(processors_allocated > num_of_processor){
                processors_allocated -= 1;
            }

            subprocesses[get_subprocess_index(subprocesses, *num_diff_subprocesses, ptr -> dataptr -> process_id)].num_processors_allocated = processors_allocated;
            
            ptr -> dataptr -> remaining_time = ceil((double) ptr -> dataptr -> initial_execution_time / processors_allocated) + 1;

        }
        ptr = ptr -> next;
    }
    return incoming_processes;
}

//get the index of the array storing the information about that subprocess
int get_subprocess_index(struct subprocess *subprocesses, int num_diff_subprocesses, int target_id){
     int i, target_index;
     for(i = 0; i < num_diff_subprocesses; i++){
         if(subprocesses[i].parent_id == target_id){
             target_index = i;
         }
     }
     return target_index;
}

//creates subprocesses for parallelisable processes
struct process *create_subprocess(struct process *new_subprocess, int i, struct process *parent_process){
    new_subprocess = malloc(sizeof(*new_subprocess));
    assert(new_subprocess);
    new_subprocess -> dataptr = malloc(sizeof(*new_subprocess -> dataptr));

    new_subprocess -> dataptr -> initial_execution_time = parent_process -> dataptr -> initial_execution_time;
    new_subprocess -> dataptr -> parallelisable = parent_process -> dataptr -> parallelisable;
    new_subprocess -> dataptr -> process_id = parent_process -> dataptr -> process_id + (0.1 * i);
    new_subprocess -> dataptr -> time_arrived = parent_process -> dataptr -> time_arrived;
    new_subprocess -> dataptr -> remaining_time = parent_process -> dataptr -> remaining_time;
    new_subprocess -> next = NULL;
    
    return new_subprocess;
}
