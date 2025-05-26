#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define MAX_PROCESSES 5
#define MAX_IO_REQUESTS 3
#define TIME_QUANTUM 4
#define QUEUE_SIZE 1000

float avg_waiting_time[6];  
float avg_turnaround_time[6];

typedef struct {
    int items[QUEUE_SIZE];
    int front;
    int rear;
} Queue;



int IsEmpty(Queue* q) {
    return q->front > q->rear;
}

void Enqueue(Queue* q, int value) {
    if (q->rear < QUEUE_SIZE - 1) {
        q->rear++;
        q->items[q->rear] = value;
    }
}

int Dequeue(Queue* q) {
    if (!IsEmpty(q)) {
        return q->items[q->front++];
    }
    return -1; 
}

void InitQueue(Queue* q) {
    q->front = 0;
    q->rear = -1;
}
void Config(Queue* ready_queue, Queue* waiting_queue) {
    InitQueue(ready_queue);
    InitQueue(waiting_queue);
}






typedef struct{
    int io_request_time; //해당 process 안에서의 시간
    int io_busrt_time;
} IO;

typedef struct{
    int pid;
    int arrival_time;
    int cpu_burst_time;
    int priority;
    int io_count;
    IO io_events[MAX_IO_REQUESTS];
} Process;


void Create_Processes(Process processes[], int process_num){
    srand(time(NULL));

    for (int i=0;i<process_num; i++){
        processes[i].pid=i;
        processes[i].arrival_time=rand()%10;
        processes[i].cpu_burst_time=rand()%10 +5;
        processes[i].priority=rand()%process_num+1;
        processes[i].io_count=rand()%(MAX_IO_REQUESTS+1);
        
        int used_time [100]={0}; //io 동시 요청 방지
        for (int j=0;j<processes[i].io_count;){
            int tmp= rand()%(processes[i].cpu_burst_time-1)+1;
            if (!used_time[tmp]){
                used_time[tmp]=1;
                processes[i].io_events[j].io_request_time=tmp;
                processes[i].io_events[j].io_busrt_time= rand()%3+1;
                j++;
            }
        }
            // I/O 요청 시점 selection sort 오름차순으로 정리 
        for (int k = 0; k < processes[i].io_count - 1; k++) {
            for (int l = k + 1; l < processes[i].io_count; l++) {
                if (processes[i].io_events[k].io_request_time > processes[i].io_events[l].io_request_time) {
                    IO tmp = processes[i].io_events[k];
                    processes[i].io_events[k] = processes[i].io_events[l];
                    processes[i].io_events[l] = tmp;
                }
            }
        }
    }

    
}

void Print_Processes(Process processes[], int process_num){
    for (int i = 0; i < process_num; i++) {
        printf("PID: %d | Arrive: %d | CPU: %d | Priority: %d | IO: %d\n",
               processes[i].pid,
               processes[i].arrival_time,
               processes[i].cpu_burst_time,
               processes[i].priority,
               processes[i].io_count);
               
        for (int j = 0; j < processes[i].io_count; j++) {
            printf("   I/O at %d for %d units\n",
                   processes[i].io_events[j].io_request_time,
                   processes[i].io_events[j].io_busrt_time);
        }
    }
}

void Evaluation(Process processes[], int completion_time[], int process_num, int algorithm_index) {
    float total_wait = 0, total_turnaround = 0;

    for (int i = 0; i < process_num; i++) {
        int turnaround = completion_time[i] - processes[i].arrival_time;
        int waiting = turnaround - processes[i].cpu_burst_time;

        total_wait += waiting;
        total_turnaround += turnaround;
    }
    avg_waiting_time[algorithm_index] = total_wait / process_num;
    avg_turnaround_time[algorithm_index] = total_turnaround / process_num;

    printf("\nAverage Waiting Time: %.2f\n", avg_waiting_time[algorithm_index]);
    printf("Average Turnaround Time: %.2f\n", avg_turnaround_time[algorithm_index]);
}
void Compare_All() {
    const char* names[6] = {"FCFS", "RR", "SJF", "Priority", "Preemptive_SJF", "Preemptive_Priority"};

    printf("\n< Algorithm Comparison >\n");
    for (int i = 0; i < 6; i++) {
        printf("%-20s | Waiting Time: %5.2f | Turnaround Time: %5.2f\n",
               names[i], avg_waiting_time[i], avg_turnaround_time[i]);
    }
}
void Print_Gantt_Chart(int gantt_chart[], int current_time) {
    printf("\nGantt Chart:\n");
    for (int t = 0; t < current_time; t++) {
        if (gantt_chart[t] == -1) printf("[IDLE] ");
        else printf("[P%d] ", gantt_chart[t]);
    }
    printf("\n");
}

void FCFS(Process processes[], int process_num){
    Queue ready_queue, waiting_queue;
    Config(&ready_queue,&waiting_queue);

    int executed_time[MAX_PROCESSES] = {0};
    int io_index[MAX_PROCESSES] = {0}; //실행시켜야 할 io
    int remaining_io_time[MAX_PROCESSES] = {0}; //남은 io 시간
    int completion_time[MAX_PROCESSES];
    int gantt_chart[1000];
    
    int current_time = 0;
    int completed = 0;
    int running = -1; //running 중인 process의 index

    
    while (completed < process_num){
        for (int i =0; i<process_num; i++){
            if(processes[i].arrival_time==current_time){
                Enqueue(&ready_queue,i);
            }
        }
      
        int waiting_queue_size = waiting_queue.rear - waiting_queue.front + 1;
            for (int i = 0; i < waiting_queue_size; i++) {
                int check_i = Dequeue(&waiting_queue);
                remaining_io_time[check_i]--;
                if (remaining_io_time[check_i] == 0) {
                    Enqueue(&ready_queue, check_i);
                } else {
                    Enqueue(&waiting_queue, check_i);  
                }
            }
      

        if(running==-1 && !IsEmpty(&ready_queue)){
            running=Dequeue(&ready_queue);
        }
        if(running!=-1){
            gantt_chart[current_time]=processes[running].pid;
            executed_time[running]++;
            if ((io_index[running]<processes[running].io_count)&&(executed_time[running]==processes[running].io_events[io_index[running]].io_request_time)){
                remaining_io_time[running]=processes[running].io_events[io_index[running]].io_busrt_time+1;
                Enqueue(&waiting_queue,running);
                io_index[running]++;
                running=-1;
            }
            else if (executed_time[running] == processes[running].cpu_burst_time) {
                completion_time[running] = current_time + 1;
                completed++;
                running = -1;

            }
        }
        else{
            gantt_chart[current_time]=-1;
        }
        current_time++;
    }
    printf("\n< FCFS 결과 >\n");
    Evaluation(processes, completion_time, process_num,0);
    Print_Gantt_Chart(gantt_chart, current_time);

}
void RR(Process processes[], int process_num){
    Queue ready_queue, waiting_queue;
    Config(&ready_queue, &waiting_queue);

    int executed_time[MAX_PROCESSES] = {0};
    int io_index[MAX_PROCESSES] = {0};
    int remaining_io_time[MAX_PROCESSES] = {0};
    int completion_time[MAX_PROCESSES];
    int gantt_chart[1000];
    
    int current_time = 0;
    int completed = 0;
    int running = -1; //running 중인 process의 index

    int quantum_used[MAX_PROCESSES]={0};

    while (completed < process_num){
        for (int i =0; i<process_num; i++){
            if(processes[i].arrival_time==current_time){
                Enqueue(&ready_queue,i);
            }
        }
      
        int waiting_queue_size = waiting_queue.rear - waiting_queue.front + 1;
            for (int i = 0; i < waiting_queue_size; i++) {
                int check_i = Dequeue(&waiting_queue);
                remaining_io_time[check_i]--;
                if (remaining_io_time[check_i] == 0) {
                    Enqueue(&ready_queue, check_i);
                } else {
                    Enqueue(&waiting_queue, check_i);  
                }
            }

        if(running==-1 && !IsEmpty(&ready_queue)){
            running=Dequeue(&ready_queue);
            quantum_used[running]=0;
        }
        if(running!=-1){
            gantt_chart[current_time]=processes[running].pid;
            executed_time[running]++;
            quantum_used[running]++;

            if ((io_index[running]<processes[running].io_count)&&(executed_time[running]==processes[running].io_events[io_index[running]].io_request_time)){
                remaining_io_time[running]=processes[running].io_events[io_index[running]].io_busrt_time+1;
                Enqueue(&waiting_queue, running);
                io_index[running]++;
                running=-1;
            }
            else if (executed_time[running] == processes[running].cpu_burst_time) {
                completion_time[running] = current_time + 1;
                completed++;
                running = -1;
            }
            else if (quantum_used[running]==TIME_QUANTUM){
                Enqueue(&ready_queue, running);
                running=-1;
            }
        }
        else{
            gantt_chart[current_time]=-1;
        }
        current_time++;
    }
    printf("\n< Round Robin (Q=%d) 결과 >\n", TIME_QUANTUM);
    Evaluation(processes, completion_time, process_num,1);
    Print_Gantt_Chart(gantt_chart, current_time);

}
int Dequeue_shortest(Process processes[], Queue* q){
    if (!IsEmpty(q)){
        int min_index=q->front;
        int min_pid=q->items[min_index];
        int min_burst=processes[min_pid].cpu_burst_time;

        for (int i=q->front+1;i<q->rear+1;i++){
            int pid=q->items[i];
            if (processes[pid].cpu_burst_time<min_burst ||
                //같으면 먼저온거
               (processes[pid].cpu_burst_time == min_burst && processes[pid].arrival_time < processes[min_pid].arrival_time)){
                min_index=i;
                min_pid=pid;
                min_burst=processes[pid].cpu_burst_time;
            }
        }
        for (int i=min_index;i<q->rear;i++){
            q->items[i]=q->items[i+1];
        }
        q->rear--;
        return min_pid;
    }
    else return -1;

}

void SJF(Process processes[], int process_num){
    Queue ready_queue, waiting_queue;
    Config(&ready_queue,&waiting_queue);

    int executed_time[MAX_PROCESSES] = {0};
    int io_index[MAX_PROCESSES] = {0};
    int remaining_io_time[MAX_PROCESSES] = {0};
    int completion_time[MAX_PROCESSES];
    int gantt_chart[1000];
    
    int current_time = 0;
    int completed = 0;
    int running = -1; //running 중인 process의 index

    while (completed < process_num){
        for (int i =0; i<process_num; i++){
            if(processes[i].arrival_time==current_time){
                Enqueue(&ready_queue,i);
            }
        }
      
        int waiting_queue_size = waiting_queue.rear - waiting_queue.front + 1;
            for (int i = 0; i < waiting_queue_size; i++) {
                int check_i = Dequeue(&waiting_queue);
                remaining_io_time[check_i]--;
                if (remaining_io_time[check_i] == 0) {
                    Enqueue(&ready_queue, check_i);
                } else {
                    Enqueue(&waiting_queue, check_i);  
                }
            }

        if(running==-1 && !IsEmpty(&ready_queue)){
            running=Dequeue_shortest(processes,&ready_queue);
        }
        if(running!=-1){
            gantt_chart[current_time]=processes[running].pid;
            executed_time[running]++;
            if ((io_index[running]<processes[running].io_count)&&(executed_time[running]==processes[running].io_events[io_index[running]].io_request_time)){
                remaining_io_time[running]=processes[running].io_events[io_index[running]].io_busrt_time+1;
                Enqueue(&waiting_queue, running);
                io_index[running]++;
                running=-1;
            }
            else if (executed_time[running] == processes[running].cpu_burst_time) {
                completion_time[running] = current_time + 1;
                completed++;
                running = -1;
            }
        }
        else{
            gantt_chart[current_time]=-1;
        }
        current_time++;
    }
    printf("\n< SJF (Non-preemptive) 결과 >\n");
    Evaluation(processes, completion_time, process_num,2);
    Print_Gantt_Chart(gantt_chart, current_time);
}
int Dequeue_highest_priority(Process processes[], Queue* q){
    if (!IsEmpty(q)){
        int min_index = q->front;
        int min_pid = q->items[min_index];
        int min_priority = processes[min_pid].priority;

        for (int i = q->front + 1; i <= q->rear; i++) {
            int pid = q->items[i];
            if (processes[pid].priority < min_priority ||
                //같으면 먼저온거
                (processes[pid].priority == min_priority && processes[pid].arrival_time < processes[min_pid].arrival_time)) {
                min_index = i;
                min_pid = pid;
                min_priority = processes[pid].priority;
            }
        }

        for (int i = min_index; i < q->rear; i++) {
            q->items[i] = q->items[i + 1];
        }
        q->rear--;

        return min_pid;
    }
    else return -1;

}
void Priority(Process processes[], int process_num){
    Queue ready_queue, waiting_queue;
    Config(&ready_queue,&waiting_queue);

    int executed_time[MAX_PROCESSES] = {0};
    int io_index[MAX_PROCESSES] = {0};
    int remaining_io_time[MAX_PROCESSES] = {0};
    int completion_time[MAX_PROCESSES];
    int gantt_chart[1000];
    
    int current_time = 0;
    int completed = 0;
    int running = -1; //running 중인 process의 index

    while (completed < process_num){
        for (int i =0; i<process_num; i++){
            if(processes[i].arrival_time==current_time){
                Enqueue(&ready_queue,i);
            }
        }
      
        int waiting_queue_size = waiting_queue.rear - waiting_queue.front + 1;
            for (int i = 0; i < waiting_queue_size; i++) {
                int check_i = Dequeue(&waiting_queue);
                remaining_io_time[check_i]--;
                if (remaining_io_time[check_i] == 0) {
                    Enqueue(&ready_queue, check_i);
                } else {
                    Enqueue(&waiting_queue, check_i);  
                }
            }

        if(running==-1 && !IsEmpty(&ready_queue)){
            running=Dequeue_highest_priority(processes,&ready_queue);
        }
        if(running!=-1){
            gantt_chart[current_time]=processes[running].pid;
            executed_time[running]++;
            if ((io_index[running]<processes[running].io_count)&&(executed_time[running]==processes[running].io_events[io_index[running]].io_request_time)){
                remaining_io_time[running]=processes[running].io_events[io_index[running]].io_busrt_time+1;
                Enqueue(&waiting_queue, running);
                io_index[running]++;
                running=-1;
            }
            else if (executed_time[running] == processes[running].cpu_burst_time) {
                completion_time[running] = current_time + 1;
                completed++;
                running = -1;
            }
        }
        else{
            gantt_chart[current_time]=-1;
        }
        current_time++;
    }
    printf("\n< Priority (Non-preemptive) 결과 >\n");
    Evaluation(processes, completion_time, process_num,3);
    Print_Gantt_Chart(gantt_chart, current_time);
}

int Dequeue_shortest_remaining_time(Process processes[], Queue* q, int executed_time[]){
    if (!IsEmpty(q)){
        int min_index = q->front;
        int min_pid = q->items[min_index];
        int min_remaining = processes[min_pid].cpu_burst_time - executed_time[min_pid];

        for (int i = q->front + 1; i <= q->rear; i++) {
            int pid = q->items[i];
            int remaining = processes[pid].cpu_burst_time - executed_time[pid];

            if (remaining < min_remaining ||
                //같으면 먼저온거
            (remaining == min_remaining && processes[pid].arrival_time < processes[min_pid].arrival_time)) {
                min_index = i;
                min_pid = pid;
                min_remaining = remaining;
            }
        }

        for (int i = min_index; i < q->rear; i++) {
            q->items[i] = q->items[i + 1];
        }
        q->rear--;

        return min_pid;
    }
    else return -1;

}


void Preemptive_SJF(Process processes[], int process_num){
    Queue ready_queue, waiting_queue;
    Config(&ready_queue,&waiting_queue);

    int executed_time[MAX_PROCESSES] = {0};
    int io_index[MAX_PROCESSES] = {0};
    int remaining_io_time[MAX_PROCESSES] = {0};
    int completion_time[MAX_PROCESSES];
    int gantt_chart[1000];
    
    int current_time = 0;
    int completed = 0;
    int running = -1; //running 중인 process의 index

    while (completed < process_num){
        for (int i =0; i<process_num; i++){
            if(processes[i].arrival_time==current_time){
                Enqueue(&ready_queue,i);
            }
        }
      
        int waiting_queue_size = waiting_queue.rear - waiting_queue.front + 1;
            for (int i = 0; i < waiting_queue_size; i++) {
                int check_i = Dequeue(&waiting_queue);
                remaining_io_time[check_i]--;
                if (remaining_io_time[check_i] == 0) {
                    Enqueue(&ready_queue, check_i);
                } else {
                    Enqueue(&waiting_queue, check_i);  
                }
            }

        // Preemption check
        if (!IsEmpty(&ready_queue)) {
            int candidate = Dequeue_shortest_remaining_time(processes, &ready_queue,executed_time);
            if (running == -1 || processes[candidate].cpu_burst_time-executed_time[candidate] < processes[running].cpu_burst_time-executed_time[running]) {
                if (running != -1){
                        Enqueue(&ready_queue, running);
                }
                running = candidate;
            } else {
                Enqueue(&ready_queue, candidate);
            }
        }

        if(running!=-1){
            gantt_chart[current_time]=processes[running].pid;
            executed_time[running]++;
            if ((io_index[running]<processes[running].io_count)&&(executed_time[running]==processes[running].io_events[io_index[running]].io_request_time)){
                remaining_io_time[running]=processes[running].io_events[io_index[running]].io_busrt_time+1;
                Enqueue(&waiting_queue, running);
                io_index[running]++;
                running=-1;
            }
            else if (executed_time[running] == processes[running].cpu_burst_time) {
                completion_time[running] = current_time + 1;
                completed++;
                running = -1;
            }
        }
        else{
            gantt_chart[current_time]=-1;
        }
        current_time++;
    }
    printf("\n< Preemptive_SJF 결과 >\n");
    Evaluation(processes, completion_time, process_num,4);
    Print_Gantt_Chart(gantt_chart, current_time);
}


void Preemptive_Priority(Process processes[], int process_num){
    Queue ready_queue, waiting_queue;
    Config(&ready_queue,&waiting_queue);

    int executed_time[MAX_PROCESSES] = {0};
    int io_index[MAX_PROCESSES] = {0};
    int remaining_io_time[MAX_PROCESSES] = {0};
    int completion_time[MAX_PROCESSES];
    int gantt_chart[1000];
    
    int current_time = 0;
    int completed = 0;
    int running = -1; //running 중인 process의 index

    while (completed < process_num){
        for (int i =0; i<process_num; i++){
            if(processes[i].arrival_time==current_time){
                Enqueue(&ready_queue,i);
            }
        }
      
        int waiting_queue_size = waiting_queue.rear - waiting_queue.front + 1;
            for (int i = 0; i < waiting_queue_size; i++) {
                int check_i = Dequeue(&waiting_queue);
                remaining_io_time[check_i]--;
                if (remaining_io_time[check_i] == 0) {
                    Enqueue(&ready_queue, check_i);
                } else {
                    Enqueue(&waiting_queue, check_i);  
                }
            }

         // Preemption check
        if (!IsEmpty(&ready_queue)) {
            int candidate = Dequeue_highest_priority(processes, &ready_queue);
            if (running == -1 || processes[candidate].priority < processes[running].priority) {
                if (running != -1){
                        Enqueue(&ready_queue, running);
                }
                running = candidate;
            } else {
                Enqueue(&ready_queue, candidate);
            }
        }
        if(running!=-1){
            gantt_chart[current_time]=processes[running].pid;
            executed_time[running]++;
            if ((io_index[running]<processes[running].io_count)&&(executed_time[running]==processes[running].io_events[io_index[running]].io_request_time)){
                remaining_io_time[running]=processes[running].io_events[io_index[running]].io_busrt_time+1;
                Enqueue(&waiting_queue, running);
                io_index[running]++;
                running=-1;
            }
            else if (executed_time[running] == processes[running].cpu_burst_time) {
                completion_time[running] = current_time + 1;
                completed++;
                running = -1;
            }
        }
        else{
            gantt_chart[current_time]=-1;
        }
        current_time++;
    }
    printf("\n< Preemptive_Priority 결과 >\n");
    Evaluation(processes, completion_time, process_num,5);
    Print_Gantt_Chart(gantt_chart, current_time);
}


void Schedule(Process processes[], int process_num, int algrithm_index) {
    if (algrithm_index == 0) {
        FCFS(processes, process_num);
    } else if (algrithm_index == 1) {
        RR(processes, process_num);
    } else if (algrithm_index== 2) {
        SJF(processes, process_num);
    } else if (algrithm_index == 3) {
        Priority(processes, process_num);
    } else if (algrithm_index == 4) {
        Preemptive_SJF(processes, process_num);
    } else if (algrithm_index == 5) {
        Preemptive_Priority(processes, process_num);
    } else {
        printf("Unknown algorithm");
    }
}
int main(){
    Process processes[MAX_PROCESSES];

    Create_Processes(processes,MAX_PROCESSES);
    Print_Processes(processes,MAX_PROCESSES);
    Schedule(processes, MAX_PROCESSES, 0); //FCFS
    Schedule(processes, MAX_PROCESSES, 1); //RR
    Schedule(processes, MAX_PROCESSES, 2); //SJF
    Schedule(processes, MAX_PROCESSES, 3); //PRIORITY
    Schedule(processes, MAX_PROCESSES, 4); //Preemptive_SJF
    Schedule(processes, MAX_PROCESSES, 5); //Preemptive_Priority
    Compare_All();
    
    
    return 0;
}
