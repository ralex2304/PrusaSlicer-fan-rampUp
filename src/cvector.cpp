#include "cvector.h"

#include <assert.h>

#include "errors.h"

void cvector_init(CVector* v, size_t elem_size) {
	assert(v);
	assert(elem_size > 0);

	v->size = 0;
	v->capacity = 0;
	v->elem_size = elem_size;
	v->data = (void**)calloc(CVECTOR_ADD, sizeof(void*));

	if (v->data == nullptr)
		throw_error(ERR_ALLOC, nullptr);

	return;
}

void cvector_resize(CVector* v, unsigned long new_cap) {

	v->capacity = new_cap;
	v->data = (void**)realloc(v->data, new_cap * sizeof(void*));

	if (v->data == nullptr)
		throw_error(ERR_ALLOC, nullptr);

	return;
}

void cvector_push(CVector* v, void* data) {
	if (v->size >= v->capacity)
		cvector_resize(v, v->capacity + CVECTOR_ADD);

	v->data[v->size] = calloc(1, v->elem_size);

	if (v->data[v->size] == NULL)
		throw_error(ERR_ALLOC, nullptr);

	memcpy(v->data[v->size], data, v->elem_size);
	v->size++;
	return;
}

void* cvector_get(CVector* v, unsigned long index) {
	if (index > v->size - 1 || v->size == 0)
		return NULL;
	return v->data[index];
}

int cvector_set(CVector* v, unsigned long index, void* data) {
	if (index > v->size - 1)
		return 0;

	v->data[index] = malloc(v->elem_size);

	if (v->data[index] == nullptr)
		throw_error(ERR_ALLOC, nullptr);

	memcpy(v->data[index], data, v->elem_size);
	return 1;
}

int cvector_delete(CVector* v, unsigned long index) {
	if (index > v->size - 1 || v->size == 0)
		return 0;

	if (cvector_get(v, index) != NULL)
		free(cvector_get(v, index));

	for (unsigned long i = index; i < v->size-1; i++)
		v->data[i] = v->data[i+1];

	v->size--;
	return 1;
}

int cvector_pop(CVector* v) {
	return cvector_delete(v, v->size-1);
}

int cvector_insert(CVector* v, unsigned long index, void* data) {
	if (index > v->size)
		return 0;

	if (v->size >= v->capacity)
		cvector_resize(v, v->capacity + CVECTOR_ADD);

	v->size++;

	for (unsigned long i = v->size-2; i >= index; i--)
		v->data[i+1] = v->data[i];

	cvector_set(v, index, data);
	return 1;
}
