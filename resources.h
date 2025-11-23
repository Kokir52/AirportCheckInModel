
#ifndef A2_RESOURCES_H
#define A2_RESOURCES_H

#include "pthread.h"
#include "stdint.h"
#include "stdlib.h"
#include "semaphore.h"

// ------------------------------------------------------------------------
// STUDENT TODO:
// * You are allowed to modify this file.
// * Please note that you are only permitted to include synchronization
//   primitives as new variables ( ==> boolean conditions, or any counter variables not allowed)!

typedef enum{
  READY,                // waiting for the next passenger - a passenger can select the self-check-in in this state 
  START,                // transition state suggesting process start
  ID_SCAN,              // passenger can scan the id
  PASSENGER_IDENTIFIED, // transition state suggesting that the passenger has been successfully identified
  OUT_OF_SERVICE,       // self-check-in needs maintenance
  PRINT_TICKET,         // self-check-in completed
  OFF                   // no more passengers, self-check-in-s switched off
} State;

typedef struct {
  pthread_t passenger_tid;
  ssize_t id;
  bool check_in_completed;  // aimed to be used if the passenger goes for the traditional self-check-in
  sem_t check_in_done;
} Passenger;

typedef struct{
  pthread_t self_check_in_tid;
  ssize_t id;
  State state;
  Passenger* passenger;
  sem_t self_check_in_started;
  sem_t self_check_id_request;
  sem_t id_scanned;
  sem_t ticket_printed;
  sem_t ticket_taken;
  sem_t service_finished;
} SelfCheckIn;

typedef struct {
  pthread_t employee_tid;
  ssize_t id;
} Employee;

typedef struct {
  pthread_t maintainer_tid;
  ssize_t id;
} Maintainer;

#endif //A2_RESOURCES_H