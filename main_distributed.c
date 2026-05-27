#include "distributed.c"

int main() {
    distributed_main_procedure(
        "STARTING DISTRIBUTED MERGE SORT: ",
        "rank,event,left,mid,right,depth,time\n",
        "distributed",
        distributed_merge_sort
    );
    
    return 0;
}