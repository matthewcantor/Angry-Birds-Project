#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "body.h"
#include "polygon.h"

struct body {
  polygon_t *poly;

  double mass;
  vector_t centroid;

  vector_t force;
  vector_t impulse;
  bool removed;

  void *info;
  free_func_t info_freer;
};

struct enemy_body {
  body_t *body;
  double health;
};

const double INITIAL_ROT = 0;
const vector_t INIT_VEL = {0, 0};

void body_reset(body_t *body) {
  body->force = VEC_ZERO;
  body->impulse = VEC_ZERO;
}

/**
 * Allocates memory for a body with the given parameters.
 * The body is initially at rest.
 * Asserts that the mass is positive and that the required memory is allocated.
 *
 * @param shape a list of vectors describing the initial shape of the body
 * @param mass the mass of the body (if INFINITY, stops the body from moving)
 * @param color the color of the body, used to draw it on the screen
 * @param info additional information to associate with the body,
 *   e.g. its type if the scene has multiple types of bodies
 * @param info_freer if non-NULL, a function call on the info to free it
 * @return a pointer to the newly allocated body
 */
body_t *body_init_with_info(list_t *shape, double mass, rgb_color_t color,
                            void *info, free_func_t info_freer) {
  body_t *ret = malloc(sizeof(body_t));
  assert(ret);

  polygon_t *poly =
      polygon_init(shape, INIT_VEL, INITIAL_ROT, color.r, color.g, color.b);

  ret->centroid = polygon_centroid(poly);
  ret->force = VEC_ZERO;
  ret->impulse = VEC_ZERO;
  ret->info = info;
  ret->info_freer = info_freer;
  ret->mass = mass;
  ret->poly = poly;
  ret->removed = false;

  return ret;
}

enemy_body_t *enemy_body_init(double health, list_t *shape, double mass,
                              rgb_color_t color) {
  enemy_body_t *ret = malloc(sizeof(enemy_body_t));
  assert(ret);

  body_t *body = body_init(shape, mass, color);
  ret->body = body;
  ret->health = health;

  return ret;
}

enemy_body_t *enemy_body_init_with_info(double health, list_t *shape,
                                        double mass, rgb_color_t color,
                                        void *info, free_func_t info_freer) {
  enemy_body_t *ret = malloc(sizeof(enemy_body_t));
  assert(ret);

  body_t *body = body_init_with_info(shape, mass, color, info, info_freer);
  ret->body = body;
  ret->health = health;

  return ret;
}

body_t *enemy_body_get_body(enemy_body_t *enemy) { return enemy->body; }

double enemy_get_health(body_t *enemy) {
  return ((enemy_body_t *)enemy)->health;
}

void enemy_subtract_health(body_t *enemy, double minus) {
  ((enemy_body_t *)enemy)->health = enemy_get_health(enemy) - minus;
}

polygon_t *body_get_polygon(body_t *body) { return body->poly; }

void *body_get_info(body_t *body) { return body->info; }

body_t *body_init(list_t *shape, double mass, rgb_color_t color) {
  return body_init_with_info(shape, mass, color, NULL, NULL);
}

void body_free(body_t *body) {
  polygon_free(body->poly);

  if (body->info_freer != NULL) {
    body->info_freer(body->info);
  }

  free(body);
}

void enemy_body_free(enemy_body_t *enemy) {
  body_free(enemy->body);
  free(enemy);
}

list_t *body_get_shape(body_t *body) {
  list_t *ret =
      list_init(list_size(polygon_get_points(body->poly)), (free_func_t)free);

  for (size_t i = 0; i < list_size(polygon_get_points(body->poly)); i++) {
    vector_t *curr = malloc(sizeof(vector_t));
    assert(curr);

    vector_t *old_vector = list_get(polygon_get_points(body->poly), i);

    curr->x = old_vector->x;
    curr->y = old_vector->y;

    list_add(ret, curr);
  }

  return ret;
}

vector_t body_get_centroid(body_t *body) { return body->centroid; }

vector_t body_get_velocity(body_t *body) {
  double x_vel = polygon_get_velocity_x(body->poly);
  double y_vel = polygon_get_velocity_y(body->poly);

  return (vector_t){x_vel, y_vel};
}

rgb_color_t *body_get_color(body_t *body) {
  return polygon_get_color(body->poly);
}

void body_set_color(body_t *body, rgb_color_t *col) {
  polygon_set_color(body->poly, col);
}

void body_set_centroid(body_t *body, vector_t v) {
  polygon_set_center(body->poly, v);

  body->centroid = v;
}

void body_set_velocity(body_t *body, vector_t v) {
  polygon_set_velocity(body->poly, v);
}

double body_get_rotation(body_t *body) {
  return polygon_get_rotation(body->poly);
}

void body_set_rotation(body_t *body, double angle) {
  polygon_set_rotation(body->poly, angle);
}

void body_tick(body_t *body, double dt) {
  vector_t old_vel = *polygon_get_velocity(body->poly);
  vector_t v_force = vec_multiply((dt / body->mass), body->force);
  vector_t v_impulse = vec_multiply((1 / body->mass), body->impulse);

  vector_t v_force_impulse = vec_add(v_force, v_impulse);
  vector_t v_new = vec_add(v_force_impulse, old_vel);

  vector_t avg_v = vec_multiply(0.5, vec_add(old_vel, v_new));
  vector_t change = vec_multiply(dt, avg_v);

  polygon_set_velocity(body->poly, v_new);
  polygon_translate(body->poly, change);
  body->centroid = polygon_centroid(body->poly);
  body->force = VEC_ZERO;
  body->impulse = VEC_ZERO;
}

double body_get_mass(body_t *body) { return body->mass; }

void body_add_force(body_t *body, vector_t force) {
  body->force = vec_add(body->force, force);
}

void body_add_impulse(body_t *body, vector_t impulse) {
  body->impulse = vec_add(body->impulse, impulse);
}

void body_remove(body_t *body) { body->removed = true; }

bool body_is_removed(body_t *body) { return body->removed; }
