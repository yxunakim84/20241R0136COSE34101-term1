#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__

#define TRUE	1
#define FALSE	0

/**
 * Process
*/
typedef struct {
    int pid;
    int arrival_time;
    int burst_time;
    int io_burst_time;
    int io_start_time;
    int priority;
    int remaining_time;
    int io_remaining_time;
    int completed_time;
    int waiting_time;
    int turnaround_time;
    int progress_time;
    _Bool entered;
    int order; // np_sjf
    int preemptive; // p_sjf

} Process;

/**
 * Algorithm Evaluation
*/
typedef struct _evaluation {
  float avg_turnaround_time;
  float avg_waiting_time;
  int idle_time;
  int finished_process;
  int finished_time;
} Evaluation;

/**
 * Queue
*/
typedef int Data;

typedef struct _node {
  Data data;
  struct _node *next;
} Node;

typedef struct _Queue {
  Node *front;
  Node *rear;
} Queue;

#endif