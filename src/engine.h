#ifndef ENGINE_H
#define ENGINE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <inttypes.h>

#include <sys/epoll.h>
#include <unistd.h>
#include <termios.h>

#define SCRIPTS_BUCKET 32 
#define MAX_EVENTS 32
#define ESC '\033'
#define CLEAR "\e[1;1H\e[2J"
#define NEWLINE '\n'
#define EMPTY ' '

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
  uint32_t ** img;
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
  struct epoll_event events[MAX_EVENTS];
  struct termios old_term;
  bool initialized;

  Event events_queue[MAX_EVENTS];
  int events_count;
  uint64_t frame_time_ms;
} InputContext;


/* FUNCTIONS */

//draw
uint32_t ascii_to_uint(const char * a);
void draw(uint32_t **img, int heigth, int width, int x, int y);
void clear_term();

//events
InputContext * input_init();
void input_capture_keys(InputContext * ctx);

Event * input_get_event(InputContext * ctx, int index);
int input_poll(InputContext * ctx);
void get_input_events(InputContext * ctx);
void close_events(InputContext * ctx);


#endif // !ENGINE_H
