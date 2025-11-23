#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "definitions.h"

// ------------------------------------------------------------------------
// STUDENT TODO: define synchronization primitives here
sem_t passenger_sem;

pthread_mutex_t queue_interaction;

sem_t self_check_ready_for_use;

sem_t find_checkin_maintain;

sem_t passenger_selfcheck_sem;

pthread_mutex_t self_check_in_vector;

pthread_mutex_t open_check_in_line;

sem_t person_added_to_queue;

sem_t baggage_drop_off;

sem_t self_check_at_maintance;

pthread_cond_t check_in_line_opened_;

pthread_mutex_t baggage_check_in;


// ------------------------------------------------------------------------

// ------------------------------------------------------------------------
/* STUDENT TODO:
 * -) Ensure that potential shared resources are properly locked to prevent data races!
 * -) Implement appropriate synchronization strategies to avoid thread starvation!
 * HINT: Initially, ignore the baggage check-in part and focus on getting the rest working correctly. 
 *       Then, integrate the baggage check-in while taking care of both - synchronization and performance.
 *       Consider the following:
 *       - Does it make sense for one passenger to wait until another finishes checking in, 
 *         if there are free spots available?
 *       - What is the maximum number of passengers that can check in simultaneously?
 *       - Should one passenger dropping off baggage prevent another from searching for 
 *         a free baggage check-in counter? 
 */
void checkInBaggage(Passenger* passenger)
{
  sem_wait(&baggage_drop_off);

  pthread_mutex_lock(&baggage_check_in);
  int i = 0;
  while (baggage_drop_counters[i] != NULL) 
  {
    i++;
  }

  baggage_drop_counters[i] = passenger;
  pthread_mutex_unlock(&baggage_check_in);

  dropOffBaggage();
  
  pthread_mutex_lock(&baggage_check_in);
  baggage_drop_counters[i] = NULL;
  pthread_mutex_unlock(&baggage_check_in);
  sem_post(&baggage_drop_off);
  
  
}

// ------------------------------------------------------------------------
/* STUDENT TODO:
 * Ensure that potential shared resources are properly locked to prevent data races!
 */

vector_iterator findFreeSelfCheckIn()  
{
  vector_iterator it = vector_begin(&free_self_check_in_machines);
  vector_iterator it_end = vector_end(&free_self_check_in_machines);

  for (; it != it_end; ++it) 
  {
    SelfCheckIn* self_check_in = (SelfCheckIn*)*it;
    if (self_check_in->state != OUT_OF_SERVICE)
    {
      return it;
    }
  }
  
  return NULL;
}

// ------------------------------------------------------------------------
/* STUDENT TODO:
 * Ensure that potential shared resources are properly locked to prevent data races!
 */
vector_iterator findSelfCheckInToBeMaintained()  
{
  vector_iterator it = vector_begin(&free_self_check_in_machines);
  vector_iterator it_end = vector_end(&free_self_check_in_machines);

  for (; it != it_end; ++it) 
  {
    SelfCheckIn* self_check_in = (SelfCheckIn*)*it;
    if (self_check_in->state == OUT_OF_SERVICE && self_check_in->state != OFF)  
    {
      return it;
    }
  }
  
  return NULL;
}

// ------------------------------------------------------------------------
/* STUDENT TODO:
 * Ensure that shared resources are properly locked to prevent data races!
 * 
 * HINT PART 1: Start by focusing on the staffed check-in process, ignoring self-check-in at first.
 *              Use appropriate synchronization primitives to ensure the following:
 *              -) No passenger should enter the waiting queue before the first employee arrives (check_in_line_opened = true).
 *              -) Employees should be notified as soon as new passengers arrive.
 *              -) No passenger should proceed to the gate before completing the check-in process.
 * 
 * HINT PART 2: After the staffed check-in is functioning correctly, shift focus to the self-check-in process.
 *              Ensure the following with proper synchronization:
 *              -) Every passenger must be able to find an available self-check-in.
 *              -) Ensure that the self-checkin/passenger interaction adheres to the assignment description and follow-up comments.
 *              -) Prevent two passengers from selecting the same self-check-in at the same time.
 *              -) Determine the maximum number of passengers that can self-check in simultaneously.
 *              -) Should a passenger wait for another to finish if a self-check-in is available?
 */
void passenger(Passenger* passenger)  
{
  // 1) Passenger takes decision - "staffed check-in lane"
  sem_init(&passenger->check_in_done, 0, 0);
  if (!useSelfCheckIn())
  {
    printf("[ OK ] Passenger %zd goes to the check-in lane\n", passenger->id);

    // 1a) Wait for the check-in lane to open
    
    pthread_mutex_lock(&open_check_in_line);
    while (!check_in_line_opened)
    {
      pthread_cond_wait(&check_in_line_opened_, &queue_interaction);
    }
    pthread_mutex_unlock(&open_check_in_line);
    
    if(!check_in_line_opened)
    {
      printf("[OOPS] Passenger %zd appears to be early for their check-in! The lane hasn't opened yet!! \n", passenger->id);
    }
    
    // 1b) Wait in the queue
    pthread_mutex_lock(&queue_interaction);
    WaitingPassenger waiting_passenger = {passenger};
    queue_push_back(waiting_passenger);
    pthread_mutex_unlock(&queue_interaction);
    sem_post(&person_added_to_queue);
    

    // 1c) Wait for the check-in process to be completed
    sem_wait(&passenger->check_in_done);
    if(!passenger->check_in_completed)
    {
      printf("[OOPS] Keep calm and wait for the check-in to complete @Customer %zd!!\n", passenger->id);
    }

    // 3) Passenger can go once check-in completed
    printf("[ OK ] Passenger %zd can head to the gate now\n", passenger->id);
    sem_destroy(&passenger->check_in_done);
    return;
  }

  // 2) Passenger takes decision - using a self-check-in machines
  printf("[ OK ] Passenger %zd is looking for an available self-check-in\n", passenger->id);

  // 2a) Find an available self-service kiosk in the self-service lane
  sem_wait(&self_check_ready_for_use); // this semaphore waits for a selfcheckin to post that it is ready
  pthread_mutex_lock(&self_check_in_vector);
  vector_iterator self_checkin_it = findFreeSelfCheckIn();
  SelfCheckIn* self_check_in = (SelfCheckIn*)*self_checkin_it;
 
  if(self_check_in->passenger)
  {
    printf("[OOPS] Oh, someone is excited before the yourney! Please, wait for your turn @Customer %zd!\n", passenger->id);
  }

  self_check_in->passenger = passenger;
  
  
  vector_erase(&free_self_check_in_machines, self_checkin_it);
  pthread_mutex_unlock(&self_check_in_vector);
  
  // 2b) Initiate the interaction with the self-check-in (state = START)
  assert(self_check_in->state == READY && "Ohh, something has gone haywire...?!"); // consider using asserts for debugging ;)
  self_check_in->state = START;
  sem_post(&self_check_in->self_check_in_started); //self_check_in_started

  // 2c) Scan the id as soon as the instruction appears on the screen (state = ID_SCAN)
  sem_wait(&self_check_in->self_check_id_request); //selfcheckin asks for id
  if(self_check_in->state == ID_SCAN)
  {
    printf("[ OK ] Passenger %zd can scan the id now.\n", passenger->id);

    scanID();
  
    // 2d) Identification completed (state = PASSENGER_IDENTIFIED)
    self_check_in->state = PASSENGER_IDENTIFIED;
  }

  sem_post(&self_check_in->id_scanned); //passenger scans id

  
  // 2e) Wait for the approvement  
  assert(self_check_in->passenger == passenger);

  sem_wait(&self_check_in->ticket_printed); //ticket_printed

  if(self_check_in->state != PRINT_TICKET)  
  {
    printf("[OOPS] Hey, passenger %zd! Where do you think you're going without a ticket?\n", passenger->id);
  }
  sem_post(&self_check_in->ticket_taken);

  printf("[ OK ] Passenger %zd has completed check-in.\n", passenger->id);

  // 2f) drop the baggage at the express line if needed
  if (hasBaggage())
  {
    printf("[ OK ] Passenger %zd must drop the baggage off at the express line.\n", passenger->id);
    checkInBaggage(passenger);
  }

  printf("[ OK ] Passenger %zd can proceed to the gate now\n", passenger->id);
  sem_destroy(&passenger->check_in_done);
}

// ------------------------------------------------------------------------
/* STUDENT TODO:
 * -) Ensure shared resources are properly locked to prevent data races!
 * -) Notify waiting passengers as soon as the first employee starts working!
 * -) Use appropriate synchronization mechanisms to ensure that an employee acts only with a reason - either there is a waiting passenger,
 *    or there are no more passengers and the employee can go home. (fee free to use is_empty() as the condition if needed)
 * -) Notify the passenger once the check-in procedure is completed!
 * -) Ensure your locking approach does not introduce issues during cancellation!
 */

void employee(Employee* employee)
{

  pthread_mutex_lock(&open_check_in_line);
  if (!check_in_line_opened)
  {
    printf("[ OK ] Employee %zd opens the check-in lane\n", employee->id);
    assert(!check_in_line_opened);
    check_in_line_opened = true;
    pthread_cond_broadcast(&check_in_line_opened_);
  }
  pthread_mutex_unlock(&open_check_in_line);
  

  
  while(true)
  {
    // 1) check-in the next passenger in the waiting line or go home if no more passengers
    sem_wait(&person_added_to_queue);
    if(no_more_passengers)
    {
      printf("[ OK ] Employee %zd: Yeey, let me fly away now :D!!\n", employee->id);
      break;
    }

    
    pthread_mutex_lock(&queue_interaction);
    WaitingPassenger waiting_passenger = queue_front();
    Passenger* passenger = waiting_passenger.passenger;
    queue_pop();
    pthread_mutex_unlock(&queue_interaction);
    

    printf("[ OK ] Employee %zd is about to check-in the passenger %zd \n", employee->id, passenger->id);

    checkIn();
    
    // 2) inform passenger once proceeded and continue
    passenger->check_in_completed = true;
    sem_post(&passenger->check_in_done);

  }
}

// ------------------------------------------------------------------------
/* STUDENT TODO:
 * -) Ensure shared resources are properly locked to prevent data races!
 * -) Ensure that your locking approach does not introduce any problems upon cancellation!
 */
void maintainer(Maintainer* maintainer)
{
  while (1) 
  {
    // 1) Check if any of the self-check-in machines needs maintenance in the self-rythmus (no notification needed)
    pthread_mutex_lock(&self_check_in_vector);
    vector_iterator self_checkin_it = findSelfCheckInToBeMaintained();
       

    // 2) Perform maintenance on an out-of-service self-check-in
    if (self_checkin_it)
    {
      SelfCheckIn* self_check_in = (SelfCheckIn*)*self_checkin_it;
      
      vector_erase(&free_self_check_in_machines, self_checkin_it);
      pthread_mutex_unlock(&self_check_in_vector);
      printf("[ OK ] Maintainer %zd is performing maintenance on self-service %zd.\n", maintainer->id, self_check_in->id);
    
      maintenance();
      
      // 2a) After maintenance the self-check-in is again ready for the next passenger
      self_check_in->state = READY;
    
      // 2b) Ensure the self-check-in is again available for active usage
      pthread_mutex_lock(&self_check_in_vector);
      vector_push_back(&free_self_check_in_machines, self_check_in);
      pthread_mutex_unlock(&self_check_in_vector);
      sem_post(&self_check_in->service_finished);
      continue;
    }
    pthread_mutex_unlock(&self_check_in_vector); 
    printf("[ OK ] Maintainer %zd has nothing to do...\n", maintainer->id);
    coffeeTime();
    continue;
  }
}

// ------------------------------------------------------------------------
/* STUDENT TODO:
 * - Ensure shared resources are properly locked to prevent data races.
 * - Verify that the self-check/passenger interaction adheres to the assignment description and follow-up comments.
 * - Explore an appropriate termination condition to allow threads to exit cleanly.
 * - Ensure your locking approach does not cause issues with the state logic.
 */

void selfCheckIn(SelfCheckIn* self_check_in) 
{
  sem_init(&self_check_in->self_check_in_started, 0, 0);
  sem_init(&self_check_in->self_check_id_request, 0, 0);
  sem_init(&self_check_in->id_scanned, 0, 0);
  sem_init(&self_check_in->ticket_printed, 0, 0);
  sem_init(&self_check_in->ticket_taken, 0, 0);
  sem_init(&self_check_in->service_finished, 0, 0);
  while(true)
  {
    printf("[ OK ] Self-Check-In %zd: Welcome!! Please press Start!!\n", self_check_in->id);
    sem_post(&self_check_ready_for_use); // this signals that the check in is ready

    // 1) Wait for the start action (==> passenger assignment + state = START or ==> closing - state = OFF)
    
    sem_wait(&self_check_in->self_check_in_started); //self_check_in_started

    if(self_check_in->state == OFF) 
    {
      break;
    }

    
  
    if(self_check_in->state != START)
    {
      printf("[OOPS] Self-Check-In %zd may need maintenance! Proceeding without new passenger! \n", self_check_in->id);
    }

    // 2) Switch to the ID_SCAN state and allow the passenger to proceed

    
    printf("[ OK ] Self-Checkout %zd: Please scan your pass/ID according to the instructions on the screen!\n", self_check_in->id);
    self_check_in->state = ID_SCAN;

    sem_post(&self_check_in->self_check_id_request); //selfcheckin asks for id
    // 3) Wait for the passenger to scan the pass and proceed
    sem_wait(&self_check_in->id_scanned); //passenger scans id
    if(self_check_in->state != PASSENGER_IDENTIFIED)
    {
      printf("[OOPS] Self-Checkout %zd: The passenger may be experiencing issues while scanning their ID...\n", self_check_in->id);    
    }

    printTicket();

    printf("[ OK ] Self-Service %zd: Thanks for flying with us! Your ticket is being printed! Please, drop out your baggage at our express line\n", self_check_in->id);
    self_check_in->state = PRINT_TICKET;

    sem_post(&self_check_in->ticket_printed); // ticket_printed

    // 4) Switch the state = READY <<-- can deal with a new passenger
    sem_wait(&self_check_in->ticket_taken);
    self_check_in->passenger = NULL;
    self_check_in->state = READY;
    pthread_mutex_lock(&self_check_in_vector);
    vector_push_back(&free_self_check_in_machines, self_check_in);
    

 
    // 6) Unfortunately, some self-checki-in machines may be out of service from time to time  !! watch out here !!
    if(outOfService())  
    {
      self_check_in->state = OUT_OF_SERVICE;
      pthread_mutex_unlock(&self_check_in_vector); 
      sem_post(&self_check_at_maintance);
      printf("[ OK ] Self-Service %zd: I will be back soon! Thanks for your patiance!! \n", self_check_in->id);
      sem_wait(&self_check_in->service_finished);
      continue;
    }
    pthread_mutex_unlock(&self_check_in_vector);  
  }
  sem_destroy(&self_check_in->self_check_in_started);
  sem_destroy(&self_check_in->self_check_id_request);
  sem_destroy(&self_check_in->id_scanned);
  sem_destroy(&self_check_in->ticket_printed);
  sem_destroy(&self_check_in->ticket_taken);
  sem_destroy(&self_check_in->service_finished);
}

// ------------------------------------------------------------------------
/* STUDENT TODO:
 * Ensure shared resources are properly locked to prevent data races.
 * Initialize and destroy mutexes, condition variables, and semaphores as needed.
 * --> You may do this in other parts of the codebase as well, but ensure synchronization primitives 
 *     are initialized before their first use!
 */

int main(int argc, char* argv[]) 
{
  srand(time(NULL));
  
  // ~ arguments
  ssize_t num_passengers;
  ssize_t num_employees;
  ssize_t num_self_checkins;
  handleArguments(argc, argv, &num_passengers, &num_employees, &num_self_checkins);
  
  // ~ create the threads cast 
  Passenger* passengers = malloc(num_passengers * sizeof(Passenger));
  Maintainer* maintainers = malloc(num_employees * sizeof(Maintainer));
  SelfCheckIn** self_chekins = malloc(num_self_checkins * sizeof(SelfCheckIn*));
  Employee* employees = malloc(num_employees * sizeof(Employee));

  if (!passengers || !employees || !self_chekins || !maintainers) 
  {
    goto error;
  }
  createSelfCheckIns(self_chekins, num_self_checkins);

  /*----Synch initialization--------*/

  pthread_mutex_init(&queue_interaction, NULL);
  sem_init(&self_check_ready_for_use, 0, 0);

  sem_init(&find_checkin_maintain, 0, 1);

  sem_init(&passenger_selfcheck_sem, 0, 0);

  pthread_mutex_init(&self_check_in_vector, NULL);

  pthread_mutex_init(&open_check_in_line, NULL);
  pthread_cond_init(&check_in_line_opened_, NULL);

  sem_init(&person_added_to_queue, 0, 0);

  sem_init(&baggage_drop_off, 0, NUMBER_EXPRESS_BAG_DROP_COUNTER);

  sem_init(&self_check_at_maintance, 0, 0);

  pthread_mutex_init(&baggage_check_in, NULL);

  /*--------------------------------*/
  
  queue_init();
  createEmployees(employees, num_employees, (void*(*)(void*))employee);
  createSelfCheckMachines(self_chekins, num_self_checkins, (void*(*)(void*))selfCheckIn);
  createPassengers(passengers, num_passengers, (void*(*)(void*))passenger);
  createMaintainers(maintainers, num_employees, (void*(*)(void*))maintainer);
  
  // ~ join passangers  
  for (ssize_t i = 0; i < num_passengers; i++) 
  {
    pthread_join(passengers[i].passenger_tid, NULL);
  }

  no_more_passengers = true;  // !! potential breaking condition for employees !!

  // ~ cancel and join maintainers  

  for (ssize_t i = 0; i < num_employees; i++) 
  {
    pthread_cancel(maintainers[i].maintainer_tid);  
    pthread_join(maintainers[i].maintainer_tid, NULL);
    sem_post(&person_added_to_queue);
  }
  
  // join employees
  
  for (ssize_t i = 0; i < num_employees; i++) 
  {
    pthread_join(employees[i].employee_tid, NULL);
  }

  // ~ finally join the self-check-in machines, !! take care of the breaking condition !!
  for (ssize_t i = 0; i < num_self_checkins; i++) 
  {
    self_chekins[i]->state = OFF;
    sem_post(&self_chekins[i]->self_check_in_started);
    sem_post(&self_chekins[i]->service_finished);
    sem_post(&self_chekins[i]->service_finished);
    pthread_join(self_chekins[i]->self_check_in_tid, NULL);
  }

  freeResources(passengers, employees, self_chekins, maintainers); 

  /*---------Synch destroy----------*/

  pthread_mutex_destroy(&queue_interaction);
  sem_destroy(&self_check_ready_for_use);

  sem_destroy(&find_checkin_maintain);

  sem_destroy(&passenger_selfcheck_sem);

  pthread_mutex_destroy(&self_check_in_vector);

  pthread_mutex_destroy(&open_check_in_line);
  pthread_cond_destroy(&check_in_line_opened_);

  sem_destroy(&person_added_to_queue);

  sem_destroy(&baggage_drop_off);

  sem_destroy(&self_check_at_maintance);

  pthread_mutex_destroy(&baggage_check_in);

  /*--------------------------------*/

  printf("[ OK ] !! T H A T ' S    A L L    F O L K S !!\n");
  printf("[ OK ] !!        B O N  V O Y A G E         !!\n");

  return 0;

  error:
    free(passengers);
    free(employees);
    free(self_chekins);
    free(maintainers);
    fprintf(stderr, "Could not allocate memory!\n");
    exit(ERROR);
}