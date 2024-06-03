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

Data QPeek(Queue * pq) {
	if (IsQueueEmpty(pq)) {
		printf("Queue Memory Error![peek]");
		exit(-1);
	}
	return pq->front->data;
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


int compare_burst_time(const void *a, const void *b) {
    Process *p1 = (Process *)a;
    Process *p2 = (Process *)b;
    return p2->burst_time - p1->burst_time;
}


void NP_SJF() {
	printf("*** Non-Preemptive Shortest Job First Scheduling ***\n");
  int selected, time, temp, i, j;
  _NP_SJF.idle_time = 0;
  Queue tempQ;
  initializeProcess();
  QueueInit(&tempQ);

  // 각 프로세스의 PID와 CPU burst time 을 arr에 저장
  int arr[10][2] = {0};
  for(i = 0; i < process_num; i++) {
    arr[i][0] = i;
    arr[i][1] = processes[i]->burst_time;
  };

  // bust time 순으로 정렬
  for (i = 0; i < process_num; i++) {
    for (j = j + 1; j < process_num; j++) {
      if(arr[i][1] > arr[j][1]) {
        temp = arr[i][0]; arr[i][0] = arr[j][0]; arr[j][0] = temp;
        temp = arr[i][1]; arr[i][1] = arr[j][1]; arr[j][1] = temp;
      }
    }
  }

  // process order
  for(i = 0; i < process_num; i++) {
    for (j = 0; j <process_num; j++) {
      if(processes[i]->pid == arr[j][0]) {
        processes[i]->order = j;
      }
    }
  }

  for(time = 0; _NP_SJF.finished_process != process_num; time++) {
    for(i = 0; i < process_num; i++) {
      if(waiting_queue[i] > 0) {
        waiting_queue[i]--;
      }
      if(waiting_queue[i] == 0 || time == processes[i]->arrival_time) {
        if(waiting_queue[i] == 0) {
          waiting_queue[i]--;
        }
        if(IsQueueEmpty(&ready_queue)) {
          Enqueue(&ready_queue, processes[i]->pid);
        }
        // 임시 큐를 사용하여 적절한 위치에 프로세스를 삽입한 후 다시 원래 큐로 복원
        else {
          while (processes[QPeek(&ready_queue)]->order < processes[i]->order) {
            Enqueue(&tempQ, Dequeue(&ready_queue));
            if(IsQueueEmpty(&ready_queue)) break;
          }
          Enqueue(&tempQ, processes[i]->pid);
          while (!IsQueueEmpty(&ready_queue)) {
            Enqueue(&tempQ, Dequeue(&ready_queue));
          }
          while (!IsQueueEmpty(&tempQ)) {
            Enqueue(&ready_queue, Dequeue(&tempQ));
          }
        }
      }
    }

    if(!IsQueueEmpty(&ready_queue) && IsQueueEmpty(&running_queue)) {
      selected = Dequeue(&ready_queue);
      Enqueue(&running_queue, selected);
      processes[selected]->entered = TRUE;
    }

    /**
     * i/o 상태일 때 ready queue 에 있는 프로세스 실행. ready queue 에 하나도 없으면 cpu는 idle 상태
    */
    if (IsQueueEmpty(&running_queue)) { 
      printf("TIME %d ~ %d\t: IDLE\n", time, time + 1); _NP_SJF.idle_time++;
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
				_NP_SJF.finished_process++;
				processes[Dequeue(&running_queue)]->completed_time = time + 1;
			}
    }
  }

  _NP_SJF.finished_time = time;

   // Evaluation
  int total_turnaround_time = 0, total_burst_time = 0;
  for(int i = 0; i < process_num; i++) {
    total_turnaround_time += processes[i]->completed_time - processes[i]->arrival_time;
    total_burst_time += processes[i]->burst_time;
  }
  _NP_SJF.avg_turnaround_time = (float)total_turnaround_time / process_num;
  _NP_SJF.avg_waiting_time = (float)(total_turnaround_time - total_burst_time) / process_num;


	printf("\n* Average Waiting Time = %.4f", _NP_SJF.avg_waiting_time);
	printf("\n* Average Turnaround Time = %.4f\n", _NP_SJF.avg_turnaround_time);
	printf("*****************************************************************************\n\n");
}

void P_SJF() {
  printf("*** Preemptive Shortest Job First Scheduling ***\n");
  int selected, time, preempted, i, j;
  _P_SJF.idle_time = 0;
  _P_SJF.finished_process = 0;
  Queue tempQ;
  initializeProcess();
  QueueInit(&tempQ);
  
  for(time = 0; _P_SJF.finished_process != process_num; time++) {
    for(i = 0; i < process_num; i++) {
      if(waiting_queue[i] > 0) {
        waiting_queue[i]--;
      }

      if(waiting_queue[i] == 0 || time == processes[i]->arrival_time) {
        if(waiting_queue[i] == 0) {
          waiting_queue[i]--;
        }

        if(IsQueueEmpty(&ready_queue) && IsQueueEmpty(&running_queue)) {
          Enqueue(&ready_queue, processes[i]->pid);
        }
        // 선점 로직
        else if(!IsQueueEmpty(&running_queue)) {
          // 현재 진행중인 프로세스의 remaining burst time 보다 다른 프로세스의 remaining burst time 이 더 적은 경우 선점
          if(processes[i]->remaining_time < processes[QPeek(&running_queue)]->remaining_time) {
            preempted = Dequeue(&running_queue);
            while (!IsQueueEmpty(&ready_queue))
            {
             Enqueue(&tempQ, Dequeue(&ready_queue));
            }
            Enqueue(&ready_queue, preempted);
            while (!IsQueueEmpty(&tempQ))
            {
              Enqueue(&ready_queue, Dequeue(&tempQ));
            }
            // -> 선점 후 ready queue에 있던 나머지 프로세스들 다시 넣어주는 것
            // 현재 더 우선순위의 프로세스를 running queue 에 넣어줌
            Enqueue(&running_queue, i);
            processes[i]->entered = TRUE;

            if(processes[preempted]->preemptive == 1) {
              processes[preempted]->preemptive = 0;
            }
          }
          else {
            if(IsQueueEmpty(&ready_queue)) {
              Enqueue(&ready_queue, i);
            }
            // 현재 작업중인 프로세스보다는 후순위인데, ready queue 에서 순서 조정이 다시 필요
            else {
              while (processes[QPeek(&ready_queue)]->remaining_time < processes[i]->remaining_time) {
                Enqueue(&tempQ, Dequeue(&ready_queue));
                if(IsQueueEmpty(&ready_queue)) {
                  break;
                }
              }
              // ready queue 보다 우선순위이면 temp 에 넣어주기
              Enqueue(&tempQ, processes[i]->pid);
              // 나머지 프로세스들 tempQ로 이동
              while (!IsQueueEmpty(&ready_queue)) {
                Enqueue(&tempQ, Dequeue(&ready_queue));
              }
              // tempQ에서 다시 ready queue 로 이동
              while(!IsQueueEmpty(&tempQ)) {
                Enqueue(&ready_queue, Dequeue(&tempQ));
              }
            }
          }
        }

        // ready queue 에는 값이 있고 running queue 에는 값이 없을 때
        // ready queue 우선순위로 정렬
        else {
          while(processes[QPeek(&ready_queue)]->remaining_time < processes[i]->remaining_time) {
            Enqueue(&tempQ, Dequeue(&ready_queue));
            if(IsQueueEmpty(&ready_queue)) break;
          };
          Enqueue(&tempQ, processes[i]->pid);
          // 나머지 프로세스들 tempQ로 이동
          while (!IsQueueEmpty(&ready_queue)) {
            Enqueue(&tempQ, Dequeue(&ready_queue));
          }
          // tempQ에서 다시 ready queue 로 이동
          while(!IsQueueEmpty(&tempQ)) {
            Enqueue(&ready_queue, Dequeue(&tempQ));
          }
        }
      }
    }

    if(!IsQueueEmpty(&running_queue)) {
      selected = QPeek(&running_queue);
    }

    if(!IsQueueEmpty(&ready_queue) && IsQueueEmpty(&running_queue)) {
      selected = Dequeue(&ready_queue);
      Enqueue(&running_queue, selected);
      processes[selected]->entered = TRUE;
      processes[selected]->preemptive = 1;
    }
    
    /**
     * i/o 상태일 때 ready queue 에 있는 프로세스 실행. ready queue 에 하나도 없으면 cpu는 idle 상태
    */
    if (IsQueueEmpty(&running_queue)) { 
      printf("TIME %d ~ %d\t: IDLE\n", time, time + 1); _P_SJF.idle_time++;
    } 
    else {
      printf("TIME %d ~ %d\t: P[%d] / [%s] \n", time, time + 1, processes[selected]->pid, processes[selected]->entered == TRUE ? "✓" : " ");
      if(processes[selected]->entered == TRUE) {
        processes[selected]->entered = FALSE;
      }
      processes[selected]->remaining_time--;
      processes[selected]->progress_time++;

    
			if (processes[selected]->progress_time == processes[selected]->io_start_time) {
				int waiting = Dequeue(&running_queue);
				waiting_queue[waiting] = processes[waiting]->io_burst_time + 1;
			}
			else if (processes[selected]->progress_time == processes[selected]->burst_time) {
				_P_SJF.finished_process++;
				processes[Dequeue(&running_queue)]->completed_time = time + 1;
			}
    }
  }

  _P_SJF.finished_time = time;

  // Evaluation
  int total_turnaround_time = 0, total_burst_time = 0;
  for(int i = 0; i < process_num; i++) {
    total_turnaround_time += processes[i]->completed_time - processes[i]->arrival_time;
    total_burst_time += processes[i]->burst_time;
  }
  _P_SJF.avg_turnaround_time = (float)total_turnaround_time / process_num;
  _P_SJF.avg_waiting_time = (float)(total_turnaround_time - total_burst_time) / process_num;


	printf("\n* Average Waiting Time = %.4f", _P_SJF.avg_waiting_time);
	printf("\n* Average Turnaround Time = %.4f\n", _P_SJF.avg_turnaround_time);
	printf("*****************************************************************************\n\n");
}

/**
 * NP_PRIORITY
*/
void NP_PRIORITY() {
  printf("*** Non-Preemptive Priority Scheduling ***\n");
  int selected, time, i;
  _NP_PRIORITY.idle_time = 0;
  _NP_PRIORITY.finished_process = 0;
  Queue tempQ;
  initializeProcess();
  QueueInit(&tempQ);

  for(time = 0; _NP_PRIORITY.finished_process != process_num; time++) {
    for(i = 0; i < process_num; i++) {
        if(waiting_queue[i] > 0) {
        waiting_queue[i]--;
      }

      if(waiting_queue[i] == 0 || time == processes[i]->arrival_time) {
        if(waiting_queue[i] == 0) {
          waiting_queue[i]--;
        }

        if(IsQueueEmpty(&ready_queue)) {
          Enqueue(&ready_queue, processes[i]->pid);
        }
        else {
          // ready queue 에서 현재 프로세스보다 우선순위 높은 것들 먼저 넣기
          while (processes[QPeek(&ready_queue)]->priority < processes[i]->priority) {
            Enqueue(&tempQ, Dequeue(&ready_queue));
            if(IsQueueEmpty(&ready_queue)) {
              break;
            }
          }
          // 현재 프로세스 넣기
          Enqueue(&tempQ, processes[i]->pid);
          // 현재 프로세스보다 우선순위 낮은 나머지 프로세스들 넣기
          while (!IsQueueEmpty(&ready_queue)) {
            Enqueue(&tempQ, Dequeue(&ready_queue));
          }
          while (!IsQueueEmpty(&tempQ)) {
            Enqueue(&ready_queue, Dequeue(&tempQ));
          }
        }
      }
    }
    if(!IsQueueEmpty(&ready_queue) && IsQueueEmpty(&running_queue)) {
      selected = Dequeue(&ready_queue);
      Enqueue(&running_queue, selected);
      processes[selected]->entered = TRUE;
    }
    if(IsQueueEmpty(&running_queue)) {
      printf("TIME %d ~ %d\t: IDLE\n", time, time + 1); _NP_PRIORITY.idle_time++;
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
        _NP_PRIORITY.finished_process++;
        processes[Dequeue(&running_queue)]->completed_time = time + 1;
      }
    }
  }
  _NP_PRIORITY.finished_time = time;

  // Evaluation
  int total_turnaround_time = 0, total_burst_time = 0;
  for(int i = 0; i < process_num; i++) {
    total_turnaround_time += processes[i]->completed_time - processes[i]->arrival_time;
    total_burst_time += processes[i]->burst_time;
  }
  _NP_PRIORITY.avg_turnaround_time = (float)total_turnaround_time / process_num;
  _NP_PRIORITY.avg_waiting_time = (float)(total_turnaround_time - total_burst_time) / process_num;


	printf("\n* Average Waiting Time = %.4f", _NP_PRIORITY.avg_waiting_time);
	printf("\n* Average Turnaround Time = %.4f\n", _NP_PRIORITY.avg_turnaround_time);
	printf("*****************************************************************************\n\n");
}

void P_PRIORITY() {
  printf("*** Preemptive Priority Scheduling ***\n");
  int selected, time, i, preempted;
  _P_PRIORITY.idle_time = 0;
  _P_PRIORITY.finished_process = 0;
  Queue tempQ;
  initializeProcess();
  QueueInit(&tempQ);

  for(time = 0; _P_PRIORITY.finished_process != process_num; time++) {
    for(i = 0; i < process_num; i++) {
      if(waiting_queue[i] > 0) {
        waiting_queue[i]--;
      }

      if(waiting_queue[i] == 0 || time == processes[i]->arrival_time) {
        if(waiting_queue[i] == 0) {
          waiting_queue[i]--;
        }

        if(IsQueueEmpty(&ready_queue) && IsQueueEmpty(&running_queue)) {
          Enqueue(&ready_queue, processes[i]->pid);
        }
        // 선점 로직
        else if(!IsQueueEmpty(&running_queue)) {
          // 현재 진행중인 프로세스의 remaining burst time 보다 다른 프로세스의 remaining burst time 이 더 적은 경우 선점
          if(processes[i]->priority < processes[QPeek(&running_queue)]->priority) {
            preempted = Dequeue(&running_queue);
            while (!IsQueueEmpty(&ready_queue))
            {
             Enqueue(&tempQ, Dequeue(&ready_queue));
            }
            Enqueue(&ready_queue, preempted);
            while (!IsQueueEmpty(&tempQ))
            {
              Enqueue(&ready_queue, Dequeue(&tempQ));
            }
            // -> 선점 후 ready queue에 있던 나머지 프로세스들 다시 넣어주는 것
            // 현재 더 우선순위의 프로세스를 running queue 에 넣어줌
            Enqueue(&running_queue, i);
            processes[i]->entered = TRUE;

            if(processes[preempted]->preemptive == 1) {
              processes[preempted]->preemptive = 0;
            }
          }
          else {
            if(IsQueueEmpty(&ready_queue)) {
              Enqueue(&ready_queue, i);
            }
            else {
              while (processes[QPeek(&ready_queue)]->priority < processes[i]->priority) {
                Enqueue(&tempQ, Dequeue(&ready_queue));
                if(IsQueueEmpty(&ready_queue)) {
                  break;
                }
              }
              Enqueue(&tempQ, processes[i]->pid);
              // 나머지 프로세스들 tempQ로 이동
              while (!IsQueueEmpty(&ready_queue)) {
                Enqueue(&tempQ, Dequeue(&ready_queue));
              }
              // tempQ에서 다시 ready queue 로 이동
              while(!IsQueueEmpty(&tempQ)) {
                Enqueue(&ready_queue, Dequeue(&tempQ));
              }
            }
          }
        }

        // ready queue 에는 값이 있고 running queue 에는 값이 없을 때
        // ready queue 우선순위로 정렬
        else {
          while(processes[QPeek(&ready_queue)]->priority < processes[i]->priority) {
            Enqueue(&tempQ, Dequeue(&ready_queue));
            if(IsQueueEmpty(&ready_queue)) break;
          };
          Enqueue(&tempQ, processes[i]->pid);
          // 나머지 프로세스들 tempQ로 이동
          while (!IsQueueEmpty(&ready_queue)) {
            Enqueue(&tempQ, Dequeue(&ready_queue));
          }
          // tempQ에서 다시 ready queue 로 이동
          while(!IsQueueEmpty(&tempQ)) {
            Enqueue(&ready_queue, Dequeue(&tempQ));
          }
        }
      }
    }


    if(!IsQueueEmpty(&running_queue)) {
      selected = QPeek(&running_queue);
    }

    if(!IsQueueEmpty(&ready_queue) && IsQueueEmpty(&running_queue)) {
      selected = Dequeue(&ready_queue);
      Enqueue(&running_queue, selected);
      processes[selected]->entered = TRUE;
      processes[selected]->preemptive = 1;
    }
    
    /**
     * i/o 상태일 때 ready queue 에 있는 프로세스 실행. ready queue 에 하나도 없으면 cpu는 idle 상태
    */
    if (IsQueueEmpty(&running_queue)) { 
      printf("TIME %d ~ %d\t: IDLE\n", time, time + 1); _P_PRIORITY.idle_time++;
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
				_P_PRIORITY.finished_process++;
				processes[Dequeue(&running_queue)]->completed_time = time + 1;
			}
    }
  }

  _P_PRIORITY.finished_time = time;

  // Evaluation
  int total_turnaround_time = 0, total_burst_time = 0;
  for(int i = 0; i < process_num; i++) {
    total_turnaround_time += processes[i]->completed_time - processes[i]->arrival_time;
    total_burst_time += processes[i]->burst_time;
  }
  _P_PRIORITY.avg_turnaround_time = (float)total_turnaround_time / process_num;
  _P_PRIORITY.avg_waiting_time = (float)(total_turnaround_time - total_burst_time) / process_num;


	printf("\n* Average Waiting Time = %.4f", _P_PRIORITY.avg_waiting_time);
	printf("\n* Average Turnaround Time = %.4f\n", _P_PRIORITY.avg_turnaround_time);
	printf("*****************************************************************************\n\n");

}





int main() {
    create_processes();
    initializeQueue();
    // FCFS();

    // Non-preemptive SJF
    // NP_SJF();
    // Preemptive SJF
    // P_SJF();

    // Non-preemptive Priority
    // NP_PRIORITY();
    // Preemptive Priority
    P_PRIORITY();


    return 0;
}