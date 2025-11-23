/*
 * !! DO NOT MODIFY THIS FILE !!
 * !! BUT - Feel free to play around with parameters for testing purposes + see tesing_defines.h !!
 */

#ifndef A2_DEFINITIONS_H
#define A2_DEFINITIONS_H

#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <string.h>
#include <assert.h>

#include "resources.h"
#include "lib/vector.h"
#include "lib/queue.h"

#include "testing_defines.h"

#define ERROR -1

#define LOWER_LIMIT 1
#define UPPER_LIMIT_PASSENGERS 2000
#define UPPER_LIMIT_EMPLOYEES 1000
#define UPPER_LIMIT_SELFCHECKINS 1000

#define NUMBER_EXPRESS_BAG_DROP_COUNTER 2

vector free_self_check_in_machines;
bool check_in_line_opened = false;
Passenger* baggage_drop_counters[NUMBER_EXPRESS_BAG_DROP_COUNTER];
bool no_more_passengers = false;

// ------------------------------------------------------------------------------------
void checkAllowedRange(ssize_t value, ssize_t upper_limit, char* name)
{
  if (value < LOWER_LIMIT || value > upper_limit)
  {
    fprintf(stderr, "%s allowed range: [%d,%zd].\n", name, LOWER_LIMIT, upper_limit);
    exit(ERROR);
  }
}

// ------------------------------------------------------------------------------------
void handleArguments(int argc, char* argv[], ssize_t* num_passengers, ssize_t* num_employees, ssize_t* num_selfcheckins)
{
  if (argc != 4) 
  {
    fprintf(stderr, "Usage: %s <num_passengers> <num_employees> <num_selfcheckins>\n", argv[0]);
    exit(ERROR);
  }

  // check the validity of parameters
  *num_passengers = atoi(argv[1]);
  *num_employees = atoi(argv[2]);
  *num_selfcheckins = atoi(argv[3]);

  checkAllowedRange(*num_passengers, UPPER_LIMIT_PASSENGERS, "num_passengers");
  checkAllowedRange(*num_employees, UPPER_LIMIT_EMPLOYEES, "num_employees");
  checkAllowedRange(*num_selfcheckins, UPPER_LIMIT_SELFCHECKINS, "num_selfcheckins");
}

// ------------------------------------------------------------------------------------
void createPassengers(Passenger* passengers, ssize_t num_passengers, void* start_routine)
{
  for (ssize_t i = 0; i < num_passengers; i++) 
  {
    passengers[i].id = i;
    assert(!pthread_create(&passengers[i].passenger_tid, NULL, start_routine, (void*)&passengers[i]));
  }
}

// ------------------------------------------------------------------------------------
void createEmployees(Employee* employees, ssize_t num_employees, void* start_routine)
{
  for (ssize_t i = 0; i < num_employees; i++) 
  {
    employees[i].id = i;
    
    if (options.self_check_in == ACTIVE) // completely avoid employee threads creation; self_check_in active means the common check_in won't be the option for any customer
      assert(!pthread_create(&employees[i].employee_tid, NULL, (void*(*)(void*))dummyStartRoutine, NULL));  
    else
      assert(!pthread_create(&employees[i].employee_tid, NULL, start_routine, (void*)&employees[i]));
  }
}

// ------------------------------------------------------------------------------------
void createMaintainers(Maintainer* maintainers, ssize_t num_maintainers, void* start_routine)  
{
  (void)start_routine;
  for (ssize_t i = 0; i < num_maintainers; i++) 
  {
    maintainers[i].id = i;
    if (options.maintenance == INACTIVE) // completely avoid maintainer threads creation, as maintenance never needed
      assert(!pthread_create(&maintainers[i].maintainer_tid, NULL, (void*(*)(void*))dummyStartRoutine, NULL)); 
    else
      assert(!pthread_create(&maintainers[i].maintainer_tid, NULL, start_routine, (void*)&maintainers[i]));
  }
}

// ------------------------------------------------------------------------------------
void createSelfCheckIns(SelfCheckIn** checkins, ssize_t num_checkins)
{
  vector_init(&free_self_check_in_machines);

  for (ssize_t i = 0; i < num_checkins; i++) 
  {
    SelfCheckIn* self_check_in = malloc(sizeof(SelfCheckIn));

    if(!self_check_in)
    {
      free(self_check_in);
      fprintf(stderr, "Could not allocate memory!\n");
      exit(-1);
    }

    self_check_in->id = i;
    self_check_in->state = READY;
    self_check_in->passenger = NULL;
    vector_push_back(&free_self_check_in_machines, self_check_in);
    
    checkins[i] = self_check_in;
  }
}

// ------------------------------------------------------------------------------------
void createSelfCheckMachines(SelfCheckIn** checkins, ssize_t num_checkins, void* start_routine)
{
  for (ssize_t i = 0; i < num_checkins; i++) 
  {
    SelfCheckIn* self_check_in = checkins[i];
    assert(!pthread_create(&self_check_in->self_check_in_tid, NULL, start_routine, (void*)self_check_in));   
  }
}

// ------------------------------------------------------------------------------------
void freeResources(Passenger* passengers, Employee* employees, SelfCheckIn** checkins, Maintainer* maintainers)
{
  vector_iterator it = vector_begin(&free_self_check_in_machines);
  while (it != vector_end(&free_self_check_in_machines)) 
  {
    free(*it);
    vector_erase(&free_self_check_in_machines, it);
  }
  
  vector_destroy(&free_self_check_in_machines);

  free(passengers);
  free(employees);
  free(checkins);
  free(maintainers);
}

// ------------------------------------------------------------------------
void maintenance() 
{
  ssize_t microsec_to_sleep = 1000 + rand() % (20 * 1000);
  nanosleep((const struct timespec[]){{0, 1000L * microsec_to_sleep}}, NULL);
}

// ------------------------------------------------------------------------
void checkIn() 
{
  ssize_t microsec_to_sleep = 1000 + rand() % (20 * 1000);
  nanosleep((const struct timespec[]){{0, 1000L * microsec_to_sleep}}, NULL);
}

// ------------------------------------------------------------------------
void printTicket() 
{
  ssize_t microsec_to_sleep = 1000 + rand() % (20 * 1000);
  nanosleep((const struct timespec[]){{0, 1000L * microsec_to_sleep}}, NULL);
}

// ------------------------------------------------------------------------
void coffeeTime() 
{
  ssize_t microsec_to_sleep = 1000 + rand() % (20 * 1000);
  nanosleep((const struct timespec[]){{0, 1000L * microsec_to_sleep}}, NULL);
}

// ------------------------------------------------------------------------
void dropOffBaggage() 
{
  ssize_t microsec_to_sleep = 1000 + rand() % (20 * 1000);
  nanosleep((const struct timespec[]){{0, 1000L * microsec_to_sleep}}, NULL);
}

// ------------------------------------------------------------------------
void scanID() 
{
  ssize_t microsec_to_sleep = 1000 + rand() % (20 * 1000);
  nanosleep((const struct timespec[]){{0, 1000L * microsec_to_sleep}}, NULL);
}

// ------------------------------------------------------------------------
ssize_t outOfService() 
{
  if (options.maintenance == RANDOM)
    return rand() % 2;
  if (options.maintenance == ACTIVE)
    return 1; 
  
  return 0;  // INACTIVE
}

// ------------------------------------------------------------------------
ssize_t hasBaggage() 
{
  if (options.baggage_drop == RANDOM)
    return rand() % 2;
  if (options.baggage_drop == ACTIVE)
    return 1; 
  
  return 0; // INACTIVE
}

// ------------------------------------------------------------------------
ssize_t useSelfCheckIn() 
{
  if (options.self_check_in == RANDOM)
    return rand() % 2;
  if (options.self_check_in == ACTIVE)
    return 1; 
  
  return 0; // INACTIVE
}

#endif //A2_DEFINITIONS_H