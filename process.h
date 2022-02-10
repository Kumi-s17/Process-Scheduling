#ifndef PROCESS_H
#define PROCESS_H

#define PARALLELISABLE 'p'
#define NON_PARALLELISABLE 'n'

#include <stdio.h>
#include "statistics.h"

struct data{
	int time_arrived;
    double process_id;
    int initial_execution_time;
    char parallelisable;
    int remaining_time;
};

struct process{
    struct data *dataptr;
    struct process *next;
    struct process *previous;
};

struct subprocess{
    int parent_id;
    int num_completed;
    int num_processors_allocated;
};

struct list *read_in(FILE *fp, struct list * list, int *num_diff_subprocesses, int *num_of_process);
struct subprocess *initialize_subprocess(int num_diff_subprocesses, struct list *processes_not_arrived);
struct process *create_new_process(int time_arrived, int id, int execution_time, char parallelisable);
struct processor **remove_completed_process(struct processor **cpu, int num_of_processor, int current_time, int *num_process_remaining, struct subprocess *subprocesses, int *num_diff_subprocesses, struct statistics *statistics);
struct subprocess *complete_subprocess(int *num_diff_subprocesses, struct subprocess *subprocesses, struct processor *target_cpu);
int num_subprocesses_complete(int *num_diff_subprocesses, struct subprocess *subprocesses, struct processor *target_cpu);
void print_finished(struct list *deleted_node, int current_time, int *num_process_remaining);
struct processor **run_processes(struct processor **cpu, int num_of_processor, int current_time);
struct process *get_next_process(struct list *processes, int current_time, int index);
struct list *sort_ascending(struct list *incoming_processes);
struct list *get_incoming_processes(struct list *processes_not_arrived, int current_time);
struct list *adjust_subprocess_remaining_time(struct list *incoming_processes, int num_of_processor, struct subprocess *subprocesses, int *num_diff_subprocesses);
int get_subprocess_index(struct subprocess *subprocesses, int num_diff_subprocesses, int target_id);
struct process *create_subprocess(struct process *new_subprocess, int i, struct process *parent_process);
void free_process(struct process * process);

#endif
