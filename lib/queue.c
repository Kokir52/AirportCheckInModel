/*
 * !! DO NOT MODIFY THIS FILE !!
*/

#include "queue.h"

TAILQ_HEAD(tailhead, entry) head;

void queue_init()
{
  TAILQ_INIT(&head);
}

void queue_push_back(WaitingPassenger waiting)
{
  struct entry *elem;
  elem = malloc(sizeof(struct entry));
  if (elem)
    elem->waiting = waiting;

  TAILQ_INSERT_TAIL(&head, elem, entries);
}

WaitingPassenger queue_front()
{
  struct entry *elem = head.tqh_first;
  return elem->waiting;
}

void queue_pop()
{
  struct entry *elem = head.tqh_first;
  TAILQ_REMOVE(&head, head.tqh_first, entries);
  free(elem);
}

bool is_empty()
{
  return head.tqh_first == NULL;
}
