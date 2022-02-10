#ifndef OPTIMIZATION_H
#define OPTIMIZATION_H

#include <stdio.h>
#include "processor.h"

struct processor **optimize_allocation(struct processor **cpu, struct list *incoming_processes, int num_of_processor, int current_time, int *num_process_remaining, struct subprocess *subprocesses, int *num_diff_subprocesses, struct sort_cpu *sorted_cpu);

#endif
