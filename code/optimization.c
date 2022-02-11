#include "optimization.h"
#include "list.h"
#include "process.h"
#include "processor.h"
#include <math.h>

//allocates processes and subprocesses to processors for the optimized simulation
struct processor **optimize_allocation(struct processor **cpu, struct list *incoming_processes, int num_of_processor, int current_time, int *num_process_remaining, struct subprocess *subprocesses, int *num_diff_subprocesses, struct sort_cpu *sorted_cpu){
    struct process *ptr = incoming_processes -> head;
    int i;

    while(ptr){
        //if the process is parallelizable
        if(ptr -> dataptr -> parallelisable == PARALLELISABLE){
            float total_remaining_time = ptr -> dataptr -> initial_execution_time;
            int max_allocation = 0, processors_allocated = 0;
            //get the sum of the total remaining time across all processors
            for(i = 0; i < num_of_processor; i++){
                total_remaining_time += cpu[i] -> remaining_time;
            }
            //gets the maximum number of processors which have smaller remaining time than the average remaining time after the new process is added and if processes were split equally 
            for(i = 0; i < num_of_processor; i++){
                if(cpu[i] -> remaining_time < total_remaining_time / num_of_processor){
                    max_allocation ++;
                }
            }

            processors_allocated = max_allocation;
            //get the number of processors the process can be split among
            while(ptr -> dataptr -> initial_execution_time / processors_allocated < 2){
                processors_allocated --;
            }
            
            //makes sure that the number of processors allocated is not larger than there are processors
             while(processors_allocated > num_of_processor){
                processors_allocated -= 1;
            }

             //update the number of processors the suprocess is allocated to
            subprocesses[get_subprocess_index(subprocesses, *num_diff_subprocesses, ptr -> dataptr -> process_id)].num_processors_allocated = processors_allocated;
            //calculate the time remaining for the subprocesses
            ptr -> dataptr -> remaining_time = ceil((double) ptr -> dataptr -> initial_execution_time / processors_allocated) + 1;
        }
        ptr = ptr -> next;
    }
    return cpu;
}