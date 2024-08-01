#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <assert.h>
#include <stdbool.h>

#include "asset.h"
#include "asset_cache.h"
#include "color.h"
#include "sdl_wrapper.h"

typedef struct asset {
  asset_type_t type;
  SDL_Rect bounding_box;
} asset_t;

typedef struct text_asset {
  asset_t base;
  TTF_Font *font;
  const char *text;
  rgb_color_t color;
} text_asset_t;

typedef struct image_asset {
  asset_t base;
  SDL_Texture *texture;
  body_t *body;
} image_asset_t;

typedef struct button_asset {
  asset_t base;
  image_asset_t *image_asset;
  text_asset_t *text_asset;
  button_handler_t handler;
  bool is_rendered;
} button_asset_t;

/**
 * Allocates memory for an asset with the given parameters.
 *
 * @param ty the type of the asset
 * @param bounding_box the bounding box containing the location and dimensions
 * of the asset when it is rendered
 * @return a pointer to the newly allocated asset
 */
static asset_t *asset_init(asset_type_t ty, SDL_Rect bounding_box) {
  asset_t *new;
  switch (ty) {
  case ASSET_IMAGE: {
    new = malloc(sizeof(image_asset_t));
    break;
  }
  case ASSET_FONT: {
    new = malloc(sizeof(text_asset_t));
    break;
  }
  case ASSET_BUTTON: {
    new = malloc(sizeof(button_asset_t));
    break;
  }
  default: {
    assert(false && "Unknown asset type");
  }
  }
  assert(new);
  new->type = ty;
  new->bounding_box = bounding_box;
  return new;
}

/**
 * Allocates memory for an image asset with an attached body. When the asset
 * is rendered, the image will be rendered on top of the body.
 *
 * @param filepath the filepath to the image file
 * @param body the body to render the image on top of
 * @return a pointer to the newly allocated image asset
 */
asset_t *asset_make_image_with_body(const char *filepath, body_t *body) {
  if (filepath == NULL) {
    return NULL;
  }

  SDL_Rect rect = make_bounding_box(body);
  image_asset_t *ret = (image_asset_t *)asset_make_image(filepath, rect);
  ret->body = body;

  return (asset_t *)ret;
}

asset_type_t asset_get_type(asset_t *asset) { return asset->type; }

asset_t *asset_make_image(const char *filepath, SDL_Rect bounding_box) {
  if (filepath == NULL) {
    return NULL;
  }
  asset_t *base = asset_init(ASSET_IMAGE, bounding_box);

  SDL_Texture *texture = asset_cache_obj_get_or_create(ASSET_IMAGE, filepath);

  image_asset_t *ret = malloc(sizeof(image_asset_t));
  assert(ret);

  ret->base = *base;
  ret->texture = texture;
  ret->body = NULL;

  return (asset_t *)ret;
}

asset_t *asset_make_text(const char *filepath, SDL_Rect bounding_box,
                         const char *text, rgb_color_t color) {
  if (filepath == NULL) {
    return NULL;
  }
  asset_t *base = asset_init(ASSET_FONT, bounding_box);

  TTF_Font *font = asset_cache_obj_get_or_create(ASSET_FONT, filepath);

  text_asset_t *ret = malloc(sizeof(text_asset_t));
  assert(ret);

  ret->base = *base;
  ret->color = color;
  ret->font = font;
  ret->text = text;

  return (asset_t *)ret;
}

asset_t *asset_make_button(SDL_Rect bounding_box, asset_t *image_asset,
                           asset_t *text_asset, button_handler_t handler) {
  if (image_asset != NULL) {
    assert(image_asset);
  }
  if (text_asset != NULL) {
    assert(text_asset);
  }

  asset_t *base = asset_init(ASSET_BUTTON, bounding_box);

  button_asset_t *ret = (button_asset_t *)base;

  ret->image_asset = (image_asset_t *)image_asset;
  ret->text_asset = (text_asset_t *)text_asset;
  ret->handler = handler;
  ret->is_rendered = false;

  asset_cache_register_button((asset_t *)ret);

  return (asset_t *)ret;
}

bool in_box(asset_t *button, double x, double y) {
  SDL_Rect box = button->bounding_box;
  size_t min_x = box.x;
  size_t max_x = box.x + box.w;
  size_t min_y = box.y;
  size_t max_y = box.y + box.h;
  return (x > min_x && x < max_x && y > min_y && y < max_y);
}

void asset_on_button_click(asset_t *button, bool mouse_type, state_t *state,
                           double x, double y) {
  if (!(((button_asset_t *)button)->is_rendered)) {
    return;
  }
  if (in_box(button, x, y)) {
    ((button_asset_t *)button)->is_rendered = false;
    (((button_asset_t *)button)->handler)(state, mouse_type, x, y);
  }
}

void asset_render(asset_t *asset) {
  if (!asset) {
    return;
  }
  if (asset->type == ASSET_IMAGE) {
    image_asset_t *img = (image_asset_t *)asset;

    if (img->body != NULL) {
      asset->bounding_box = make_bounding_box(img->body);
    }

    vector_t loc = {asset->bounding_box.x, asset->bounding_box.y};
    vector_t size = {asset->bounding_box.w, asset->bounding_box.h};

    sdl_draw_img(img->texture, loc, size);
  } else if (asset->type == ASSET_FONT) {
    text_asset_t *text = (text_asset_t *)asset;

    vector_t loc = {asset->bounding_box.x, asset->bounding_box.y};

    make_text((char *)text->text, text->font, loc, asset->bounding_box.w,
              asset->bounding_box.h, text->color);
  } else if (asset->type == ASSET_BUTTON) {
    ((button_asset_t *)asset)->is_rendered = true;
    asset_render((asset_t *)((button_asset_t *)asset)->image_asset);
    asset_render((asset_t *)((button_asset_t *)asset)->text_asset);
  }
}

body_t *get_body(asset_t *asset) { return ((image_asset_t *)asset)->body; }

bool asset_body_removed(asset_t *asset) {
  return body_is_removed((body_t *)(((image_asset_t *)asset)->body));
}

void asset_destroy(asset_t *asset) { free(asset); }
