#include "list.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

const size_t GROWTH_FACTOR = 2;

typedef struct list {
  void **data;
  size_t length;
  size_t size;
  free_func_t freer;
} list_t;

list_t *list_init(size_t initial_size, free_func_t freer) {
  list_t *list = malloc(sizeof(list_t));
  assert(list);

  void **data = malloc(sizeof(void *) * initial_size);
  assert(data);

  list->data = data;
  list->length = initial_size;
  list->size = 0;
  list->freer = freer;
  return list;
}

void list_free(list_t *list) {
  if (list->freer != NULL) {
    for (size_t i = 0; i < list->size; i++) {
      list->freer(list->data[i]);
    }
  }
  free(list->data);
  free(list);
}

size_t list_size(list_t *list) { return list->size; }

void *list_get(list_t *list, size_t index) {
  assert(index < list_size(list));

  return list->data[index];
}

void list_add(list_t *list, void *value) {

  if (list_size(list) == list->length) {
    size_t old_length = list->length;
    void **temp = malloc(sizeof(void *) * GROWTH_FACTOR * list->length);
    assert(temp);

    for (size_t i = 0; i < list->length; i++) {
      temp[i] = list->data[i];
    }
    free(list->data);
    list->data = temp;
    list->length = GROWTH_FACTOR * old_length;
    assert(list->length > old_length);
  }
  assert(value != NULL);

  list->data[list->size] = value;
  list->size++;
}

void *list_remove(list_t *list, size_t index) {
  assert(list->size != 0);
  assert(index < list_size(list));

  if (list->size == 1 || index == list->size - 1) {
    void *old_val = list->data[index];
    list->size--;
    return old_val;
  } else {
    void *old_val = list->data[index];
    for (size_t i = index; i < list->size - 1; i++) {
      list->data[i] = list->data[i + 1];
    }
    list->data[list->size - 1] = NULL;
    list->size--;
    return old_val;
  }
}
