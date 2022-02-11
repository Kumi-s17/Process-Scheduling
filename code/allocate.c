#include "list.h"
#include "process.h"
#include "processor.h"
#include "statistics.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define MAX_PROCESSORS 1024
#define MAX_STDIN 5



int main(int argc, char *argv[]) {
    char *filename = NULL;
    int num_of_processor;
    int performance_improved = FALSE;
    char *input[MAX_STDIN];
    int i = 0;

    while(i < argc - 1 && i < MAX_STDIN){
        input[i] = argv[i+1];
        i ++;
    }


    for(i = 0; i < argc - 1; i++){
        if(!strcmp(input[i], "-f")){
            i += 1;
            filename = input[i];
        }
        else if(!strcmp(input[i], "-p")){
             i += 1;
            num_of_processor = atoi(input[i]);
        }
        else if(!strcmp(input[i], "-c")){
            performance_improved = TRUE;
        }
    }
    
    
    //open the input file 
    FILE *fp;
    fp = fopen(filename, "r");
    assert(fp);

    
    //keeps track of the number of processes available for execution across the processors at the current time
    int num_process_remaining = 0;
    int num_of_process = 0, num_diff_subprocesses = 0;

    struct processor **cpu = NULL;
    cpu = initialize_cpu(num_of_processor);

    struct list *processes_not_arrived = create_empty_list();

    //read in the processes into a list
    processes_not_arrived = read_in(fp, processes_not_arrived, &num_diff_subprocesses, &num_of_process);

    struct subprocess *subprocesses = initialize_subprocess(num_diff_subprocesses, processes_not_arrived);

    struct statistics *statistics = initialize_statistics();

    struct sort_cpu *sorted_cpu = initialize_sorted_cpu(num_of_processor);
    
    cpu = run_processors(cpu, processes_not_arrived, num_of_processor, &num_process_remaining, subprocesses, &num_diff_subprocesses, statistics, num_of_process, performance_improved, sorted_cpu);
    
   
    fclose(fp);
    free(processes_not_arrived);
    free_cpu(cpu, num_of_processor);
    free(subprocesses);
    free(statistics); 
    free_sorted_cpu(sorted_cpu, num_of_processor);

    return 0;
}
