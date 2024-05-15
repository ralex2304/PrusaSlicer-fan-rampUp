#ifndef CVECTOR_H_
#define CVECTOR_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CVECTOR_ADD 10000

struct CVector {
	void** data;
	unsigned long size;
	unsigned long capacity;
	size_t elem_size;
};

void cvector_init(CVector* v, size_t elem_size);

void cvector_resize(CVector* v, unsigned long new_cap);

void cvector_push(CVector* v, void* data);

void* cvector_get(CVector* v, unsigned long index);

int cvector_set(CVector* v, unsigned long index, void* data);

int cvector_delete(CVector* v, unsigned long index);

int cvector_pop(CVector* v);

int cvector_insert(CVector* v, unsigned long index, void* data);

#endif
