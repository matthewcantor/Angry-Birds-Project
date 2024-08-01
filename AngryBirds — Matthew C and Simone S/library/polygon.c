#include "polygon.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

const double GRAVITY = -9.8;
const double ROT_ANGLE = 0;

typedef struct polygon {
  list_t *points;
  vector_t vel;
  double rot_speed;
  rgb_color_t *color;
  vector_t center;
  double rot_angle;
} polygon_t;

polygon_t *polygon_init(list_t *points, vector_t initial_velocity,
                        double rotation_speed, double red, double green,
                        double blue) {
  polygon_t *polygon = malloc(sizeof(polygon_t));
  assert(polygon);

  polygon->points = points;
  polygon->vel = initial_velocity;
  polygon->rot_speed = rotation_speed;
  polygon->color = color_init(red, green, blue);
  polygon->rot_angle = ROT_ANGLE;
  polygon->center = polygon_centroid(polygon);

  return polygon;
}

list_t *polygon_get_points(polygon_t *polygon) { return polygon->points; }

void polygon_move(polygon_t *polygon, double time_elapsed) {

  polygon_translate(polygon, vec_multiply(time_elapsed, polygon->vel));

  polygon_rotate(polygon, time_elapsed * polygon->rot_speed,
                 polygon_centroid(polygon));
}

void polygon_set_velocity(polygon_t *polygon, vector_t velocity) {
  polygon->vel.x = velocity.x;
  polygon->vel.y = velocity.y;
}

void polygon_free(polygon_t *polygon) {
  list_free(polygon->points);
  color_free(polygon->color);
  free(polygon);
}

double polygon_get_velocity_x(polygon_t *polygon) { return polygon->vel.x; }

double polygon_get_velocity_y(polygon_t *polygon) { return polygon->vel.y; }

vector_t *polygon_get_velocity(polygon_t *polygon) { return &polygon->vel; }

double polygon_area(polygon_t *polygon) {
  double area = 0;

  for (size_t i = 0; i < list_size(polygon_get_points(polygon)); i++) {
    vector_t vec_i = *(vector_t *)list_get(polygon_get_points(polygon), i);
    vector_t vec_i_plus =
        *(vector_t *)list_get(polygon_get_points(polygon),
                              (i + 1) % list_size(polygon_get_points(polygon)));

    area += vec_i.x * vec_i_plus.y;

    area -= vec_i.y * vec_i_plus.x;
  }

  return fabs(area) * 0.5;
}

vector_t polygon_centroid(polygon_t *polygon) {
  double x = 0;
  double y = 0;

  size_t size = list_size(polygon_get_points(polygon));

  for (size_t i = 0; i < size; i++) {

    // following the summation formula for a centroid

    vector_t *curr = list_get(polygon_get_points(polygon), i);
    vector_t *next = list_get(polygon_get_points(polygon), (i + 1) % size);

    x += (curr->x + next->x) * (vec_cross(*curr, *next));

    y += (curr->y + next->y) * (vec_cross(*curr, *next));
  }

  vector_t center = {x, y};

  return vec_multiply((1 / (6 * polygon_area(polygon))), center);
}

void polygon_translate(polygon_t *polygon, vector_t translation) {
  for (size_t i = 0; i < list_size(polygon_get_points(polygon)); i++) {
    *(vector_t *)list_get(polygon_get_points(polygon), i) = vec_add(
        *(vector_t *)list_get(polygon_get_points(polygon), i), translation);
  }
}

void polygon_rotate(polygon_t *polygon, double angle, vector_t point) {
  for (size_t i = 0; i < list_size(polygon_get_points(polygon)); i++) {
    vector_t vec_i = *(vector_t *)list_get(polygon_get_points(polygon), i);

    // subtract to shift origin to point being rotated about
    // rotate using the rotation matrix
    // add to bring the origin back to the original position

    vector_t to_origin = vec_subtract(vec_i, point);
    vector_t rotate = vec_rotate(to_origin, angle);
    vector_t ret = vec_add(rotate, point);
    *(vector_t *)list_get(polygon_get_points(polygon), i) = ret;
  }
}

rgb_color_t *polygon_get_color(polygon_t *polygon) { return polygon->color; }

void polygon_set_color(polygon_t *polygon, rgb_color_t *color) {
  rgb_color_t *old = polygon->color;
  polygon->color = color;
  color_free(old);
}

void polygon_set_center(polygon_t *polygon, vector_t centroid) {
  vector_t old_centroid = polygon_centroid(polygon);
  vector_t translate = vec_subtract(centroid, old_centroid);
  polygon_translate(polygon, translate);
  polygon->center = centroid;
}

vector_t polygon_get_center(polygon_t *polygon) { return polygon->center; }

void polygon_set_rotation(polygon_t *polygon, double rot) {
  polygon_rotate(polygon, rot - polygon->rot_angle, polygon->center);
  polygon->rot_angle = rot;
}

double polygon_get_rotation(polygon_t *polygon) { return polygon->rot_angle; }
