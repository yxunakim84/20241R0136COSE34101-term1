#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include "scheduler.h"

// Process
#define MAX_PROCESS_NUM 10

int process_num = 0;
Process *processes[MAX_PROCESS_NUM];

// Algorithm Evaluation
Evaluation _FCFS, _NP_SJF, _P_SJF, _NP_PRIORITY, _P_PRIORITY, _ROUND_ROBIN;

// Queue
Queue ready_queue, running_queue;
int waiting_queue[MAX_PROCESS_NUM];

// Functions related to Queue
void QueueInit(Queue *pq) {
  pq->front = NULL;
  pq->rear = NULL;
}

int IsQueueEmpty(Queue *pq) {
  if(pq->front == NULL) return TRUE; // if true, return 1 
  else return FALSE; // if false, return 0
}

void Enqueue(Queue *pq, Data data) {
  Node *newNode = (Node*)malloc(sizeof(Node));
  newNode->next = NULL;
  newNode->data = data;
  
  if(IsQueueEmpty(pq)) {
    pq->front = newNode;
    pq->rear = newNode;
  } else {
    // 기존 맨 마지막 노드의 다음으로 연결
    pq->rear->next = newNode;
    // 맨 마지막 노드로 설정
    pq->rear = newNode;
  }
}

Data Dequeue(Queue *pq) {
  Node *delNode;
  Data retProcess;

  if(IsQueueEmpty(pq)) {
    printf("Dequeue Memory Error");
    exit(-1);
  }

  delNode = pq->front;
  retProcess = delNode->data;
  pq->front = pq->front->next;

  free(delNode);
  return retProcess;
}

// Create Processes
void create_processes() {
  printf("*** CPU SCHEDULER ***\n");
  printf("How many processes do you want to create?: ");
  while (1)
  {
    scanf("%d", &process_num);
    if( process_num > MAX_PROCESS_NUM || process_num < 0) printf("[ERROR] Number of processes must be 1 to 10. Please enter the number of processes again: ");
    else break;
  }

  	srand(time(NULL));

	int found[10]; int pri, flag;

  for(int i = 0; i < process_num; i++) {
    Process *process = (Process*)malloc(sizeof(Process) * 1);
    process->pid = i;
    process->burst_time = (int)(rand() % 9 + 2); // 2 ~ 10
    process->io_burst_time = (int)(rand() % 5 + 1); // 1 ~ 6
    process->arrival_time = (int)(rand() % (4 * process_num));
    process->io_start_time = (int)(rand() % (process->burst_time - 1) + 1); // 1 ~ (cpu burst time - 1)
    process->remaining_time = process->burst_time;
    process->io_remaining_time = process->io_burst_time;
    process->entered = FALSE;
    process->completed_time = 0; // 완료된 시각
    process->progress_time = 0; // 진행 정도
    
    // priority
	  while (1) {
			pri = (rand() % process_num + 1);
			flag = 0;
			for (int j = 0; j <= i; j++) {
				if (pri != found[j]) { flag++; }
			}
			if (flag == i + 1) { found[i] = pri; break; }
		}
		process->priority = found[i];

    processes[i] = process;
  };

	// 적어도 하나의 프로세스는 0초에 도착하도록 보장
  processes[(int)(rand() % process_num)]->arrival_time = 0;


	printf("----------------------------------------------------------------------------------------\n");
	printf("PID\t\tArrival Time\tCPU Burst Time\tI/O Burst Time\tI/O Start Time\tPriority\n");
	printf("----------------------------------------------------------------------------------------\n");

  for (int i = 0; i < process_num; i++) {
    printf("%d\t\t%d\t\t%d\t\t%d\t\t%d\t\t%d\n",
      processes[i]->pid,
      processes[i]->arrival_time,
      processes[i]->burst_time,
      processes[i]->io_burst_time,
      processes[i]->io_start_time,
      processes[i]->priority);
  }

  
	printf("----------------------------------------------------------------------------------------\n");
	printf("* Finished creating processes!\n\n");
}

// queue init
void initializeQueue() {
  QueueInit(&ready_queue);
  QueueInit(&running_queue);
  for(int i = 0; i < process_num; i++) {
    waiting_queue[i] = -1;
  }
	printf("* Finished creating queues!\n\n");
}

// initialize progress_time, completed_time of processes for each algorithm
void initializeProcess() {
  for (int i = 0; i < process_num; i++) {
    waiting_queue[i] = -1;
    processes[i]->progress_time = 0;
    processes[i]->completed_time = 0;

  }
}

/**
 * FCFS
*/
void FCFS() {
	printf("*** FCFS Scheduling ***\n");
  // init
  int time;
  Data selected;
  _FCFS.idle_time = 0; _FCFS.finished_process = 0;
  initializeProcess();

  for(time = 0; _FCFS.finished_process != process_num; time++) {
    for(int i = 0; i < process_num; i++) {
      // 도착한 프로세스가 있는지 확인 후 enqueue
      if(time == processes[i]->arrival_time) {
        Enqueue(&ready_queue, processes[i]->pid);
      }
      // 대기큐 - io 작업 시간 업데이트
      if(waiting_queue[i] > 0) {
        waiting_queue[i]--;
      }
      // io 작업이 끝나면 준비큐로 넘겨주기
      if(waiting_queue[i] == 0) {
        Enqueue(&ready_queue, i);
        waiting_queue[i]--;
      }
    }
    // ready queue에 프로세스가 있고, running 중인 것이 없을 때
    if(!IsQueueEmpty(&ready_queue) && IsQueueEmpty(&running_queue)) {
      selected = Dequeue(&ready_queue);
      Enqueue(&running_queue, selected);
      processes[selected]->entered = TRUE;
    }

    /**
     * i/o 상태일 때 ready queue 에 있는 프로세스 실행. ready queue 에 하나도 없으면 cpu는 idle 상태
    */
    if (IsQueueEmpty(&running_queue)) { 
      printf("TIME %d ~ %d\t: IDLE\n", time, time + 1); _FCFS.idle_time++;
    } 
    else {
      printf("TIME %d ~ %d\t: P[%d] / [%s] \n", time, time + 1, processes[selected]->pid, processes[selected]->entered == TRUE ? "✓" : " ");
      if(processes[selected]->entered == TRUE) {
        processes[selected]->entered = FALSE;
      }
      processes[selected]->progress_time++;

    
			if (processes[selected]->progress_time == processes[selected]->io_start_time) {
				int waiting = Dequeue(&running_queue);
				waiting_queue[waiting] = processes[waiting]->io_burst_time + 1;
			}
			else if (processes[selected]->progress_time == processes[selected]->burst_time) {
				_FCFS.finished_process++;
				processes[Dequeue(&running_queue)]->completed_time = time + 1;
			}
    }
  }
  _FCFS.finished_time = time;

  // Evaluation
  int total_turnaround_time = 0, total_burst_time = 0;
  for(int i = 0; i < process_num; i++) {
    total_turnaround_time += processes[i]->completed_time - processes[i]->arrival_time;
    total_burst_time += processes[i]->burst_time;
  }
  _FCFS.avg_turnaround_time = (float)total_turnaround_time / process_num;
  _FCFS.avg_waiting_time = (float)(total_turnaround_time - total_burst_time) / process_num;


	printf("\n* Average Waiting Time = %.4f", _FCFS.avg_waiting_time);
	printf("\n* Average Turnaround Time = %.4f\n", _FCFS.avg_turnaround_time);
	printf("*****************************************************************************\n\n");
}



int main() {
    create_processes();
    initializeQueue();
    FCFS();

    return 0;
}