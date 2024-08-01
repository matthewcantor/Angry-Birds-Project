#ifndef __SDL_WRAPPER_H__
#define __SDL_WRAPPER_H__

#include "color.h"
#include "list.h"
#include "polygon.h"
#include "scene.h"
#include "state.h"
#include "vector.h"
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL_mixer.h>
#include <stdbool.h>

// Values passed to a key handler when the given arrow key is pressed
typedef enum {
  LEFT_ARROW = 1,
  UP_ARROW = 2,
  RIGHT_ARROW = 3,
  DOWN_ARROW = 4,
  SPACE_BAR = 5,
} arrow_key_t;

/**
 * The possible types of key events.
 * Enum types in C are much more primitive than in Java; this is equivalent to:
 * typedef unsigned int KeyEventType;
 * #define KEY_PRESSED 0
 * #define KEY_RELEASED 1
 */
typedef enum { KEY_PRESSED, KEY_RELEASED } key_event_type_t;

/**
 * A keypress handler.
 * When a key is pressed or released, the handler is passed its char value.
 * Most keys are passed as their char value, e.g. 'a', '1', or '\r'.
 * Arrow keys have the special values listed above.
 *
 * @param key a character indicating which key was pressed
 * @param type the type of key event (KEY_PRESSED or KEY_RELEASED)
 * @param held_time if a press event, the time the key has been held in seconds
 */
typedef void (*key_handler_t)(char key, key_event_type_t type, double held_time,
                              void *state);
/**
 * A mouse handler.
 * When a mouse button event occurs, the handler is passed the type of event
 that
 * happens, whether it is up or down, and the x and y coordinates which this
 * occured at and the number of cliks, and a void *state so that it can edit the
 * state according to the mouse press.
 *
 * @param x the x-location the mouse has been pressed down
 * @param y the y-location the mouse has been pressed down
 */
typedef void (*mouse_handler_t)(state_t *state, double x, double y);
/**
 * Initializes the SDL window and renderer.
 * Must be called once before any of the other SDL functions.
 *
 * @param min the x and y coordinates of the bottom left of the scene
 * @param max the x and y coordinates of the top right of the scene
 */
void sdl_init(vector_t min, vector_t max);

/**
 * Processes all SDL events and returns whether the window has been closed.
 * This function must be called in order to handle inputs.
 *
 * @return true if the window was closed, false otherwise
 */
bool sdl_is_done(void *state);

/**
 * Clears the screen. Should be called before drawing polygons in each frame.
 */
void sdl_clear(void);

/**
 * Draws a polygon from the given list of vertices and a color.
 *
 * @param poly a struct representing the polygon
 * @param color the color used to fill in the polygon
 */
void sdl_draw_polygon(polygon_t *poly, rgb_color_t color);

/**
 * Displays the rendered frame on the SDL window.
 * Must be called after drawing the polygons in order to show them.
 */
void sdl_show(void);

/**
 * Draws all bodies in a scene.
 * This internally calls sdl_clear(), sdl_draw_polygon(), and sdl_show(),
 * so those functions should not be called directly.
 *
 * @param scene the scene to draw
 * @param aux an additional body to draw (can be NULL if no additional bodies)
 */
void sdl_render_scene(scene_t *scene, void *aux);

/**
 * Registers a function to be called every time a key is pressed.
 * Overwrites any existing handler.
 *
 * Example:
 * ```
 * void on_key(char key, key_event_type_t type, double held_time) {
 *     if (type == KEY_PRESSED) {
 *         switch (key) {
 *             case 'a':
 *                 printf("A pressed\n");
 *                 break;
 *             case UP_ARROW:
 *                 printf("UP pressed\n");
 *                 break;
 *         }
 *     }
 * }
 * int main(void) {
 *     sdl_on_key(on_key);
 *     while (!sdl_is_done());
 * }
 * ```
 *
 * @param handler the function to call with each key press
 */
void sdl_on_key(key_handler_t handler);

/**
 * Registers a function to be called every time a key is pressed.
 * Overwrites any existing handler.

 @param handler the function to call when the mouse is clicked or released
 */
void sdl_on_click(mouse_handler_t handler);

/**
 * Gets the amount of time that has passed since the last time
 * this function was called, in seconds.
 *
 * @return the number of seconds that have elapsed
 */
double time_since_last_tick(void);

/**
 * Takes in a image path and loads the texture to return a SDL_Texture type.

 @param img_path

 @return the SDL_Texture pointer that now has the image loaded on that can be
 used to render the image
 */
SDL_Texture *make_img(const char *img_path);

/**
 * Loads the texture for the image and puts the texture where it is
 * wanted according to the x and y coordinates from loc and makes it the size
 given by the size vector on the rendered rectangle.

 @param img SDL_Texture * that can be used to render the image
 @param loc vector_t of the location on the window for the image
 @param size vector_t of the size of the image
 */
void sdl_draw_img(SDL_Texture *img, vector_t loc, vector_t size);

/**
 * Opens a font path and initializes it with a size. Returns the TTF_Font type.

 @param font_path where the font is located
 @param font_size
 */
TTF_Font *open_font(const char *font_path, int8_t font_size);

void sdl_play_music(char *path);

/**
 * Loads a text with some inputted font and puts the texture where it is
 * wanted on the rendered rectangle. Positions the rectangle on the window.

 @param text string of text that is to be displayed
 @param font
 @param vector_t of where the location should be on the window
 */
void make_text(char *text, TTF_Font *font, vector_t loc, size_t w, size_t h,
               rgb_color_t color);

SDL_Rect make_bounding_box(body_t *body);

#endif // #ifndef __SDL_WRAPPER_H__