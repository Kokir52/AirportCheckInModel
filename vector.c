/*
 * !! DO NOT MODIFY THIS FILE !!
 */

#include "vector.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void vector_init(vector* this) {
  this->size_ = VECTOR_INIT_SIZE;
  this->used_ = 0;
  this->data_ = malloc(VECTOR_INIT_SIZE * sizeof(void*));
}

void vector_destroy(vector* this) { free(this->data_); }

vector_iterator vector_begin(vector* this) { return this->data_; }

vector_iterator vector_end(vector* this) { return this->data_ + this->used_; }

void vector_erase(vector* this, vector_iterator it) {
  vector_iterator next = it + 1;
  for (; next < vector_end(this); ++it, ++next)
    *it = *next;
  --this->used_;
}

void vector_push_back(vector* this, void* element) {
  if (this->used_ + 1 == this->size_) {
    this->size_ *= 2;
    void** temp = (void**)realloc(this->data_, this->size_ * sizeof(void**));
    if (temp) {
      this->data_ = temp;
    } else {
      printf("out of memory\n");
      exit(-1);
    }
  }
  this->data_[this->used_++] = element;
}