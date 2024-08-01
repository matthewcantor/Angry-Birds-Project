#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "body.h"
#include "forces.h"
#include "list.h"
#include "scene.h"

struct scene {
  size_t num_bodies;
  list_t *bodies;
  list_t *force_creators;
  list_t *auxs;
  list_t *force_bodies;
};

const size_t SCENE_CAPACITY = 15;

force_creator_t force_creator_scene = NULL;

scene_t *scene_init(void) {
  scene_t *scene = malloc(sizeof(scene_t));
  assert(scene);

  list_t *body = list_init(SCENE_CAPACITY, (free_func_t)body_free);
  list_t *force_creators = list_init(SCENE_CAPACITY, NULL);
  list_t *aux = list_init(SCENE_CAPACITY, (free_func_t)body_aux_free);
  list_t *force_bodies = list_init(SCENE_CAPACITY, (free_func_t)list_free);

  scene->bodies = body;
  scene->force_creators = force_creators;
  scene->auxs = aux;
  scene->force_bodies = force_bodies;
  scene->num_bodies = 0;

  return scene;
}

void scene_free(scene_t *scene) {
  list_free(scene->bodies);
  list_free(scene->force_creators);
  list_free(scene->auxs);
  list_free(scene->force_bodies);
  free(scene);
}

size_t scene_bodies(scene_t *scene) { return scene->num_bodies; }

body_t *scene_get_body(scene_t *scene, size_t index) {
  return list_get(scene->bodies, index);
}

void scene_add_body(scene_t *scene, body_t *body) {
  list_add(scene->bodies, body);
  scene->num_bodies++;
}

void scene_remove_body(scene_t *scene, size_t index) {
  assert(index < list_size(scene->bodies));

  body_t *body = list_get(scene->bodies, index);
  body_remove(body);
}

void scene_add_force_creator(scene_t *scene, force_creator_t force_creator,
                             void *aux) {
  scene_add_bodies_force_creator(scene, force_creator, aux, list_init(0, free));
}

void scene_add_bodies_force_creator(scene_t *scene, force_creator_t forcer,
                                    void *aux, list_t *bodies) {
  list_add(scene->force_bodies, bodies);
  list_add(scene->auxs, aux);
  list_add(scene->force_creators, forcer);
}

void scene_tick(scene_t *scene, double dt) {
  for (ssize_t i = 0; i < (ssize_t)(scene->num_bodies); i++) {
    body_t *curr = scene_get_body(scene, i);

    if (body_is_removed(curr)) {
      for (ssize_t j = 0; j < (ssize_t)(list_size(scene->force_creators));
           j++) {
        list_t *curr_force_bodies = list_get(scene->force_bodies, j);

        for (ssize_t k = 0; k < (ssize_t)(list_size(curr_force_bodies)); k++) {
          if (list_get(curr_force_bodies, k) == curr) {
            list_remove(scene->force_creators, j);

            body_aux_t *remove_aux = list_remove(scene->auxs, j);
            body_aux_free(remove_aux);

            list_t *remove_force_body = list_remove(scene->force_bodies, j);
            list_free(remove_force_body);

            j--;
            break;
          }
        }
      }

      list_remove(scene->bodies, i);
      body_free(curr);
      i--;
      scene->num_bodies--;
    } else {
      body_tick(curr, dt);
    }
  }

  for (ssize_t h = 0; h < (ssize_t)(list_size(scene->force_creators)); h++) {
    force_creator_t force = list_get(scene->force_creators, h);
    force(list_get(scene->auxs, h));
  }
}
