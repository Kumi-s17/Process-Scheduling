#ifndef STATISTICS_H
#define STATISTICS_H

#include <stdio.h>

struct statistics{
    double turnaround_time;
    double average_time_overhead;
    double max_time_overhead;
};

void print_statistics(struct statistics *statistics, int current_time, int num_of_process);
struct statistics *initialize_statistics();

#endif
