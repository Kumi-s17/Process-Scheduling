#include "processor.h"
#include "process.h"
#include "list.h"
#include "statistics.h"
#include "optimization.h"
#include <stdlib.h>
#include <assert.h>
#include <limits.h>


//Runs the simulation until all processes terminate
struct processor **run_processors(struct processor **cpu, struct list *processes_not_arrived, int num_of_processor, int *num_process_remaining, struct subprocess *subprocesses, int *num_diff_subprocesses, struct statistics *statistics, int num_of_process, int performance_improved, struct sort_cpu *sorted_cpu){
    int current_time = 0;
    struct list *incoming_processes = NULL;

    while(!terminate_simulation(cpu, num_of_processor, processes_not_arrived, current_time)){
        //remove process that has already been completed by the cpu
        cpu = remove_completed_process(cpu, num_of_processor, current_time, num_process_remaining, subprocesses, num_diff_subprocesses, statistics);

        //returns all the processes that arrive at that time
	    incoming_processes = get_incoming_processes(processes_not_arrived, current_time);
        
        //if there are incoming processes, allocate them to the suitable cpus
        if(incoming_processes -> head){
            //removes all processes that are going to be added to the cpus from the list of remaining processes
            processes_not_arrived = remove_added_processes(processes_not_arrived, incoming_processes);

            struct process *ptr = incoming_processes ->head;
             while(ptr){
                ptr = ptr->next;
            }

            incoming_processes = sort_ascending(incoming_processes); 

            ptr = incoming_processes ->head;
            while(ptr){
                ptr = ptr->next;
            }
        
            if(!performance_improved){
                //subprocesses = update_number_allocated(incoming_processes, num_of_processor, subprocesses,num_diff_subprocesses);
                incoming_processes = adjust_subprocess_remaining_time(incoming_processes, num_of_processor, subprocesses, num_diff_subprocesses);
            }
            //for optimization 
            else{
                cpu = optimize_allocation(cpu, incoming_processes, num_of_processor, current_time, num_process_remaining, subprocesses, num_diff_subprocesses, sorted_cpu);
            }
            cpu = allocate_processes(cpu, incoming_processes, num_of_processor, current_time, num_process_remaining, subprocesses, num_diff_subprocesses, sorted_cpu);
        }
        //run the current process
        cpu = run_processes(cpu, num_of_processor, current_time);
        current_time ++;
        free(incoming_processes);
    }
    //print performance statistics when the simulation ends
    print_statistics(statistics, current_time, num_of_process);

    return cpu;
}



//checks if there are no more incoming processes and if all arrived processes have finished executing
int terminate_simulation(struct processor **cpu, int num_of_processor, struct list *processes_not_arrived, int current_time){
    int i;
    //check if there are any processes running on the cpu or if there are any more incoming processes
    for(i = 0; i < num_of_processor; i++){
        if(cpu[i] -> processes -> head || processes_not_arrived -> head || current_time == 0){
            return FALSE;
        }
    }
    return TRUE;
}


//allocates incoming processes to the next cpu with the minimum total execution time
struct processor **allocate_processes(struct processor **cpu, struct list *incoming_processes, int num_of_processor, int current_time, int *num_process_remaining, struct subprocess *subprocesses, int *num_diff_subprocesses, struct sort_cpu *sorted_cpu){
    int i, processors_allocated;
    struct process *ptr = incoming_processes -> head;
    //while there are incoming processes, we add these processes to the cpu with the least total execution time
    while(ptr){
        //if the process is parallelisable, add split the subprocess into 
        if(ptr -> dataptr -> parallelisable == PARALLELISABLE){
            processors_allocated = subprocesses[get_subprocess_index(subprocesses, *num_diff_subprocesses, ptr -> dataptr -> process_id)].num_processors_allocated;
            //allocate the processes to the processors in order of the one with the least remaining execution time
            sorted_cpu = get_min_cpu(cpu, num_of_processor, sorted_cpu);
            struct process *new_subprocess = ptr;
            for(i = 0; i < processors_allocated; i++){
                if(i > 0){
                    new_subprocess = create_subprocess(new_subprocess, i, ptr);
                }
                cpu = insert_process(cpu, new_subprocess, current_time, sorted_cpu[i].dataptr -> cpu_id);
            }
        }
        else{
            sorted_cpu = get_min_cpu(cpu, num_of_processor, sorted_cpu);
            cpu = insert_process(cpu, ptr, current_time, sorted_cpu[0].dataptr -> cpu_id);
        }
        *num_process_remaining += 1;
        ptr = ptr -> next;
    }
    return cpu;
}

//free memory for the list storing cpu ids in ascending remaining time order
void free_sorted_cpu(struct sort_cpu * sorted_cpu, int num_of_processor){
    int i;
    for(i = 0; i < num_of_processor; i++){
        free(sorted_cpu[i].dataptr);
    }
    free(sorted_cpu);
}


//allocate memory for sorting processors in ascending order of remaining time 
struct sort_cpu *initialize_sorted_cpu(int num_of_processor){
    int i;
    struct sort_cpu *sorted_cpu = malloc(sizeof(*sorted_cpu) * num_of_processor);
    assert(sorted_cpu);
    for(i = 0; i < num_of_processor; i++){
        sorted_cpu[i].dataptr = malloc(sizeof(*sorted_cpu[i].dataptr));
        assert(sorted_cpu[i].dataptr);
        sorted_cpu[i].dataptr -> cpu_id = i;
        sorted_cpu[i].dataptr -> remaining_time = 0;
    }

    return sorted_cpu;
}

//adds the incoming process to its cpu
struct processor **insert_process(struct processor **cpu, struct process *process_added, int current_time, int cpu_index){
    cpu[cpu_index] -> processes = insert_node(cpu[cpu_index] -> processes, process_added);
    cpu[cpu_index] -> remaining_time += process_added -> dataptr -> remaining_time; 

    //for stimulation with no optimization, if there is a current process running, then check if this process has a smaller execution time, and swap if so
    if(cpu[cpu_index] -> current_process){
        if(process_added -> dataptr -> remaining_time < cpu[cpu_index] -> current_process -> dataptr -> remaining_time){
            cpu[cpu_index] -> current_process = process_added;
            if(process_added -> dataptr -> parallelisable == PARALLELISABLE){
                printf("%d,RUNNING,pid=%.1f,remaining_time=%d,cpu=%d\n", current_time, cpu[cpu_index] -> current_process -> dataptr -> process_id, cpu[cpu_index] -> current_process -> dataptr -> remaining_time, cpu_index);
            }
            else{
                printf("%d,RUNNING,pid=%d,remaining_time=%d,cpu=%d\n", current_time, (int) cpu[cpu_index] -> current_process -> dataptr -> process_id, cpu[cpu_index] -> current_process -> dataptr -> remaining_time, cpu_index);
            }
           
        }
    }
    return cpu;
}


//allocates space for the number of processors and sets each of them to their initial state
struct processor **initialize_cpu(int num_of_processor){
    struct processor **cpu = NULL;
    cpu = malloc(sizeof(*cpu)*num_of_processor);
    int i;
    for(i = 0; i < num_of_processor; i++){
        cpu[i] = malloc(sizeof(*cpu[i]));
        cpu[i] -> processes = create_empty_list();
        cpu[i] -> current_process = NULL;
        cpu[i] -> remaining_time = 0;
    }

    return cpu;
}


//returns an array of cpu indexes sorted in ascending remaining time order
struct sort_cpu *get_min_cpu(struct processor **cpu, int num_of_processor, struct sort_cpu *sorted_cpu){
    int i, j;
    struct sort_cpu *min_cpu;
    for(i = 0; i < num_of_processor; i++){
        sorted_cpu[i].dataptr -> cpu_id = i;
        sorted_cpu[i].dataptr -> remaining_time = cpu[i] -> remaining_time;   
    }


    //sort the cpu index according to least minimum time
    for(i = 0; i < num_of_processor - 1; i++){
        min_cpu = &sorted_cpu[i];
        for(j = i + 1; j < num_of_processor; j++){
            if(sorted_cpu[j].dataptr -> remaining_time < min_cpu -> dataptr -> remaining_time || 
                (sorted_cpu[j].dataptr -> remaining_time == min_cpu -> dataptr -> remaining_time && sorted_cpu[j].dataptr -> cpu_id < min_cpu -> dataptr -> cpu_id)){
                min_cpu = &sorted_cpu[j];
            }
        }
        struct cpu_data *temp = min_cpu -> dataptr;
        min_cpu -> dataptr = sorted_cpu[i].dataptr;
        sorted_cpu[i].dataptr = temp;   
    }

    return sorted_cpu;
}

//frees memory allocated for processors
void free_cpu(struct processor **cpu, int num_of_processor){
    int i;
    for(i = 0; i < num_of_processor; i++){
        free(cpu[i] -> processes);
        free(cpu[i]);
    }
    free(cpu);
}


