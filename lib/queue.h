/*
 * !! DO NOT MODIFY THIS FILE !!
 */

#ifndef A2_QUEUE_H
#define A2_QUEUE_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/queue.h>

#include "../resources.h"

typedef struct
{
  Passenger* passenger;
} WaitingPassenger;

struct entry
{
  WaitingPassenger waiting;
  TAILQ_ENTRY(entry) entries;
};

void queue_init();
void queue_push_back(WaitingPassenger waiting);
void queue_pop();
WaitingPassenger queue_front();
bool is_empty();

#endif //A2_QUEUE_H