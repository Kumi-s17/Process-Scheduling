#include "process.h"
#include "list.h"
#include "processor.h"
#include "statistics.h"
#include <stdlib.h>
#include <math.h>

//prints the performance statistics to stdout
void print_statistics(struct statistics *statistics, int current_time, int num_of_process){
    statistics -> turnaround_time = ceil(statistics -> turnaround_time / num_of_process);
    statistics -> average_time_overhead /= num_of_process;
    printf("Turnaround time %d\n", (int) statistics -> turnaround_time);

    //code to round up turnaround time adapted from stack overflow
    //https://stackoverflow.com/questions/39149543/using-round-function-in-c/39150104
    statistics -> max_time_overhead = round(statistics -> max_time_overhead * 100) / 100;
    statistics -> average_time_overhead = round(statistics -> average_time_overhead * 100) / 100;


    printf("Time overhead %g %g\n", statistics -> max_time_overhead, statistics -> average_time_overhead);
    printf("Makespan %d\n", current_time - 1);
}

//initializes the structure containing information about the performance statistics of the simulation
struct statistics *initialize_statistics(){
    struct statistics * statistics = malloc(sizeof(*statistics));

    statistics -> average_time_overhead = 0;
    statistics -> max_time_overhead = 0;
    statistics -> turnaround_time = 0;

    return statistics;
}
