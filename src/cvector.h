#ifndef _CVECTOR_H
#define _CVECTOR_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "errors.h"

#define CVECTOR_ADD 10000

typedef struct {
  void** data;
  unsigned long size;
  unsigned long capacity;
  size_t elem_size;
} cvector;

void cvector_init(cvector* v, size_t elem_size);
void cvector_resize(cvector* v, unsigned long new_cap);
void cvector_push(cvector* v, void* data);
void* cvector_get(cvector* v, long index);
int cvector_set(cvector* v, long index, void* data);
int cvector_delete(cvector* v, long index);
int cvector_pop(cvector* v);
int cvector_insert(cvector* v, long index, void* data);

void cvector_init(cvector* v, size_t elem_size) {
  v->size = 0;
  v->capacity = 0;
  v->elem_size = elem_size;
  v->data = (void **) malloc(CVECTOR_ADD * sizeof(void*));
  if (v->data == NULL)
    throw_error(ERR_ALLOC);
  return;
}

void cvector_resize(cvector* v, unsigned long new_cap) {
  v->capacity = new_cap;
  v->data = (void**)realloc(v->data, new_cap * sizeof(void*));
  if (v->data == NULL)
    throw_error(ERR_ALLOC);
  return;
}

void cvector_push(cvector* v, void* data) {
  if (v->size >= v->capacity)
    cvector_resize(v, v->capacity + CVECTOR_ADD);
  v->data[v->size] = malloc(v->elem_size);
  if (v->data[v->size] == NULL)
    throw_error(ERR_ALLOC);
  memcpy_s(v->data[v->size], v->elem_size, data, v->elem_size);
  v->size++;
  return;
}

void* cvector_get(cvector* v, long index) {
  if (index > v->size - 1 || v->size == 0)
    return NULL;
  return v->data[index];
}

int cvector_set(cvector* v, long index, void* data) {
  if (index < 0 || index > v->size - 1)
    return 0;
  v->data[index] = malloc(v->elem_size);
  if (v->data[index] == NULL)
    throw_error(ERR_ALLOC);
  memcpy_s(v->data[index], v->elem_size, data, v->elem_size);
  return 1;
}

int cvector_delete(cvector* v, long index) {
  if (index > v->size - 1 || v->size == 0)
    return 0;
  if (cvector_get(v, index) != NULL)
    free(cvector_get(v, index));
  for (long i = index; i < v->size-1; i++)
    v->data[i] = v->data[i+1];
  v->size--;
  return 1;
}

int cvector_pop(cvector* v) {
  return cvector_delete(v, v->size-1);
}

int cvector_insert(cvector* v, long index, void* data) {
  if (index < 0 || index > v->size)
    return 0;
  if (v->size >= v->capacity)
    cvector_resize(v, v->capacity + CVECTOR_ADD);
  v->size++;
  for (long i = v->size-2; i >= index; i--) {
    v->data[i+1] = v->data[i];
  }
  cvector_set(v, index, data);
  return 1;
}
#endif
