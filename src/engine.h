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
#include <bits/time.h>
#include <time.h>

#define SCRIPTS_BUCKET 32 
#define MAX_EVENTS 32
#define ESC '\033'
#define CLEAR "\033[1;1H\033[2J"
#define NEWLINE "\n"
#define SPACE " "
#define ESCAPE_BUFER_SIZE 8

/* SPRITES  */
typedef struct Script {
  char * key;
  char * type;
  union {int ival; float fval; char cval; char * sval;} value;
  struct Script *next;
} Script;

typedef  struct {
  char * utf8;
  size_t length;
} UtfChar;

typedef struct {
  char * name;
  int heigth, width;
  int x, y;
  UtfChar ** img;
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

typedef enum {
    KEY_TYPE_CHAR,
    KEY_TYPE_ARROW_UP,
    KEY_TYPE_ARROW_DOWN,
    KEY_TYPE_ARROW_LEFT,
    KEY_TYPE_ARROW_RIGHT,
    KEY_TYPE_ESC,
    KEY_TYPE_BACKSPACE,
    KEY_TYPE_ENTER,
    KEY_TYPE_TAB,
    KEY_TYPE_DELETE,
    KEY_TYPE_HOME,
    KEY_TYPE_END,
    KEY_TYPE_PAGEUP,
    KEY_TYPE_PAGEDOWN,
    KEY_TYPE_UNKNOWN
} KeyType;

typedef struct {
  KeyType key;
  char raw_char;
  uint64_t timestamp;
  bool is_hold;
} KeyEvent;

typedef struct {
  EventType type;
  KeyEvent key;
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

/*KEY PARSING */
typedef struct {
  char buffer[ESCAPE_BUFER_SIZE];
  int pos;
  uint64_t last_read_time_ms;
} EscapeParser;


/* FUNCTIONS */

//draw
UtfChar get_utfchar(char * s);
void draw(UtfChar **img, int heigth, int width, int x, int y);
void draw_sprite(Sprite sprite);
void clear_term();

//events
InputContext * input_init();
void input_capture_keys(InputContext * ctx);

Event * input_get_event(InputContext * ctx, int index);
int input_poll(InputContext * ctx);
void get_input_events(InputContext * ctx);
void close_events(InputContext * ctx);


#endif // !ENGINE_H
