#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "asset.h"
#include "asset_cache.h"
#include "collision.h"
#include "forces.h"
#include "sdl_wrapper.h"

const vector_t MIN = {0, 0};
const vector_t MAX = {1000, 500};

const size_t NUM_BUTTONS = 3;
const size_t NUM_BODIES = 10;
const size_t NUM_BIRDS = 5;
const size_t NUM_INIT_ENEMS = 4;
const size_t NUM_WOOD = 19;
const size_t NUM_BACKGROUNDS = 4;
const size_t START_BACKGROUND_INDEX = 0;
const size_t PLAY_BACKGROUND_INDEX = 1;
const size_t GAME_OVER_BACKGROUND_INDEX = 2;
const size_t WIN_BACKGROUND_INDEX = 3;

vector_t enem_locs[] = {{500, 100}, {600, 350}, {675, 175}, {825, 400}};

vector_t wood_locs[] = {{500, 0},   {500, 50},  {600, 0},   {600, 50},
                        {600, 100}, {600, 250}, {600, 300}, {675, 0},
                        {675, 50},  {675, 100}, {675, 125}, {825, 0},
                        {825, 50},  {825, 100}, {825, 150}, {825, 200},
                        {825, 250}, {825, 300}, {825, 350}};

const char *BACKGROUND_PATH = "assets/angry_birds_background.jpg";
const char *START_PATH = "assets/Angry_birds_start.jpg";
const char *BIRD_PATH = "assets/RedBird.png";
const char *PIG_PATH = "assets/EnemyPig.png";
const char *SLINGSHOT_PATH = "assets/SlingShot.png";
const char *PLAY_PATH = "assets/play_button.png";
const char *PAUSE_PATH = "assets/pause_button.png";
const char *WOOD_PATH = "assets/WoodenBlock.png";
const char *GAME_OVER_PATH = "assets/Game over screen.png";
const char *WIN_PATH = "assets/WinScreen.png";
const char *FONT_PATH = "assets/BrownieStencil-8O8MJ.ttf";
const char *REPLAY_BUTTON_PATH = "assets/Replay Button.png";
const char *SONG = "assets/Angry Birds Theme Song.wav";

const char *POINT_LABEL = "Points:";
const char *SHOT_MARKER_LABEL = "Birds Remaining:";

const size_t CIRC_NPOINTS = 100;
const double ENEMY_RADIUS = 25;
const double BIRD_RADIUS = 20;
const double ENEMY_MASS = __DBL_MAX__;
const double BIRD_MASS = 5;
const double ENEMY_HEALTH = 150;
const double ELASTICITY = 1;
const vector_t BIRD_START_LOC = {100, 80};
const double FIRST_MARKER_X = 35;
const double FIRST_MARKER_Y = 425;
const double MARKER_MULTIPLIER = 40;
const double VEL_MULTIPLIER = 10;
const double GAME_GRAVITY = 750;
const double POINT_INCREMENT = 100;
const double WOOD_WIDTH = 50;
const double WOOD_HEIGHT = 80;
const double MAX_POINT_LEN = 6;
const double GROUND_WEIGHT = 100000;

const size_t SCENE_BIRD_INDEX = 0;

const double WALL_DIM = 1;
rgb_color_t white = (rgb_color_t){1, 1, 1};

const vector_t SLING_SIZE = {30, 100};
const SDL_Rect SLING_BOX = (SDL_Rect){15, 400, 130, 100};
const SDL_Rect PLAY_BOX = (SDL_Rect){420, 270, 200, 150};
const SDL_Rect RESET_BOX = (SDL_Rect){420, 340, 200, 150};
const SDL_Rect PAUSE_BOX = (SDL_Rect){875, 7, 75, 75};
const SDL_Rect POINT_BOX = (SDL_Rect){575, 7, 200, 75};
const SDL_Rect SHOT_MARKER_BOX = (SDL_Rect){20, 7, 225, 75};

struct state {
  list_t *body_assets;
  list_t *button_assets;
  asset_t *sling;
  asset_t *play_button;
  asset_t *pause_button;
  asset_t *reset_button;
  body_t *ground;
  size_t curr_bird_num;
  list_t *backgrounds;
  list_t *birds;
  list_t *enemies;
  list_t *walls;
  list_t *shot_marker;
  scene_t *scene;
  vector_t mouse;
  bool sling_down;
  bool play;
  bool over;
  TTF_Font *font;
  size_t points;
};

typedef enum { PROJECTILE, WALL, ENEMY, GROUND } body_type_t;

body_type_t *make_type_info(body_type_t type) {
  body_type_t *info = malloc(sizeof(body_type_t));
  assert(info);
  *info = type;
  return info;
}

body_type_t get_type(body_t *body) {
  return *(body_type_t *)body_get_info(body);
}

void slingshot(state_t *state, bool mouse_type, double x, double y) {
  if (!(mouse_type)) {
    state->mouse = (vector_t){x, y};

  } else {
    body_remove(get_body(
        list_get(state->shot_marker, list_size(state->shot_marker) - 1)));
    list_remove(state->shot_marker, list_size(state->shot_marker) - 1);
    state->curr_bird_num -= 1;

    vector_t sling_force = vec_subtract(state->mouse, (vector_t){x, y});

    vector_t new_vel = (vector_t){sling_force.x, -1 * sling_force.y};
    body_set_velocity(get_body(list_get(state->birds, 0)),
                      vec_multiply(VEL_MULTIPLIER, new_vel));

    create_newtonian_gravity(state->scene, GAME_GRAVITY,
                             get_body(list_get(state->birds, 0)),
                             state->ground);
  }
}

void toggle_play(state_t *state, bool mouse_type, double x, double y) {
  state->play = !state->play;
}

void pig_bird_collision_handler(body_t *bird, body_t *enemy, vector_t axis,
                                void *aux, double force_const) {
  physics_collision_handler(bird, enemy, axis, aux, force_const);
  ((state_t *)aux)->points += POINT_INCREMENT;
  body_remove(enemy);
}

void create_pig_bird_collision(scene_t *scene, body_t *bird, body_t *enemy,
                               double elasticity, state_t *state) {
  create_collision(scene, bird, enemy,
                   (collision_handler_t)pig_bird_collision_handler, state,
                   elasticity);
}

void ground_wall_collision_handler(body_t *bird, body_t *boundary,
                                   vector_t axis, void *aux,
                                   double force_const) {
  if (((state_t *)aux)->curr_bird_num > 0) {
    body_remove(bird);
  } else if (((state_t *)aux)->curr_bird_num == 0) {
    body_remove(bird);
    return;
  }
}

void create_ground_wall_collision(scene_t *scene, body_t *bird,
                                  body_t *boundary, double elasticity,
                                  state_t *state) {
  create_collision(scene, bird, boundary,
                   (collision_handler_t)ground_wall_collision_handler, state,
                   elasticity);
}

list_t *make_circle(vector_t center, double radius) {
  list_t *c = list_init(CIRC_NPOINTS, free);
  for (size_t i = 0; i < CIRC_NPOINTS; i++) {
    double angle = 2 * M_PI * i / CIRC_NPOINTS;
    vector_t *v = malloc(sizeof(*v));
    assert(v);
    *v = (vector_t){center.x + radius * cos(angle),
                    center.y + radius * sin(angle)};
    list_add(c, v);
  }
  return c;
}

list_t *make_rectangle(vector_t center, double width, double height) {
  list_t *points = list_init(4, free);
  vector_t *p1 = malloc(sizeof(vector_t));
  assert(p1);
  *p1 = (vector_t){center.x - width / 2, center.y - height / 2};

  vector_t *p2 = malloc(sizeof(vector_t));
  assert(p2);
  *p2 = (vector_t){center.x + width / 2, center.y - height / 2};

  vector_t *p3 = malloc(sizeof(vector_t));
  assert(p3);
  *p3 = (vector_t){center.x + width / 2, center.y + height / 2};

  vector_t *p4 = malloc(sizeof(vector_t));
  assert(p4);
  *p4 = (vector_t){center.x - width / 2, center.y + height / 2};

  list_add(points, p1);
  list_add(points, p2);
  list_add(points, p3);
  list_add(points, p4);

  return points;
}

asset_t *make_enemy(state_t *state, double health, double mass,
                    free_func_t info_freer, vector_t loc) {
  list_t *shape = make_circle(MIN, ENEMY_RADIUS);
  enemy_body_t *ret = enemy_body_init_with_info(
      health, shape, mass, white, make_type_info(ENEMY), info_freer);

  body_t *body = enemy_body_get_body(ret);

  body_set_centroid(body, loc);
  scene_add_body(state->scene, body);

  asset_t *pig = asset_make_image_with_body(PIG_PATH, body);
  list_add(state->enemies, pig);

  return pig;
}

asset_t *make_bird(state_t *state, double mass, rgb_color_t color,
                   free_func_t info_freer, vector_t loc, bool shooter) {

  list_t *shape = make_circle(MIN, BIRD_RADIUS);
  body_t *body = body_init_with_info(shape, mass, color,
                                     make_type_info(PROJECTILE), info_freer);

  body_set_centroid(body, loc);
  scene_add_body(state->scene, body);
  asset_t *bird = asset_make_image_with_body(BIRD_PATH, body);
  if (shooter) {
    list_add(state->birds, bird);
  } else {
    list_add(state->shot_marker, bird);
  }

  return bird;
}

asset_t *make_wood(state_t *state, double mass, rgb_color_t color,
                   vector_t loc) {
  list_t *shape = make_rectangle(MIN, WOOD_WIDTH, WOOD_HEIGHT);
  body_t *body =
      body_init_with_info(shape, mass, color, make_type_info(WALL), free);

  body_set_centroid(body, loc);
  scene_add_body(state->scene, body);

  asset_t *wall = asset_make_image_with_body(WOOD_PATH, body);
  list_add(state->walls, wall);

  return wall;
}

void add_walls(state_t *state) {
  list_t *wall1_shape =
      make_rectangle((vector_t){MAX.x, MAX.y / 2}, WALL_DIM, MAX.y);
  body_t *wall1 = body_init_with_info(wall1_shape, __DBL_MAX__, white,
                                      make_type_info(WALL), free);
  list_t *wall2_shape =
      make_rectangle((vector_t){0, MAX.y / 2}, WALL_DIM, MAX.y);
  body_t *wall2 = body_init_with_info(wall2_shape, __DBL_MAX__, white,
                                      make_type_info(WALL), free);
  list_t *ceiling_shape =
      make_rectangle((vector_t){MAX.x / 2, MAX.y}, MAX.x, WALL_DIM);
  body_t *ceiling = body_init_with_info(ceiling_shape, __DBL_MAX__, white,
                                        make_type_info(WALL), free);
  list_t *ground_shape =
      make_rectangle((vector_t){MAX.x / 2, 0}, MAX.x, WALL_DIM);
  body_t *ground = body_init_with_info(ground_shape, GROUND_WEIGHT, white,
                                       make_type_info(GROUND), free);
  state->ground = ground;
  scene_add_body(state->scene, wall1);
  scene_add_body(state->scene, wall2);
  scene_add_body(state->scene, ceiling);
  scene_add_body(state->scene, ground);
}

void add_force_creators(state_t *state) {
  for (size_t j = 0; j < list_size(state->birds); j++) {
    body_t *bird = get_body(list_get(state->birds, j));
    for (size_t i = 0; i < scene_bodies(state->scene); i++) {
      body_t *body = scene_get_body(state->scene, i);
      switch (get_type(body)) {
      case ENEMY:
        create_pig_bird_collision(state->scene, bird, body, ELASTICITY, state);
        break;
      case WALL:
        create_ground_wall_collision(state->scene, bird, body, ELASTICITY,
                                     state);
        break;
      case GROUND:
        create_ground_wall_collision(state->scene, bird, body, ELASTICITY,
                                     state);
        break;
      default:
        break;
      }
    }
  }
}

void make_birds_enemies_forces(state_t *state) {
  for (size_t i = 0; i < NUM_BIRDS; i++) {
    make_bird(state, BIRD_MASS, white, free, BIRD_START_LOC, true);
  }
  for (size_t i = 0; i < NUM_BIRDS; i++) {
    make_bird(
        state, BIRD_MASS, white, free,
        (vector_t){FIRST_MARKER_X + (MARKER_MULTIPLIER * i), FIRST_MARKER_Y},
        false);
  }
  for (size_t i = 0; i < NUM_INIT_ENEMS; i++) {
    vector_t loc = enem_locs[i];
    make_enemy(state, ENEMY_HEALTH, ENEMY_MASS, free, loc);
  }
  add_force_creators(state);
}

void reset_play(state_t *state, bool mouse_type, double x, double y) {
  state->points = 0;
  state->sling_down = false;
  state->play = true;
  state->over = false;
  state->curr_bird_num = NUM_BIRDS;

  for (size_t i = 0; i < list_size(state->birds); i++) {
    list_remove(state->birds, i);
  }

  for (size_t i = 0; i < list_size(state->shot_marker); i++) {
    list_remove(state->shot_marker, i);
  }

  for (size_t i = 0; i < list_size(state->enemies); i++) {
    list_remove(state->enemies, i);
  }

  make_birds_enemies_forces(state);
}

asset_t *create_sling_button(state_t *state, SDL_Rect box,
                             const char *img_path) {
  asset_t *img = asset_make_image(img_path, box);
  list_add(state->button_assets, img);

  asset_t *button = asset_make_button(box, img, NULL, (void *)slingshot);
  return button;
}

asset_t *create_play_button(state_t *state, SDL_Rect box,
                            const char *img_path) {
  asset_t *img = asset_make_image(img_path, box);
  list_add(state->button_assets, img);

  asset_t *button = asset_make_button(box, img, NULL, (void *)toggle_play);
  return button;
}

asset_t *create_pause_button(state_t *state, SDL_Rect box,
                             const char *img_path) {
  asset_t *img = asset_make_image(img_path, box);
  list_add(state->button_assets, img);

  asset_t *button = asset_make_button(box, img, NULL, (void *)toggle_play);
  return button;
}

asset_t *create_reset_button(state_t *state, SDL_Rect box,
                             const char *img_path) {
  asset_t *img = asset_make_image(img_path, box);
  list_add(state->button_assets, img);

  asset_t *button = asset_make_button(box, img, NULL, (void *)reset_play);
  return button;
}

asset_t *points_text_assets(size_t points) {

  char *point_str =
      malloc(sizeof(char) * (MAX_POINT_LEN + strlen(POINT_LABEL)));
  assert(point_str);

  sprintf(point_str, "%s %zu", POINT_LABEL, points);

  return asset_make_text(FONT_PATH, POINT_BOX, point_str, white);
}

state_t *emscripten_init() {
  asset_cache_init();
  sdl_init(MIN, MAX);
  state_t *state = malloc(sizeof(state_t));
  state->points = 0;
  state->scene = scene_init();
  state->body_assets = list_init(1, (free_func_t)asset_destroy);
  state->button_assets = list_init(NUM_BUTTONS, (free_func_t)asset_destroy);
  state->birds = list_init(NUM_BIRDS, (free_func_t)asset_destroy);
  state->walls = list_init(NUM_WOOD, (free_func_t)asset_destroy);
  state->enemies = list_init(NUM_INIT_ENEMS, (free_func_t)asset_destroy);
  state->sling_down = false;
  state->shot_marker = list_init(NUM_BIRDS, (free_func_t)asset_destroy);
  state->curr_bird_num = NUM_BIRDS;
  state->backgrounds = list_init(NUM_BACKGROUNDS, (free_func_t)asset_destroy);
  state->play = false;
  state->over = false;

  SDL_Rect background_box = {
      .x = MIN.x, .y = MIN.y, .w = MAX.x - MIN.x, .h = MAX.y - MIN.y};

  list_add(state->backgrounds, asset_make_image(START_PATH, background_box));
  list_add(state->backgrounds,
           asset_make_image(BACKGROUND_PATH, background_box));
  list_add(state->backgrounds,
           asset_make_image(GAME_OVER_PATH, background_box));
  list_add(state->backgrounds, asset_make_image(WIN_PATH, background_box));

  state->sling = create_sling_button(state, SLING_BOX, SLINGSHOT_PATH);

  state->play_button = create_play_button(state, PLAY_BOX, PLAY_PATH);
  state->pause_button = create_pause_button(state, PAUSE_BOX, PAUSE_PATH);
  state->reset_button =
      create_reset_button(state, RESET_BOX, REPLAY_BUTTON_PATH);

  for (size_t i = 0; i < NUM_WOOD; i++) {
    vector_t loc = wood_locs[i];
    make_wood(state, ENEMY_MASS, white, loc);
  }

  add_walls(state);

  make_birds_enemies_forces(state);
  sdl_play_music((char *)SONG);

  return state;
}

bool emscripten_main(state_t *state) {
  double dt = time_since_last_tick();
  sdl_clear();

  if (state->play) {

    asset_render(list_get(state->backgrounds, PLAY_BACKGROUND_INDEX));
    asset_render(state->sling);
    asset_render(state->pause_button);

    asset_render(
        asset_make_text(FONT_PATH, SHOT_MARKER_BOX, SHOT_MARKER_LABEL, white));

    asset_render(points_text_assets(state->points));

    for (size_t i = 0; i < list_size(state->birds); i++) {
      if (!(asset_body_removed(list_get(state->birds, i)))) {
        asset_render(list_get(state->birds, i));
      } else {
        list_remove(state->birds, i);
      }
    }

    for (size_t i = 0; i < list_size(state->shot_marker); i++) {
      asset_render(list_get(state->shot_marker, i));
    }

    for (size_t i = 0; i < list_size(state->walls); i++) {
      asset_render(list_get(state->walls, i));
    }

    for (size_t i = 0; i < list_size(state->enemies); i++) {
      if (!(asset_body_removed(list_get(state->enemies, i)))) {
        asset_render(list_get(state->enemies, i));
      } else {
        list_remove(state->enemies, i);
      }
    }

    if (list_size(state->birds) == 0 || list_size(state->enemies) == 0) {
      state->over = true;
      state->play = false;
    }
  } else if (state->over) {
    if (list_size(state->enemies) == 0) {
      asset_render(list_get(state->backgrounds, WIN_BACKGROUND_INDEX));
      asset_render(state->reset_button);
    } else {
      asset_render(list_get(state->backgrounds, GAME_OVER_BACKGROUND_INDEX));
      asset_render(state->reset_button);
    }
  } else {
    asset_render(list_get(state->backgrounds, START_BACKGROUND_INDEX));
    asset_render(state->play_button);
  }
  sdl_show();

  scene_tick(state->scene, dt);
  return false;
}

void emscripten_free(state_t *state) {
  list_free(state->body_assets);
  list_free(state->button_assets);
  list_free(state->backgrounds);
  list_free(state->birds);
  list_free(state->enemies);
  list_free(state->walls);
  list_free(state->shot_marker);
  body_free(state->ground);
  scene_free(state->scene);
  asset_cache_destroy();
  free(state);
}