#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <assert.h>
#include <stdbool.h>

#include "asset.h"
#include "asset_cache.h"
#include "list.h"
#include "sdl_wrapper.h"

static list_t *ASSET_CACHE;

const size_t FONT_SIZE = 18;
const size_t INITIAL_CAPACITY = 5;

typedef struct {
  asset_type_t type;
  const char *filepath;
  void *obj;
} entry_t;

static void asset_cache_free_entry(entry_t *entry) {
  if (entry->type == ASSET_IMAGE) {
    SDL_DestroyTexture(entry->obj);
  } else if (entry->type == ASSET_FONT) {
    TTF_CloseFont(entry->obj);
  } else if (entry->type == ASSET_BUTTON) {
    asset_destroy(entry->obj);
  }
  free(entry);
}

void asset_cache_init() {
  ASSET_CACHE =
      list_init(INITIAL_CAPACITY, (free_func_t)asset_cache_free_entry);
}

void asset_cache_destroy() { list_free(ASSET_CACHE); }

void *file_already_exists(const char *filepath) {
  for (size_t i = 0; i < list_size(ASSET_CACHE); i++) {
    entry_t *curr = list_get(ASSET_CACHE, i);
    if (curr->filepath == NULL) {
      continue;
    }
    if (strcmp(curr->filepath, filepath) == 0) {
      return curr->obj;
    }
  }
  return NULL;
}

void *asset_cache_obj_get_or_create(asset_type_t ty, const char *filepath) {
  if (file_already_exists(filepath) != NULL) {
    return file_already_exists(filepath);
  } else {
    entry_t *ret = malloc(sizeof(entry_t));
    assert(ret);

    ret->filepath = filepath;

    if (ty == ASSET_IMAGE) {
      SDL_Texture *object = make_img(filepath);
      ret->obj = object;
      ret->type = ASSET_IMAGE;
      list_add(ASSET_CACHE, ret);

      return object;
    } else {
      TTF_Font *object = open_font(filepath, FONT_SIZE);
      ret->obj = object;
      ret->type = ASSET_FONT;
      list_add(ASSET_CACHE, ret);

      return object;
    }
  }
}

void asset_cache_register_button(asset_t *button) {
  assert(asset_get_type(button) == ASSET_BUTTON);
  entry_t *ret = malloc(sizeof(entry_t));
  assert(ret);

  ret->type = ASSET_BUTTON;
  ret->filepath = NULL;
  ret->obj = button;
  list_add(ASSET_CACHE, ret);
}

void asset_cache_handle_buttons(state_t *state, bool mouse_type, double x,
                                double y) {
  for (size_t i = 0; i < list_size(ASSET_CACHE); i++) {
    if (asset_get_type(list_get(ASSET_CACHE, i)) == ASSET_BUTTON) {
      asset_on_button_click(((entry_t *)list_get(ASSET_CACHE, i))->obj,
                            mouse_type, state, x, y);
    }
  }
}
