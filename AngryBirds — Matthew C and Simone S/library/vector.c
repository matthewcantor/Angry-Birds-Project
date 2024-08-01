#include "vector.h"
#include <math.h>
#include <stdlib.h>

const vector_t VEC_ZERO = {0, 0};

vector_t vec_add(vector_t v1, vector_t v2) {
  double final_x = v1.x + v2.x;
  double final_y = v1.y + v2.y;
  vector_t vec = {final_x, final_y};
  return vec;
}

vector_t vec_subtract(vector_t v1, vector_t v2) {
  double final_x = v1.x - v2.x;
  double final_y = v1.y - v2.y;

  vector_t vec = {final_x, final_y};
  return vec;
}

vector_t vec_negate(vector_t v) {
  double final_x = v.x * -1;
  double final_y = v.y * -1;

  vector_t vec = {final_x, final_y};
  return vec;
}

vector_t vec_multiply(double scalar, vector_t v) {
  double final_x = v.x * scalar;
  double final_y = v.y * scalar;

  vector_t vec = {final_x, final_y};
  return vec;
}

double vec_dot(vector_t v1, vector_t v2) {
  double final_x = v1.x * v2.x;
  double final_y = v1.y * v2.y;

  double final_dot = final_x + final_y;
  return final_dot;
}

double vec_cross(vector_t v1, vector_t v2) {
  double final_z = (v1.x * v2.y) - (v2.x * v1.y);

  return final_z;
}

vector_t vec_rotate(vector_t v, double angle) {
  double final_x = (v.x * cos(angle)) - (v.y * sin(angle));
  double final_y = (v.x * sin(angle)) + (v.y * cos(angle));

  vector_t vec = {final_x, final_y};
  return vec;
}

double vec_get_length(vector_t v) { return sqrt(pow(v.x, 2) + pow(v.y, 2)); }
