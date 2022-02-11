#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <stdio.h>
#include "process.h"


#define TRUE 1
#define FALSE 0

struct processor{
    struct list *processes;
    struct process *current_process;
    int remaining_time;
};

struct cpu_data{
    int cpu_id;
    int remaining_time;
};

struct sort_cpu{
    struct cpu_data *dataptr;
};

struct processor **run_processors(struct processor **cpu, struct list *processes_not_arrived, int num_of_processor, int *num_process_remaining, struct subprocess *subprocesses, int *num_diff_subprocesses, struct statistics *statistics, int num_of_process, int performance_improved, struct sort_cpu *sorted_cpu);
int terminate_simulation(struct processor **cpu, int num_of_processor, struct list *processes_not_arrived, int current_time);
struct processor **allocate_processes(struct processor **cpu, struct list *incoming_processes, int num_of_processor, int current_time, int *num_process_remaining, struct subprocess *subprocesses, int *num_diff_subprocesses, struct sort_cpu *sorted_cpu);
struct processor **insert_process(struct processor **cpu, struct process *process_added, int current_time, int cpu_index);
struct processor **initialize_cpu(int num_of_processor);
struct sort_cpu *get_min_cpu(struct processor **cpu, int num_of_processor, struct sort_cpu *sorted_cpu);
void free_cpu(struct processor **cpu, int num_of_processor);
void free_sorted_cpu(struct sort_cpu *sorted_cpu, int num_of_processor);
struct sort_cpu *initialize_sorted_cpu(int num_of_processor);


#endif
