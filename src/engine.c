#include "engine.h"
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

//event 
InputContext * input_init()
{
  InputContext * ctx = malloc(sizeof(InputContext));
  if (!ctx) return NULL;

  tcgetattr(STDIN_FILENO, &ctx->old_term);
  struct termios new_term = ctx->old_term;
  new_term.c_lflag &= ~ICANON;
  new_term.c_lflag &= ~ECHO;
  new_term.c_cc[VMIN] = 0;
  new_term.c_cc[VTIME] = 0;
  tcsetattr(STDIN_FILENO, TCSANOW, &new_term);

  ctx->epoll_fd = epoll_create1(0);
  if (ctx->epoll_fd == -1) {
    perror("epoll_create1");
    free(ctx);
    return NULL;
  }

  struct epoll_event ev;
  ev.events = EPOLLIN;
  ev.data.fd = STDIN_FILENO;
  epoll_ctl(ctx->epoll_fd, EPOLL_CTL_ADD, STDIN_FILENO, &ev);
  ctx->events_count = 0;
  ctx->frame_time_ms = 0;

  ctx->initialized = true;
  return ctx;
}

static uint64_t get_time_ms()
{
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (ts.tv_sec * 1000) + (ts.tv_nsec / 1000000);
}

void input_capture_keys(InputContext * ctx)
{
  ctx->events_count = 0;
  ctx->frame_time_ms = get_time_ms();

  if(ctx->escape_buffer_len > 0 && 
      (ctx->frame_time_ms - ctx->escape_last_ms) > 50) 
  {
    ParsedKey key = parse_single_key(27);
    if(key.type != KEY_NORMAL || key.raw_byte != 0) {
      input_add_event(ctx, EVENT_KEY_PRESSED, key);
    }
  }

  int n = epoll_wait(ctx->epoll_fd, ctx->events, MAX_EVENTS, 0);
  if(n < 0)
  {
    perror("epoll_wait");
    return;
  }

  if(n > 0)
  {
    char buffer[MAX_EVENTS];
    int bytes = read(STDIN_FILENO, buffer, MAX_EVENTS);

    for (int i = 0; i < bytes && ctx->events_count < MAX_EVENTS; i++)
    {
      char byte = buffer[i];

      if (byte == 27) {
        ctx->escape_buffer[0] = byte;
        ctx->escape_buffer_len = 1;
        ctx->escape_last_ms = ctx->frame_time_ms;
        continue;
      }

      if(ctx->escape_buffer_len > 0)
      {
        ctx->escape_buffer[ctx->escape_buffer_len++] = byte;
        ctx->escape_last_ms = ctx->frame_time_ms;
        if(is_escape_complete(ctx->escape_buffer, ctx->escape_buffer_len))
        {
          ParsedKey key = parse_escape_sequence(ctx->escape_buffer, ctx->escape_buffer_len);
          input_add_event(ctx, EVENT_KEY_PRESSED, key);
          ctx->escape_buffer_len = 0; 
          continue;
        }
        continue;
      }
      ParsedKey key = parse_single_key(byte);
      input_add_event(ctx, EVENT_KEY_PRESSED, key);
    }
  }
}

void input_add_event(InputContext * ctx, EventType type, ParsedKey key)
{
  if (ctx->events_count >= MAX_EVENTS) return;
  Event evt = {0};
  evt.type = type;
  evt.key.type = key.type;
  evt.key.raw_char = key.raw_byte;
  evt.key.timestamp = ctx->frame_time_ms;

  ctx->events_queue[ctx->events_count++] = evt;

  if(type == EVENT_KEY_PRESSED) {
    ctx->key_state[key.raw_byte] = 1;
  }
}

int input_poll(InputContext * ctx)
{
  input_capture_keys(ctx);
  return ctx->events_count;
}

Event * input_get_event(InputContext * ctx, int index)
{
  if (index < 0 || index >= ctx->events_count)
    return NULL;
  return &ctx->events_queue[index];
}

void input_clear_events(InputContext* ctx) {
    ctx->events_count = 0;
}

void close_events(InputContext * ctx)
{
  if(!ctx) return;
  close(ctx->epoll_fd);
  tcsetattr(STDIN_FILENO, TCSANOW, &ctx->old_term);
  free(ctx);
}

void input_shutdown(InputContext *ctx)
{
  close(ctx->epoll_fd);
  tcsetattr(STDIN_FILENO, TCSANOW, &ctx->old_term);
  free(ctx);
}

void detect_key_releases(InputContext* ctx)
{
  for (int i = 0; i < 250; i++) {
    if(ctx->key_state[i])
    {
      Event evt =  {
        .type = EVENT_KEY_RELEASED,
        .key.type = i,
        .key.raw_char = i,
        .key.timestamp = ctx->frame_time_ms
      };

      if(ctx->events_count < MAX_EVENTS) {
        ctx->events_queue[ctx->events_count++] = evt;
      }
      ctx->key_state[i] = 0;
    }
  }
}

/* ESCAPE PARSING */
ParsedKey parse_single_key(char byte)
{
  ParsedKey key = {.type = KEY_NORMAL, .raw_byte = byte};
  if(byte == 23) key.type = KEY_NORMAL;
  else if(byte == 127 || byte == 8) key.type = KEY_BACKSPACE;
  else if(byte == 13 || byte == 10) key.type = KEY_ENTER;
  else if(byte == 9) key.type = KEY_TAB;
  else if(byte >= 32 && byte <= 126) key.type = KEY_NORMAL;
  return key;
}

ParsedKey parse_escape_sequence(const char * seq, int len)
{
  ParsedKey key = {.type = KEY_NORMAL, .raw_byte = 0};
  if (len >= 3 && seq[0] == 27 && seq[1] == '[') 
  {
    switch (seq[2]) {
      case 'A': key.type = KEY_ARROW_UP;    break;
      case 'B': key.type = KEY_ARROW_DOWN;  break;
      case 'C': key.type = KEY_ARROW_RIGHT; break;
      case 'D': key.type = KEY_ARROW_LEFT;  break;
      default:  key.type = KEY_NORMAL;      break;
    }
  }

  return key;
}

int is_escape_complete(const char* buffer, int len)
{
  if (len < 3) return 0;

  if (buffer[0] == 27 && buffer[1] == '[') 
  {
    if (buffer[2] >= 'A' && buffer[2] <= 'Z') return 1;
    if (buffer[2] >= 'a' && buffer[2] <= 'z') return 1;
  }
  
  return 0;
}

/* DRAW */
void clear_term()
{
  printf("%s\n", CLEAR);
}

void draw(UtfChar **img, int heigth, int width, int x, int y)
{
  for(int i = 0; i < y; i++)
    write(1, "\n", 1);
  for (int h = 0; h < heigth; h++) 
  {
    for(int i = 0; i < x; i++)
      write(1, SPACE, 1);

    for (int w = 0; w < width; w++)
    {
      write(1, img[h][w].utf8, img[h][w].length);
    }
    write(1, "\n", 1);
  }
}

void draw_sprite(Sprite sprite)
{
  UtfChar ** img = sprite.img;
  int x = sprite.x;
  int y = sprite.y;
  int width = sprite.width;
  int heigth = sprite.heigth;
  for(int i = 0; i < y; i++)
    write(1, "\n", 1);
  for (int h = 0; h < heigth; h++) 
  {
    for(int i = 0; i < x; i++)
      write(1, SPACE, 1);

    for (int w = 0; w < width; w++)
    {
      write(1, img[h][w].utf8, img[h][w].length);
    }
    write(1, "\n", 1);
  }

}

UtfChar get_utfchar(char * s){
  UtfChar utfchar;
  utfchar.utf8 = s;
  utfchar.length = strlen(s);
  return utfchar;
}


