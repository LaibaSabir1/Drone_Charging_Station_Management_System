Drone Charging Station Management System

A multithreaded Operating Systems simulation project that demonstrates core OS concepts through a real-world drone charging management scenario. The system manages multiple drones requesting access to limited charging stations while ensuring efficient scheduling, synchronization, and fairness.

## Project Overview

In modern smart delivery systems, drones must periodically recharge before continuing deliveries. Since charging stations are limited resources, efficient scheduling and synchronization mechanisms are required.

This project simulates:

* Multiple drones operating concurrently
* Limited charging stations
* Dynamic charging requests
* Priority-based scheduling
* Resource synchronization
* Starvation prevention

The project was developed as part of the CS342 Operating Systems course at International Islamic University Islamabad (IIUI). 

---

## Objectives

The project demonstrates the implementation of:

* Multithreading
* Producer-Consumer Architecture
* Multi-Level Feedback Queue (MLFQ) Scheduling
* Semaphore Synchronization
* Mutex Synchronization
* Shared Resource Management
* Starvation Prevention Using Aging
* Efficient Resource Allocation

---

## ⚙️ System Architecture

### Main Components

| Component         | Purpose                     |
| ----------------- | --------------------------- |
| Drone Threads     | Simulate independent drones |
| Producer Thread   | Generates charging requests |
| MLFQ Scheduler    | Manages drone priorities    |
| Charging Stations | Shared charging resources   |
| Semaphore         | Controls station access     |
| Mutex             | Protects shared resources   |
| Aging Thread      | Prevents starvation         |

---

##  Multithreading

Each drone is implemented as an independent thread.

Drone lifecycle:

1. Request charging
2. Enter scheduling queue
3. Wait for available station
4. Charge battery
5. Resume delivery

This allows multiple drones to operate concurrently and efficiently.

---

##  Producer-Consumer Model

The project uses the classic Producer-Consumer synchronization model.

### Producer

* Generates delivery requests dynamically

### Consumer

* Drone threads process charging requests

### Workflow


Producer Creates Request
          ↓
Request Enters Queue
          ↓
Scheduler Selects Drone
          ↓
Drone Gets Charging Station
          ↓
Battery Charges
          ↓
Delivery Resumes

---

##  Multi-Level Feedback Queue (MLFQ)

The scheduler uses four priority queues:

| Queue | Drone Type | Priority |
| ----- | ---------- | -------- |
| Q0    | Emergency  | Highest  |
| Q1    | Medical    | High     |
| Q2    | Food       | Medium   |
| Q3    | Parcel     | Low      |

### Charging Time Quantum

| Queue | Charging Time |
| ----- | ------------- |
| Q0    | 5 Seconds     |
| Q1    | 4 Seconds     |
| Q2    | 3 Seconds     |
| Q3    | 2 Seconds     |

Higher-priority drones always receive charging access first.

---

##  Synchronization

### Semaphore

Controls access to limited charging stations.


sem_wait(&charging_stations);

/* Charging Process */

sem_post(&charging_stations);


Only a fixed number of drones can charge simultaneously.

### Mutex Locks

Used to protect:

* Shared queues
* Charging station array
* Console output


pthread_mutex_lock(&queue_mutex);

/* Critical Section */

pthread_mutex_unlock(&queue_mutex);


---

## 🛡 Starvation Prevention (Aging)

To prevent low-priority drones from waiting indefinitely, the system implements Aging.

### Promotion Rules

| From | To | Waiting Time |
| ---- | -- | ------------ |
| Q3   | Q2 | 10 Seconds   |
| Q2   | Q1 | 10 Seconds   |

This ensures fairness and avoids starvation.

---

## 🖥 Technologies Used

* C Programming
* POSIX Threads (pthreads)
* Semaphores
* Mutex Locks
* Linux System Calls
* Operating System Scheduling Concepts

---

##  Project Structure

Drone-Charging-Station-Management-System/
│
├── src/
│   └── drone_charging_system.c
│
├── report/
│   └── Drone_Charging_Station_Report.pdf
│
├── screenshots/
│   ├── output1.png
│   ├── output2.png
│   └── ...
│
└── README.md
----
## 🚀 Compilation and Execution

### Compile

gcc Project.c -o Project -lpthread


### Run

./Project


---

##  Sample Output

[PRODUCER] New request --> Drone #01 (EMERGENCY)

[SYNC] Drone #01 EMERGENCY waiting for station...

[STATION] Drone #01 EMERGENCY --> Station 1 acquired

[CHARGING] Drone #01 EMERGENCY charging for 5s...

[DONE] Drone #01 EMERGENCY --> Station 1 released


---

##  Operating System Concepts Demonstrated

✅ Multithreading

✅ Producer-Consumer Problem

✅ Multi-Level Feedback Queue Scheduling

✅ Semaphores

✅ Mutex Locks

✅ Shared Resource Management

✅ Starvation Prevention

✅ Concurrent Programming

---

##  Team Members

* Laiba Sabir
* Khadeeja Taj
* Najia Zainab
* Manahil Khan


---

## References

* Operating System Concepts — Silberschatz
* POSIX pthread Documentation
* Linux Semaphore Documentation
* GeeksforGeeks Operating System Articles
* MLFQ Scheduling Tutorials
* Multithreading and Synchronization Tutorials

---

##  License

This project was developed for educational purposes as part of the CS342 Operating Systems course at International Islamic University Islamabad (IIUI).
