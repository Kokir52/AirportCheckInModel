/*
 * !! DO NOT MODIFY THIS FILE !!
 */

#ifndef VECTOR_H
#define VECTOR_H

#define VECTOR_INIT_SIZE 1000

typedef struct {
  void** data_;
  unsigned int size_;
  unsigned int used_;
} vector;

typedef void** vector_iterator;

void vector_init(vector* this);
void vector_destroy(vector* this);

vector_iterator vector_begin(vector* this);
vector_iterator vector_end(vector* this);

void vector_push_back(vector* this, void* element);
void vector_erase(vector* this, vector_iterator it);

#endif // VECTOR_H