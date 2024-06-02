#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h> 
#include <time.h>

#define PROCESS_COUNT 5
#define IO_OPERATION_OCCUR 2000
#define IO_OPERATION_TIME 5

static int process_pid_count = 1;
static int cpu_time = 0;
static struct process_* process_list[PROCESS_COUNT];
//static struct process_* dup_process_list[PROCESS_COUNT];

static int cpu_mutex = 0; // 0�� CPU ��� x, 1�� CPU ��� ��

typedef struct process_ { // ���μ��� ����ü
    int ProcessID;
    int CPU_burst_time;
    int CPU_bursted_time;
    int I_O_burst_time;
    int Arrival_time;
    int Priority;
    struct process_* next;
} process;

typedef struct Queue_ { // ���μ��� ť ����ü
    process* front;
    process* rear;
} Queue;

typedef struct result_ {
    int avr_waiting_time;
    int avr_turnaround_time;
} result;

result schedule_result[6]; //�����췯 �� ����

//��Ʈ��Ʈ�� ���� �迭
int gant_chart[6][150]; //1���� ���ʷ� fcfs non_preemp~~

Queue* create_queue();
void enqueue(Queue* q, process* p);
process* dequeue(Queue* q);
void swap_node(Queue* q, process* a, process* b);
process* create_process(int CPU_burst_time, int I_O_burst_time, int Arrival_time, int Priority);
void config(Queue** ready_queue, Queue** waiting_queue);
void schedule();
void evaluation(result, result, result, result, result, result);
void CPU_Scheduling_Simulator();
void create_random_processes(process* process_list[PROCESS_COUNT]);
//void create_dup_procesess(process* process_list[PROCESS_COUNT], process* dup_process_list[PROCESS_COUNT]);
int dispatch_cpu(process* process, int burst_time, int* time);
int is_IO_occur(int time);
void update_waiting_queue(Queue* waiting_queue, Queue* ready_queue, int current_time);
void enqueue_by_arrive_time(Queue* ready_queue, process* process_list[PROCESS_COUNT], int time);
void fcfs_scheduler(Queue* ready_queue, Queue* waiting_queue, result* result);
void non_preemptive_sjf(Queue* ready_queue, Queue* waiting_queue, result* res);
void preemptive_sjf(Queue* ready_queue, Queue* waiting_queue, result* res);
void non_preemptive_priority(Queue* ready_queue, Queue* waiting_queue, result* res);
void preemptive_priority(Queue* ready_queue, Queue* waiting_queue, result* res);
void round_robin(Queue* ready_queue, Queue* waiting_queue, result* res);
void init();
void clear_screen();
int result_view();
void reset_queue(Queue** ready_queue, Queue** waiting_queue);
void reset_process(process* process_list[PROCESS_COUNT]);
void print_gantt_chart(int, int);
void cpu_run(process*);
void cpu_out(process*);
void show_process();
Queue** ready_queue;
Queue** waiting_queue;

int main(void) {
    CPU_Scheduling_Simulator(); // �ùķ����� ����
    return 0;
}



void CPU_Scheduling_Simulator() {
    while (1) {
        init();
        config(&ready_queue, &waiting_queue); // �ý��� ȯ�� ����
        create_random_processes(process_list); // ������ ������� �򰡸� ���� ���μ��� ����
       // create_dup_procesess(process_list, dup_process_list); //���μ��� ���纻 ����
        schedule(); // �����ٸ� ����
        int n=  result_view();
        if (n == -1) {
            break;
        }
    }
}

Queue* create_queue() {
    Queue* q = (Queue*)malloc(sizeof(Queue));
    q->front = q->rear = NULL;
    return q;
}

void enqueue(Queue* q, process* p) {
    p->next = NULL; // ���� �߰��Ǵ� ���μ����� next �����͸� NULL�� ����

    if (q->rear == NULL) {
        // ť�� ��� �ִ� ���
        q->front = q->rear = p;
    }
    else {
        // ť�� ��� ���� ���� ���
        q->rear->next = p;
        q->rear = p;
    }
}

process* dequeue(Queue* q) {
    if (q == NULL || q->front == NULL) {
        return NULL; // ť�� ��� �ְų� NULL�� ���
    }

    process* temp = q->front;
    q->front = q->front->next; // front�� ���� ���� ������Ʈ

    if (q->front == NULL) { // ť�� ���Ұ� �ϳ��� �־��� ���
        q->rear = NULL; // rear�� NULL�� ����
        q->front = NULL; //front�� NULL�� ����
    }

    temp->next = NULL; // ��ȯ�� ����� ���� ��带 NULL�� �����Ͽ� ť���� ���� ����
    return temp;
}

process* create_process(int CPU_burst_time, int I_O_burst_time, int Arrival_time, int Priority) {
    process* new_process = (process*)malloc(sizeof(process));

    // PID �� �Ҵ�
    if (process_pid_count != 32769) {
        new_process->ProcessID = process_pid_count++;
    }
    else {
        new_process->ProcessID = 1;
    }
    // ������ �� �Ҵ�
    new_process->CPU_burst_time = CPU_burst_time;
    new_process->CPU_bursted_time = 0;
    new_process->I_O_burst_time = I_O_burst_time;
    new_process->Arrival_time = Arrival_time;
    new_process->Priority = Priority;
    new_process->next = NULL;

    return new_process;
}

// waiting ť�� ready ť ȯ�漳��
void config(Queue** ready_queue, Queue** waiting_queue) {
    // �ùķ����͸� ���� ť ����
    *ready_queue = create_queue();
    *waiting_queue = create_queue();

    //��Ʈ��Ʈ �ʱ�ȭ
    for (int i = 0; i < 6; i++) {
        for (int k = 0; k < 150; k++) {
            gant_chart[i][k] = -1;
        }
    }
}

// ������ ������� �򰡸� ���� ���μ��� ������ ������ŭ ����
void create_random_processes(process* process_list[PROCESS_COUNT] ) {
    srand(time(NULL)); // ���� �ʱ�ȭ

    // ���μ��� ����, ���� ����
    for (int i = 0; i < PROCESS_COUNT; i++) {
        int cpu_burst_time = rand() % 20 + 5; // 5�� 20 ����
        int I_O_burst_time = rand() % 20 + 1; // 1�� 20 ����
        int Arrival_time = rand() % 50 + 1; // 1�� 50 ����
        int Priority = rand() % 100 + 1; // 1�� 10 ����

        process* temp;
        //process* temp2;

        // ���μ��� ���� �� ����Ʈ�� ���� �ִ´�
        temp = create_process(cpu_burst_time, I_O_burst_time, Arrival_time, Priority);
        //temp2 = create_process(cpu_burst_time, I_O_burst_time, Arrival_time, Priority);
        process_list[i] = temp;
        //dup_process_list[i] = temp2;
        
    }
    
}

// �����층 & �򰡴ܰ�
void schedule() {


    reset_queue(&ready_queue, &waiting_queue);
    reset_process(process_list);
    result fcfs_result;
    fcfs_scheduler(ready_queue, waiting_queue, &fcfs_result);
    printf("FCFS scheduling complete.\n");



    reset_queue(&ready_queue, &waiting_queue);
    reset_process(process_list);
    result non_preemptive_sjf_result;
    non_preemptive_sjf(ready_queue, waiting_queue, &non_preemptive_sjf_result);
    printf("Non-preemptive SJF scheduling complete.\n");

    reset_queue(&ready_queue, &waiting_queue);
    reset_process(process_list);
    result preemptive_sjf_result;
    preemptive_sjf(ready_queue, waiting_queue, &preemptive_sjf_result);
    printf("Preemptive SJF scheduling complete.\n");

    reset_queue(&ready_queue, &waiting_queue);
    reset_process(process_list);
    result non_preemptive_priority_result;
    non_preemptive_priority(ready_queue, waiting_queue, &non_preemptive_priority_result);
    printf("Non-preemptive Priority scheduling complete.\n");

    reset_queue(&ready_queue, &waiting_queue);
    reset_process(process_list);
    result preemptive_priority_result;
    preemptive_priority(ready_queue, waiting_queue, &preemptive_priority_result);
    printf("Preemptive Priority scheduling complete.\n");

    reset_queue(&ready_queue, &waiting_queue);
    reset_process(process_list);
    result round_robin_result;
    round_robin(ready_queue, waiting_queue, &round_robin_result);
    printf("Round Robin scheduling complete.\n");

    evaluation(fcfs_result, non_preemptive_sjf_result, preemptive_sjf_result, non_preemptive_priority_result, preemptive_priority_result, round_robin_result); // ��
}

void fcfs_scheduler(Queue* ready_queue, Queue* waiting_queue, result* result) {
    cpu_time = 0;
    int turnaround_time = 0;
    int waiting_time = 0;
    int turnaround_time_sum = 0;
    int waiting_time_sum = 0;
    int count = PROCESS_COUNT;

    while (cpu_time < 150) {
       
        enqueue_by_arrive_time(ready_queue, process_list, cpu_time);
        update_waiting_queue(waiting_queue, ready_queue, cpu_time);
        
    // printf("%d", 1);

        if ((ready_queue->front != NULL) && cpu_mutex == 0) {
        //  printf("%d", 2);
            process* current = ready_queue->front;
            process* earliest_process = current;
            while (current != NULL) {
                if (current->Arrival_time < earliest_process->Arrival_time) {
                    earliest_process = current;
                }
                current = current->next;
            }
         // printf("%d", 3);
           
            /*
            if (earliest_process != ready_queue->front) {
                swap_node(ready_queue, ready_queue->front, earliest_process);
            }
            */
            
            process* execute_process = earliest_process;
          //  printf("%d", 4);
            cpu_mutex = 1;
            int burst_time = execute_process->CPU_burst_time - execute_process->CPU_bursted_time;
            for (int i = 0; i < burst_time; i++) {
                cpu_run(execute_process);
                gant_chart[0][cpu_time] = execute_process->ProcessID;
                cpu_time++;

                enqueue_by_arrive_time(ready_queue, process_list, cpu_time);

                if (cpu_time == IO_OPERATION_OCCUR) {
                    execute_process->I_O_burst_time = cpu_time + IO_OPERATION_TIME;
                    dequeue(ready_queue);
                    enqueue(waiting_queue, execute_process);
                    cpu_mutex = 0;
                    break;
                }
                else {
                    execute_process->CPU_bursted_time += 1;
                }
            }
          //  printf("%d", 5);
            cpu_mutex = 0;
            if (execute_process->CPU_bursted_time >= execute_process->CPU_burst_time) {
                turnaround_time = cpu_time - execute_process->Arrival_time;
                waiting_time = turnaround_time - execute_process->CPU_burst_time;
                turnaround_time_sum += turnaround_time;
                waiting_time_sum += waiting_time;
                cpu_out(execute_process);
                // Remove shortest_process from ready_queue
                if (execute_process == ready_queue->front) {
                    dequeue(ready_queue);
                }
                else if (execute_process == ready_queue->rear) {
                    process* prev = ready_queue->front;
                    while (prev->next != execute_process) {
                        prev = prev->next;
                    }
                    prev->next = NULL;
                }
                else {
                    process* prev = ready_queue->front;
                    while (prev->next != execute_process) {
                        prev = prev->next;
                    }
                    prev->next = execute_process->next;
                }
            }
         //   printf("%d", 6);
        }
        else {
            gant_chart[0][cpu_time] = -1; //-1�� �ƹ��͵� ���������ʾҴٴ� ��
            cpu_time++;
        }
    }

    result->avr_turnaround_time = turnaround_time_sum / count;
    result->avr_waiting_time = waiting_time_sum / count;
}

void non_preemptive_sjf(Queue* ready_queue, Queue* waiting_queue, result* res) {
    int cpu_time = 0;
    int turnaround_time = 0;
    int waiting_time = 0;
    int turnaround_time_sum = 0;
    int waiting_time_sum = 0;
    int count = PROCESS_COUNT;
    int cpu_mutex = 0;

    while (cpu_time < 150) {
        enqueue_by_arrive_time(ready_queue, process_list, cpu_time);
        update_waiting_queue(waiting_queue, ready_queue, cpu_time);

        if ((ready_queue->front != NULL) && cpu_mutex == 0) {
            process* shortest_process = ready_queue->front;
            process* current = ready_queue->front;
            process* previous = NULL;
            process* shortest_previous = NULL;

            // Find the process with the shortest CPU burst time
            while (current != NULL) {
                if (current->CPU_burst_time < shortest_process->CPU_burst_time) {
                    shortest_process = current;
                    shortest_previous = previous;
                }
                previous = current;
                current = current->next;
            }

            cpu_mutex = 1;
            int burst_time = shortest_process->CPU_burst_time - shortest_process->CPU_bursted_time;
            for (int i = 0; i < burst_time; i++) {
                cpu_run(shortest_process);
                gant_chart[1][cpu_time] = shortest_process->ProcessID;
                cpu_time++;
                enqueue_by_arrive_time(ready_queue, process_list, cpu_time);
                update_waiting_queue(waiting_queue, ready_queue, cpu_time);

                if (cpu_time == IO_OPERATION_OCCUR) {
                    shortest_process->I_O_burst_time = cpu_time + IO_OPERATION_TIME;
                    if (shortest_previous == NULL) {
                        ready_queue->front = shortest_process->next;
                    }
                    else {
                        shortest_previous->next = shortest_process->next;
                    }
                    enqueue(waiting_queue, shortest_process);
                    cpu_mutex = 0;
                    break;
                }
                else {
                    shortest_process->CPU_bursted_time += 1;
                }
            }

            // Check if the process has completed its CPU burst
            if (shortest_process->CPU_bursted_time >= shortest_process->CPU_burst_time) {
                turnaround_time = cpu_time - shortest_process->Arrival_time;
                waiting_time = turnaround_time - shortest_process->CPU_burst_time;
                turnaround_time_sum += turnaround_time;
                waiting_time_sum += waiting_time;
                cpu_out(shortest_process);
                /*
                if (shortest_previous == NULL) {
                    ready_queue->front = shortest_process->next;
                }
                else {
                    shortest_previous->next = shortest_process->next;
                }
                */
                if (shortest_process == ready_queue->front) {
                    dequeue(ready_queue);
                }
                else if (shortest_process == ready_queue->rear) {
                    process* prev = ready_queue->front;
                    while (prev->next != shortest_process) {
                        prev = prev->next;
                    }
                    prev->next = NULL;
                }
                else {
                    process* prev = ready_queue->front;
                    while (prev->next != shortest_process) {
                        prev = prev->next;
                    }
                    prev->next = shortest_process->next;
                }
                cpu_mutex = 0;  // Make sure to release the mutex here
            }
        }
        else {
            gant_chart[1][cpu_time] = -1;  // ���� �ð��� ó���մϴ�.
            cpu_time++;
        }
    }

    res->avr_turnaround_time = turnaround_time_sum / count;
    res->avr_waiting_time = waiting_time_sum / count;
}

void preemptive_sjf(Queue* ready_queue, Queue* waiting_queue, result* res) {
    cpu_time = 0;
    int turnaround_time = 0;
    int waiting_time = 0;
    int turnaround_time_sum = 0;
    int waiting_time_sum = 0;
    int count = PROCESS_COUNT;
    cpu_mutex = 0;

    enqueue_by_arrive_time(ready_queue, process_list, cpu_time);

    update_waiting_queue(waiting_queue, ready_queue, cpu_time);
    while (cpu_time < 150) {
      
        //enqueue_by_arrive_time(ready_queue, process_list, cpu_time);

        //update_waiting_queue(waiting_queue, ready_queue, cpu_time);

        if (ready_queue->front != NULL) {
            process* shortest_process = ready_queue->front;
            process* current = ready_queue->front;

            // Find the process with the shortest remaining burst time
            while (current != NULL) {
                if ((current->CPU_burst_time - current->CPU_bursted_time) < (shortest_process->CPU_burst_time - shortest_process->CPU_bursted_time)) {
                    shortest_process = current;
                }
                current = current->next;
            }

            // ��Ʈ ��Ʈ�� ���� �ð��� ���� ���� ���μ����� ���
            cpu_run(shortest_process);
            gant_chart[2][cpu_time] = shortest_process->ProcessID;
            shortest_process->CPU_bursted_time += 1;
            cpu_time++;
            enqueue_by_arrive_time(ready_queue, process_list, cpu_time);

            update_waiting_queue(waiting_queue, ready_queue, cpu_time);

            // Check if the shortest process needs I/O
            if (shortest_process->CPU_bursted_time == IO_OPERATION_OCCUR) {
                shortest_process->I_O_burst_time = cpu_time + IO_OPERATION_TIME;
                // Remove shortest_process from ready_queue and add to waiting_queue
                if (shortest_process == ready_queue->front) {
                    dequeue(ready_queue);
                }
                else {
                    process* prev = ready_queue->front;
                    while (prev->next != shortest_process) {
                        prev = prev->next;
                    }
                    prev->next = shortest_process->next;
                }
                enqueue(waiting_queue, shortest_process);
            }
            else if (shortest_process->CPU_bursted_time == shortest_process->CPU_burst_time) {
                // Process has completed its execution
                cpu_out(shortest_process);
                turnaround_time = cpu_time - shortest_process->Arrival_time;
                waiting_time = turnaround_time - shortest_process->CPU_burst_time;
                turnaround_time_sum += turnaround_time;
                waiting_time_sum += waiting_time;
                // Remove shortest_process from ready_queue
                if (shortest_process == ready_queue->front) {
                    dequeue(ready_queue);
                }
                else if (shortest_process == ready_queue->rear) {
                    process* prev = ready_queue->front;
                    while (prev->next != shortest_process) {
                        prev = prev->next;
                    }
                    prev->next = NULL;
                }
                else {
                    process* prev = ready_queue->front;
                    while (prev->next != shortest_process) {
                        prev = prev->next;
                    }
                    prev->next = shortest_process->next;
                }
            }
        }
        else {
            gant_chart[2][cpu_time] = -1; // No process is executing
            cpu_time++;
            cpu_mutex = 0;
            enqueue_by_arrive_time(ready_queue, process_list, cpu_time);

            update_waiting_queue(waiting_queue, ready_queue, cpu_time);
        }
    }

    res->avr_turnaround_time = turnaround_time_sum / count;
    res->avr_waiting_time = waiting_time_sum / count;
}
void non_preemptive_priority(Queue* ready_queue, Queue* waiting_queue, result* res) {
    cpu_time = 0;
    int turnaround_time = 0;
    int waiting_time = 0;
    int turnaround_time_sum = 0;
    int waiting_time_sum = 0;
    int count = PROCESS_COUNT;
    cpu_mutex = 0;

    while (cpu_time < 150) {
        enqueue_by_arrive_time(ready_queue, process_list, cpu_time);
        update_waiting_queue(waiting_queue, ready_queue, cpu_time);

        if ((ready_queue->front != NULL) && cpu_mutex == 0) {
            process* highest_priority_process = ready_queue->front;
            process* current = ready_queue->front;
            while (current != NULL) {
                if (current->Priority < highest_priority_process->Priority) {
                    highest_priority_process = current;
                }
                current = current->next;
            }

            cpu_mutex = 1;
            for (int i = 0; i < highest_priority_process->CPU_burst_time; i++) {
                gant_chart[3][cpu_time] = highest_priority_process->ProcessID;
                cpu_run(highest_priority_process);
                cpu_time++;
                enqueue_by_arrive_time(ready_queue, process_list, cpu_time);
                update_waiting_queue(waiting_queue, ready_queue, cpu_time);

                if (cpu_time == IO_OPERATION_OCCUR) {
                    highest_priority_process->I_O_burst_time = cpu_time + IO_OPERATION_TIME;
                    dequeue(ready_queue);
                    enqueue(waiting_queue, highest_priority_process);
                    break;
                }
                else {
                    highest_priority_process->CPU_bursted_time += 1;
                }
            }

            if (highest_priority_process->CPU_bursted_time >= highest_priority_process->CPU_burst_time) {
                cpu_out(highest_priority_process);
                turnaround_time = cpu_time - highest_priority_process->Arrival_time;
                waiting_time = turnaround_time - highest_priority_process->CPU_burst_time;
                turnaround_time_sum += turnaround_time;
                waiting_time_sum += waiting_time;
                cpu_mutex = 0;
                if (highest_priority_process == ready_queue->front) {
                    dequeue(ready_queue);
                }
                else if ( highest_priority_process == ready_queue->rear) {
                    process* prev = ready_queue->front;
                    while (prev->next != highest_priority_process) {
                        prev = prev->next;
                    }
                    prev->next = NULL;
                }
                else {
                    process* prev = ready_queue->front;
                    while (prev->next != highest_priority_process) {
                        prev = prev->next;
                    }
                    prev->next = highest_priority_process->next;
                }
            }
        }
        else {
            gant_chart[3][cpu_time] = -1;
            cpu_time++;
        }
    }

    res->avr_turnaround_time = turnaround_time_sum / count;
    res->avr_waiting_time = waiting_time_sum / count;
}

void preemptive_priority(Queue* ready_queue, Queue* waiting_queue, result* res) {
    int cpu_time = 0;
    int turnaround_time = 0;
    int waiting_time = 0;
    int turnaround_time_sum = 0;
    int waiting_time_sum = 0;
    int count = PROCESS_COUNT;
    int cpu_mutex = 0;

    while (cpu_time < 150) {
        enqueue_by_arrive_time(ready_queue, process_list, cpu_time);
        update_waiting_queue(waiting_queue, ready_queue, cpu_time);

        if ((ready_queue->front != NULL) && cpu_mutex == 0) {
            process* highest_priority_process = ready_queue->front;
            process* current = ready_queue->front;
            while (current != NULL) {
                if (current->Priority < highest_priority_process->Priority) {
                    highest_priority_process = current;
                }
                current = current->next;
            }

            cpu_mutex = 1;
            while (highest_priority_process->CPU_bursted_time < highest_priority_process->CPU_burst_time) {
                cpu_run(highest_priority_process);
                gant_chart[4][cpu_time] = highest_priority_process->ProcessID;
                cpu_time++;
                enqueue_by_arrive_time(ready_queue, process_list, cpu_time);
                update_waiting_queue(waiting_queue, ready_queue, cpu_time);

                if (cpu_time == IO_OPERATION_OCCUR) {
                    highest_priority_process->I_O_burst_time = cpu_time + IO_OPERATION_TIME;
                    dequeue(ready_queue);
                    enqueue(waiting_queue, highest_priority_process);
                    cpu_mutex = 0;
                    break;
                }
                else {
                    highest_priority_process->CPU_bursted_time += 1;
                }

                // Check for new higher priority process in the ready queue
                process* new_process = ready_queue->front;
                process* current_highest_priority_process = highest_priority_process;
                while (new_process != NULL) {
                    if (new_process->Priority < current_highest_priority_process->Priority) {
                        current_highest_priority_process = new_process;
                    }
                    new_process = new_process->next;
                }

                if (current_highest_priority_process != highest_priority_process) {
                    highest_priority_process = current_highest_priority_process;
                    // Ensure that the loop condition is checked with the new highest priority process
                    if (highest_priority_process->CPU_bursted_time >= highest_priority_process->CPU_burst_time) {
                        break;
                    }
                }


                if (highest_priority_process->CPU_bursted_time >= highest_priority_process->CPU_burst_time) {
                    cpu_out(highest_priority_process);
                    turnaround_time = cpu_time - highest_priority_process->Arrival_time;
                    waiting_time = turnaround_time - highest_priority_process->CPU_burst_time;
                    turnaround_time_sum += turnaround_time;
                    waiting_time_sum += waiting_time;
                    cpu_mutex = 0;
                    if (highest_priority_process == ready_queue->front) {
                        dequeue(ready_queue);
                    }
                    else if (highest_priority_process == ready_queue->rear) {
                        process* prev = ready_queue->front;
                        while (prev->next != highest_priority_process) {
                            prev = prev->next;
                        }
                        prev->next = NULL;
                    }
                    else {
                        process* prev = ready_queue->front;
                        while (prev->next != highest_priority_process) {
                            prev = prev->next;
                        }
                        prev->next = highest_priority_process->next;
                    }
                }
            }
        }
        else {
            gant_chart[4][cpu_time] = -1;
            cpu_time++;
        }
    }

    res->avr_turnaround_time = turnaround_time_sum / count;
    res->avr_waiting_time = waiting_time_sum / count;
}

void round_robin(Queue* ready_queue, Queue* waiting_queue, result* res) {
    
    int time_quantum = 4;
    cpu_time = 0;
    int turnaround_time = 0;
    int waiting_time = 0;
    int turnaround_time_sum = 0;
    int waiting_time_sum = 0;
    int count = PROCESS_COUNT;

    while (cpu_time < 150) {
     
        enqueue_by_arrive_time(ready_queue, process_list, cpu_time);
    
        update_waiting_queue(waiting_queue, ready_queue, cpu_time);

        if (ready_queue->front != NULL) {
            process* execute_process = dequeue(ready_queue);
            int executed_time = 0;

          
            while (executed_time < time_quantum && execute_process->CPU_bursted_time < execute_process->CPU_burst_time) {
                gant_chart[5][cpu_time] = execute_process->ProcessID;
                cpu_run(execute_process);
                cpu_time++;
                executed_time++;
                execute_process->CPU_bursted_time++;

            
                if (execute_process->CPU_bursted_time == IO_OPERATION_OCCUR) {
                    execute_process->I_O_burst_time = cpu_time + IO_OPERATION_TIME;
                    enqueue(waiting_queue, execute_process);
                    break;
                }

             
                enqueue_by_arrive_time(ready_queue, process_list, cpu_time);
            }

          
            if (execute_process->CPU_bursted_time >= execute_process->CPU_burst_time) {
                cpu_out(execute_process);
                turnaround_time = cpu_time - execute_process->Arrival_time;
                waiting_time = turnaround_time - execute_process->CPU_burst_time;
                turnaround_time_sum += turnaround_time;
                waiting_time_sum += waiting_time;
                //free(execute_process);
            }
            else if (executed_time == time_quantum) {
                
                enqueue(ready_queue, execute_process);
            }
        }
        else {
            gant_chart[5][cpu_time] = -1; // No process is executing
            cpu_time++;
        }
    }


    res->avr_turnaround_time = turnaround_time_sum / count;
    res->avr_waiting_time = waiting_time_sum / count;
    
}

void evaluation(result fcfs_result, result non_preemptive_sjf_result, result preemptive_sjf_result, result non_preemptive_priority_result, result preemptive_priority_result, result round_robin_result) {
    schedule_result[0] = fcfs_result;
    schedule_result[1] = non_preemptive_sjf_result;
    schedule_result[2] = preemptive_sjf_result;
    schedule_result[3] = non_preemptive_priority_result;
    schedule_result[4] = preemptive_priority_result;
    schedule_result[5] = round_robin_result;
}

// ť ���� ���� �Լ�
int queue_count(Queue* q) {
    return PROCESS_COUNT;
}

// ť���� ��带 �����Ѵ�
void swap_node(Queue* q, process* a, process* b) {
    if (q == NULL || a == NULL || b == NULL || a == b) return;

    process* prev_a = NULL;
    process* prev_b = NULL;
    process* current = q->front;

    // Find the previous nodes of a and b
    while (current != NULL && (prev_a == NULL || prev_b == NULL)) {
        if (current->next == a) prev_a = current;
        if (current->next == b) prev_b = current;
        current = current->next;
    }

    // If either a or b is not in the list, return
    if (a != q->front && prev_a == NULL) return;
    if (b != q->front && prev_b == NULL) return;

    // Update front if a or b is the front node
    if (q->front == a) q->front = b;
    else if (q->front == b) q->front = a;

    // Update the previous nodes to point to the correct swapped nodes
    if (prev_a != NULL) prev_a->next = b;
    if (prev_b != NULL) prev_b->next = a;

    // Swap the next pointers
    process* temp = a->next;
    a->next = b->next;
    b->next = temp;

    // Ensure rear pointer is correct if we swapped with the last element
    if (a->next == NULL) q->rear = a;
    if (b->next == NULL) q->rear = b;

    // Ensure the last node's next pointer is NULL
    if (q->rear == a) a->next = NULL;
    if (q->rear == b) b->next = NULL;
}

int is_IO_occur(int time) {
    if (time == IO_OPERATION_OCCUR) { // I/O ������ �߻�
        return 1;
    }
    return 0;
}

void update_waiting_queue(Queue* waiting_queue, Queue* ready_queue, int current_time) {
    process* current = waiting_queue->front;
    process* prev = NULL;

    while (current != NULL) {
        // ���� �ð��� ���μ����� IO ����Ʈ �ð��� �ʰ��ϸ� �غ� ť�� �̵�
        if (current->I_O_burst_time <= current_time) {
            if (prev == NULL) {
                dequeue(waiting_queue);
                enqueue(ready_queue, current);
                current = waiting_queue->front;
            }
            else {
                prev->next = current->next;
                enqueue(ready_queue, current);
                current = prev->next;
            }
        }
        else {
            prev = current;
            current = current->next;
        }
    }
}

// ���� ���μ��� ������, �ð��� ���� ������ ���μ����� ť�� �ִ´�
void enqueue_by_arrive_time(Queue* ready_queue, process* process_list[PROCESS_COUNT], int cpu_time) {
    for (int i = 0; i < PROCESS_COUNT; i++) {
        
        if (process_list[i]->Arrival_time == cpu_time) {
            enqueue(ready_queue, process_list[i]);
        }
    }
}

void init() {
    clear_screen();
    
    printf("-----------------------------------------------------------------------------------------------------------------------\n");
    printf("|  CPU Scheduling Simulator                                                                                           |\n");
    printf("|                                                                                                                     |\n");
    printf("|  * %d���� �Է��ϸ� �����층�� ����˴ϴ�.                                                                            |\n",1);
    printf("|                                                                                                                     |\n");
    printf("-----------------------------------------------------------------------------------------------------------------------\n");


    int n;
    do{
        scanf("%d", &n);
    } while (n != 1);
    
}

void clear_screen() {
#ifdef _WIN32
    system("cls");
#elif __linux__
    system("clear");

#endif
}

int result_view() {
    int n;

    while (1) {
        clear_screen();
        printf("-----------------------------------------------------------------------------------------------------------------------\n");
        printf("| �����층 �Ϸ�.                                                                                                       |\n");
        printf("| �ٽ� �����Ϸ��� %d�� �Է�                                                                                             |\n", 0);
        printf("| �����ٷ��� Gantt Chart�� Ȯ���Ϸ���, �ش� �����ٷ��� ��ȣ�� �Է�                                                     |\n");
        printf("| ���μ��� ������ Ȯ���Ϸ��� %d �Է�                                                                                    |\n", 7);
        printf("| ���α׷��� �����Ϸ��� %d �Է�                                                                                        |\n", -1);
        printf("-----------------------------------------------------------------------------------------------------------------------\n");

        printf("\n");
        

        for (int i = 0; i < 6; i++) {
            switch (i)
            {
            case 0:
                printf("%d : fcfs_schedule ", 1);
                break;
            case 1:
                printf("%d : non_preemptive_sjf ", 2);
                break;
            case 2:
                printf("%d : preemptive_sjf ", 3);
                break;
            case 3:
                printf("%d : non_preemptive_priorit ", 4);
                break;
            case 4:
                printf("%d : preemptive_priority ", 5);
                break;
            case 5:
                printf("%d : round_robin ", 6);
                break;
            default:
                break;
            }


            printf("(avr_turnaround_time : %d, avr_wating_time : %d)\n", schedule_result[i].avr_turnaround_time, schedule_result[i].avr_waiting_time);
        }

        printf("\n");


        scanf("%d", &n);
        if (n == 0 || n == -1) {
            break;
        }
        else if (1 <= n && n <= 6) {
            print_gantt_chart(n-1, 150);
        }
        else if (n == 7) {
            show_process();
        }
    }

    if (n == -1) {
        return -1;
    }
    return 0;
   

}

/*
void made_virtual_queue_state() {


    for (int i = 0; i < PROCESS_COUNT; i++) {
        process* temp = process_list[i];
;
        enqueue(ready_queue, temp);
    }

}
*/

void reset_queue(Queue** ready_queue, Queue** waiting_queue) {
    *ready_queue = create_queue();
    *waiting_queue = create_queue();


}

void create_dup_procesess(process* process_list[PROCESS_COUNT], process* dup_process_list[PROCESS_COUNT]) {
    process* temp;
    process* original;

    for (int i = 0; i < PROCESS_COUNT; i++) {
        original = process_list[i];

        // ���μ��� ���� �� ����Ʈ�� ���� �ִ´�
        temp = create_process(original->CPU_burst_time, original->I_O_burst_time, original->Arrival_time, original->Priority);
        
        dup_process_list[i] = temp;
    }

}

void reset_process(process* process_list[PROCESS_COUNT]) {
    process* original;


    for (int i = 0; i < PROCESS_COUNT; i++) {
        original = process_list[i];
        original->CPU_bursted_time = 0;
        original->next = NULL;

    }

}

void print_gantt_chart(int algorithm_index, int time_units) {
    clear_screen();
    const char* algorithm_names[] = {
            "FCFS", "Non-Preemptive SJF", "Preemptive SJF",
            "Non-Preemptive Priority", "Preemptive Priority", "Round Robin"
    };
    printf("Gantt Chart for %s:\n", algorithm_names[algorithm_index]);

    // ��� ������ ����մϴ�.
    printf("Time:  ");
    for (int t = 0; t < time_units; t++) {
        printf("%4d", t);
    }
    printf("\n");

    // ��Ʈ ��Ʈ�� ���μ��� ID�� ����մϴ�.
    printf("PIDs:  ");
    for (int t = 0; t < time_units; t++) {
        int pid = gant_chart[algorithm_index][t];
        if (pid == -1) {
            printf("   -");
        }
        else {
            printf("%4d", pid);
        }
    }
    printf("\n");
    printf("\n");
    printf("\n");
    printf("�ǵ��ư����� %d �Է�", 1);

    int num;
    do {
        scanf("%d", &num);


    } while (num!=1);


}

void cpu_run(process* process) {
    
}

void cpu_out(process* process) {

}

void show_process() {
    clear_screen();
    int n;
    process* temp;
    while (1) {
        printf("-----------------------------------------------------------------------------------------------------------------------\n");
        printf("| <���μ��� ����>                                                                                                      |\n");
        printf("| �ǵ��ư����� %d�� �Է�                                                                                                |\n", 0);  
        printf("|                                                                                                                      |\n");
        printf("-----------------------------------------------------------------------------------------------------------------------\n");

        printf("\n");

        for (int i = 0; i < PROCESS_COUNT; i++) {
            temp = process_list[i];
           
            printf("[PID %d] : ArriveTime %d, CPU BurstTime %d, Priority : %d \n", temp->ProcessID, temp->Arrival_time, temp->CPU_burst_time, temp->Priority);
             
        }

        do {
            scanf("%d", &n);
        } while (n != 0);

        return;



    }


}