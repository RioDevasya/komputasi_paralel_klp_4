// Copyright (C) 2026 Kelompok 4
#include "distributed.c"

int main() {
    distributed_main_procedure(
        "STARTING DISTRIBUTED MERGE SORT: ",
        "rank,task_created,merge_count,merge_time\n",
        "distributed",
        log_distributed_sort_event,
        distributed_merge_sort
    );
    
    return 0;
}