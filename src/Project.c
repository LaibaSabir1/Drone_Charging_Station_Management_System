#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#define NUM_DRONES      8
#define NUM_STATIONS    3
#define NUM_QUEUES      4
#define MAX_DELIVERIES  3
#define QUEUE_SIZE      20
 
//DRONE STRUCT 
typedef struct {
    int  id;
    char type[20];
    int  priority;
    int  wait_time;
    int  queue_level;
} Drone;
 //GLOBAL VARIABLES 
Drone q0[QUEUE_SIZE];
Drone q1[QUEUE_SIZE];
Drone q2[QUEUE_SIZE];
Drone q3[QUEUE_SIZE];
int q0_count = 0, q1_count = 0, q2_count = 0, q3_count = 0;
 
pthread_mutex_t queue_mutex;
pthread_mutex_t station_mutex;
pthread_mutex_t print_mutex;
sem_t  charging_stations;
 
int stations[NUM_STATIONS] = {0};
volatile int simulation_running = 1;
volatile int drones_done = 0;
pthread_mutex_t done_mutex = PTHREAD_MUTEX_INITIALIZER;
//PRINT QUEUE STATE
void print_queue_state() {
    printf("  [QUEUES]  Q0-Emergency:%d  Q1-Medical:%d  Q2-Food:%d  Q3-Parcel:%d\n",q0_count, q1_count, q2_count, q3_count);
}
//ENQUEUE DRONE
void enqueue_drone(Drone d) {
    pthread_mutex_lock(&queue_mutex);
    if(d.priority == 1) { 
    q0[q0_count++] = d; 
    }
    else if (d.priority == 2) { 
    q1[q1_count++] = d; 
    }
    else if (d.priority == 3) { 
    q2[q2_count++] = d; 
    }
    else {
     q3[q3_count++] = d; 
     }
 
    printf("  [ENQUEUE] Drone #%02d %-9s --> Q%d\n",d.id, d.type, d.priority - 1);
    print_queue_state();
    pthread_mutex_unlock(&queue_mutex);
}
//SCHEDULER
Drone schedule_next_drone() {
    Drone selected;
    selected.id = -1;
    pthread_mutex_lock(&queue_mutex);
    if (q0_count > 0) {
        selected = q0[0];
        for (int i = 0; i < q0_count - 1; i++) 
        q0[i] = q0[i+1];
        q0_count--;
    } else if (q1_count > 0) {
        selected = q1[0];
        for (int i = 0; i < q1_count - 1; i++) 
        q1[i] = q1[i+1];
        q1_count--;
    } else if (q2_count > 0) {
        selected = q2[0];
        for (int i = 0; i < q2_count - 1; i++) 
        q2[i] = q2[i+1];
        q2_count--;
    } else if (q3_count > 0) {
        selected = q3[0];
        for (int i = 0; i < q3_count - 1; i++) 
        q3[i] = q3[i+1];
        q3_count--;
    }
    pthread_mutex_unlock(&queue_mutex);
    return selected;
}
 
//RODUCER THREAD 
void* producer_thread(void* arg) {
    (void)arg;
    srand(time(NULL));
    int request_id = 1;
    while (simulation_running) {
        sleep(rand() % 3 + 1);
        if (!simulation_running) 
           break;
        Drone d;
        d.id = request_id++;
        d.wait_time= 0;
        d.queue_level= 0;
 
        int t = rand() % 4;   
        if(t == 0) { 
        strcpy(d.type, "EMERGENCY"); 
        d.priority = 1; 
        }
        else if (t == 1) { 
        strcpy(d.type, "MEDICAL");   
        d.priority = 2; 
        }
        else if (t == 2) { 
        strcpy(d.type, "FOOD");    
        d.priority = 3;
        }
        else{
        strcpy(d.type, "PARCEL");    
        d.priority = 4; 
        }
        pthread_mutex_lock(&print_mutex);
        printf("  ----------------------------------------------------\n");  
        printf("  [PRODUCER] New request --> Drone #%02d (%s)\n", d.id, d.type);
        enqueue_drone(d);
        pthread_mutex_unlock(&print_mutex);
    }
    pthread_mutex_lock(&print_mutex);
    printf("  ----------------------------------------------------\n");  
    printf("  [PRODUCER] Shutting down\n");
    pthread_mutex_unlock(&print_mutex);
    return NULL;
}
//DRONE THREAD 
void* drone_thread(void* arg) {
    Drone* drone = (Drone*)arg;
 
    pthread_mutex_lock(&print_mutex);
    printf("  [THREAD]   Drone #%02d started\n", drone->id);
    pthread_mutex_unlock(&print_mutex);
 
    for (int cycle = 0; cycle < MAX_DELIVERIES; cycle++) {
        //wait for a request 
        Drone current;
        current.id = -1;
        while (current.id == -1 && simulation_running) {
            current = schedule_next_drone();
            if (current.id == -1) 
            sleep(1);
        }
        if (!simulation_running) 
        break;
        //waiting for station
        pthread_mutex_lock(&print_mutex);
        printf("  ----------------------------------------------------\n");  
        printf("  [SYNC]     Drone #%02d %-9s waiting for station...\n",current.id, current.type);
        pthread_mutex_unlock(&print_mutex);
        sem_wait(&charging_stations);
 
        //acquire a free station 
        int station_id = -1;
        pthread_mutex_lock(&station_mutex);
        for (int i = 0; i < NUM_STATIONS; i++) {
            if (stations[i] == 0) {
                stations[i] = current.id;
                station_id  = i;
                break;
            }
        }
        pthread_mutex_unlock(&station_mutex);
        //charging 
        int charge_time;
        if (current.priority == 1) 
        charge_time = 5;
        else if (current.priority == 2) 
        charge_time = 4;
        else if (current.priority == 3) 
      charge_time = 3;
        else                            
        charge_time = 2;
        pthread_mutex_lock(&print_mutex);
        printf("  [STATION]  Drone #%02d %-9s --> Station %d acquired\n",current.id, current.type, station_id + 1);
        printf("  [CHARGING] Drone #%02d %-9s charging for %ds...\n",current.id, current.type, charge_time);
        pthread_mutex_unlock(&print_mutex);
 
        sleep(charge_time);
 
        //release station 
        pthread_mutex_lock(&station_mutex);
        stations[station_id] = 0;
        pthread_mutex_unlock(&station_mutex);
 
        sem_post(&charging_stations);
        pthread_mutex_lock(&print_mutex);
        printf("  [DONE]     Drone #%02d %-9s --> Station %d released | delivery resumed\n",current.id, current.type, station_id + 1);
        pthread_mutex_unlock(&print_mutex);
        sleep(1);
    } 
    pthread_mutex_lock(&print_mutex);
    printf("  ----------------------------------------------------\n");  
    printf("  [COMPLETE] Drone #%02d finished all %d deliveries\n",drone->id, MAX_DELIVERIES);
    pthread_mutex_unlock(&print_mutex);
 
    pthread_mutex_lock(&done_mutex);
    drones_done++;
    if (drones_done == NUM_DRONES) {
        simulation_running = 0;
        pthread_mutex_lock(&print_mutex);
        printf("  =======================================================\n");  
        printf("  [SYSTEM]   All %d drones finished. Shutting down...\n", NUM_DRONES);
        printf("  =======================================================\n"); 
        pthread_mutex_unlock(&print_mutex);
    }
    pthread_mutex_unlock(&done_mutex);
    return NULL;
}
//AGING THREAD
void* aging_thread(void* arg) {
    (void)arg;
    while (simulation_running) {
        sleep(5);
        if (!simulation_running) 
        break;
        pthread_mutex_lock(&queue_mutex);
        /* Q3 -> Q2 */
        for (int i = 0; i < q3_count; i++) {
            q3[i].wait_time += 5;
            if (q3[i].wait_time >= 10) {
                pthread_mutex_lock(&print_mutex);
                printf("  --------------------------------------------------------------\n"); 
                printf("  [AGING]    Drone #%02d waited %ds in Q3 --> promoted to Q2\n",q3[i].id, q3[i].wait_time);
                pthread_mutex_unlock(&print_mutex);
                q3[i].wait_time = 0;
                q2[q2_count++] = q3[i];
                for (int j = i; j < q3_count - 1; j++) 
                q3[j] = q3[j+1];
                q3_count--;
                i--;
            }
        }
        /* Q2 -> Q1 */
        for (int i = 0; i < q2_count; i++) {
            q2[i].wait_time += 5;
            if (q2[i].wait_time >= 10) {
                pthread_mutex_lock(&print_mutex);
                printf("  --------------------------------------------------------------\n"); 
                printf("  [AGING]    Drone #%02d waited %ds in Q2 --> promoted to Q1\n",q2[i].id, q2[i].wait_time);
                pthread_mutex_unlock(&print_mutex);
                q2[i].wait_time = 0;
                q1[q1_count++] = q2[i];
                for (int j = i; j < q2_count - 1; j++) 
                q2[j] = q2[j+1];
                q2_count--;
                i--;
            }
        }
        pthread_mutex_unlock(&queue_mutex);
    }
    return NULL;
}
 //MAIN
int main() {
    pthread_mutex_init(&print_mutex,NULL);
    pthread_mutex_init(&queue_mutex,NULL);
    pthread_mutex_init(&station_mutex,NULL);
    sem_init(&charging_stations, 0, NUM_STATIONS);
    printf("================================================\n");  
    printf("      DRONE CHARGING STATION MANAGEMENT SYSTEM\n"); 
    printf("  \nDrones: %d  |  Stations: %d  |  Deliveries per drone: %d\n",NUM_DRONES, NUM_STATIONS, MAX_DELIVERIES);
    printf("  Queues: Q0=Emergency  Q1=Medical  Q2=Food  Q3=Parcel\n");
    printf("  Charge: Q0=5s  Q1=4s  Q2=3s  Q3=2s\n");
    printf("  Aging:  Q3->Q2 after 10s  |  Q2->Q1 after 10s\n");
    printf("  ================================================\n");  
    printf("\n");
 
    Drone drones[NUM_DRONES];
    for (int i = 0; i < NUM_DRONES; i++) {
        drones[i].id = i + 1;
        drones[i].wait_time= 0;
        drones[i].queue_level = 0;
    }
    pthread_t producer, aging, drone_threads[NUM_DRONES];
    pthread_create(&producer, NULL, producer_thread, NULL);
    pthread_create(&aging,NULL, aging_thread,NULL);
    for (int i = 0; i < NUM_DRONES; i++)
        pthread_create(&drone_threads[i], NULL, drone_thread, &drones[i]);
    for (int i = 0; i < NUM_DRONES; i++)
        pthread_join(drone_threads[i], NULL);
 
    pthread_join(producer, NULL);
    pthread_join(aging,NULL);

    sem_destroy(&charging_stations);
    pthread_mutex_destroy(&queue_mutex);
    pthread_mutex_destroy(&station_mutex);
    pthread_mutex_destroy(&print_mutex);
    pthread_mutex_destroy(&done_mutex);
    printf("  ================================================\n");  
    printf("               SIMULATION COMPLETE\n");
    printf("  ================================================\n");  
    return 0;
}
