// Copyright (C) 2026 Kelompok 4
#include "distributed.c"

int main() {
    distributed_main_procedure(
        "STARTING DISTRIBUTED MERGE SORT: ",
        "rank,event,left,mid,right,depth,time\n",
        "distributed",
        log_event,
        distributed_merge_sort
    );
    
    return 0;
}