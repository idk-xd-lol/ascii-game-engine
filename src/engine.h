#ifndef ENGINE_H
#define ENGINE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>

#include <sys/epoll.h>
#include <unistd.h>
#include <termios.h>

#define SCRIPTS_BUCKET 32 
#define MAX_EVENTS 256

#define CLEAR "\e[1;1H\e[2J"

/* SPRITES  */
typedef struct Script {
  char * key;
  char * type;
  union {int ival; float fval; char cval; char * sval;} value;
  struct Script *next;
} Script;

typedef struct {
  char * name;
  int heigth, width;
  int x, y;
  int ** img;
  bool has_colison;
  int ** col_mask;
  Script * scripts;
} Sprite;

typedef struct {
  Script *buckets[SCRIPTS_BUCKET];
  int count;
} ScriptMap;

/* EVENTS */
typedef enum {
  EVENT_KEY_PRESSED,
  EVENT_KEY_RELEASED,
  EVENT_QUIT,
} EventType;

typedef struct {
  EventType type;
  int key;
  uint64_t timestamp;
  bool is_hold;
} KeyEvent;

typedef struct {
  EventType type;
  union {KeyEvent key; } data;
} Event;


typedef struct {
  int epoll_fd;
  struct epoll_event events[10];
  struct termios old_term;
  bool initialized;

  Event events_queue[MAX_EVENTS];
  int events_count;
  uint64_t frame_time_ms;
} InputContext;


/* FUNCTIONS */

//draw
void draw(char *img, float x, float y);
void draw_sprite(Sprite * sprite);
void clear_term();

//events
static uint64_t get_time_ms();
InputContext * input_init();
void input_capture_keys(InputContext * ctx);
void get_input_events(InputContext * ctx);
void close_events(InputContext * ctx);

#endif // !ENGINE_H
