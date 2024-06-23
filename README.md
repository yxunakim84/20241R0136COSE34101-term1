# 20241R0136COSE34101-term1

#### 2020130826 김유나

OS-CPU 스케줄링 시뮬레이터 텀 프로젝트

## Term Project 의 목적

- OS 의 기본 개념을 바탕으로 CPU 스케줄러의 기능 및 작동 원리를 이해하는데 있다.
- Project 의 수행을 통해 Linux 환경에서의 코딩 능력을 향상시킬 수 있다.

## CPU Scheduling Simulator 의 구성

- create_process(): 실행할 프로세스를 생성하고 각 프로세스에 데이터가 주어진다. (Random data 부여)

  - pid (Process Id)
  - arrival_time
  - burst_time
  - io_burst_time
  - io_start_time
  - priority
  - remaining_time
  - completed_time
  - waiting_time
  - turnaround_time
  - progress_time
  - entered
  - order (#np_sjf)
  - preemptive (#p_sjf)
  - time_quantum (#RR)

- Config(): 시스템 환경 설정

  - Ready Queue / Waiting Queue

- Scheduling Algorithm: CPU 스케줄링 알고리즘

  - FCFS(First Come First Served)
  - SJF(Shortest Job First)
  - Priority
  - RR(Round Robin)
  - NP_SJF(Shortest Job First)
  - P_SJF(Shortest Job First, Preemptive 방식 적용)
  - NP_Priority
  - P_Priority(Preemptive 방식 적용)

- evaluation(): 각 스케줄링 알고리즘들간 비교 평가한다.

  - Average waiting time
  - Average turnaround time
  - CPU Utilization

  ## 시뮬레이터의 시스템 구성도

  ![simulator](/assets/simulator.png)

1. new
   : create_processes()를 통해 새로운 프로세스들을 생성하고, Config()를 실행하여 CPU simulating에 필요한 ready queue, running queue, waiting queue를 생성하고 초기화한다.
2. ready queue
   : 각 프로세스의 arrival_time에 해당되면 ready queue로 이동한다. CPU 작업을 수행하다가 작업 이 끝나기 전에 다른 프로세스에 의해 선점된 경우에도 ready queue로 다시 이동하며, I/O 작업이 끝난 프로세스도 ready queue로 옮겨진다.
3. running queue
   : CPU 스케줄링 기법에 따라, CPU 스케줄러는 ready queue에 있는 프로세스 중 하나를 running queue로 이동시킨다. 다른 프로세스의 선점이 발생한 경우 기존에 수행되던 프로세스는 running queue에서 ready queue로 이동된다.
4. waiting queue
   : 각 프로세스에는 io_burst_time과 io_start_time 값이 존재하는데, I/O 작업 시간에 해당되면 running queue에서 빼낸 후 waiting queue로 이동시키고, waiting queue의 해당 프로세스의 pid에 해당하는 자리에 io_burst_time을 넣어준 후 매 시간마다 감소시켜준다. I/O 작업이 끝나고 아직 burst_time이 남아있는 경우에는 ready queue로 다시 이동시킨다.
5. terminated
   : 프로세스의 progress_time(cpu 작업이 진행된 정도)과 burst_time이 같아지면, CPU 작업을 전 부 완료했다는 뜻이므로, Terminated 상태로 처리해줘야 한다. 스케줄링 기법의 속성 중 finished_process의 값을 1 증가시켜 완료된 프로세스의 개수를 관리하고, 프로세스의 completed_time에도 끝난 시간을 저장해놓는다. 모든 프로세스의 작업이 종료되었다면, Average Waiting Time과 Average Turnaround Time을 계산해주는 evaluation 작업을 수행한다.
