#include "engine.h"
#include <bits/time.h>
#include <stdint.h>
#include <sys/epoll.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

//event 
InputContext * input_init()
{
  InputContext * ctx = malloc(sizeof(InputContext));

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
      Event evt;
      evt.data.key.key = buffer[i];
      if (buffer[i] == ESC) evt.type = EVENT_QUIT; 
      else evt.type = EVENT_KEY_PRESSED;
      evt.data.key.timestamp = ctx->frame_time_ms;
      evt.data.key.is_hold = 0;

      ctx->events_queue[ctx->events_count++] = evt;
    }
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

//draw
void clear_term()
{
  printf(CLEAR);
}

uint32_t ascii_to_uint(const char *a) 
{
  unsigned char *bytes = (unsigned char *)a;
  size_t length = strlen(a);
  uint32_t code = 0;
  for (int i = 0; i < length; i++)
  {
    code = code << 8 | bytes[i];
  }
  return code;
}

//TODO
// char * uint_to_ascii(uint32_t i) 
// {
//   buf * ;
//   size_t length = strlen(a);
//   uint32_t code = 0;
//   for (int i = 0; i < length; i++)
//   {
//     code = code << 8 | bytes[i];
//   }
//   return code;
// }
//

void draw(uint32_t **img, int heigth, int width, int x, int y)
{
  for(int i = 0; i < y; i++)
    write(1, (int *)NEWLINE, 1);
  for(int i = 0; i < x; i++)
    write(1, (int *)EMPTY, 1);
  write(1, &img[0][0], 1);
}
