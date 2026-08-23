#include "engine.h"
#include <stdlib.h>
#include <unistd.h>

int main()
{
  InputContext * input = input_init();
  bool running = true;
  Sprite cube = {
    .heigth = 1,
    .width = 1,
    .name = "cube",
  };
  cube.img = malloc(sizeof(int *) * cube.heigth);
  while(running)
  {
    int event_count = input_poll(input);
    for (int i = 0; i < event_count; i++)
    {
      Event * evt = input_get_event(input, i);

      if (evt->type == EVENT_KEY_PRESSED)
      {
        char key = evt->data.key.key;
        uint64_t timestamp = evt->data.key.timestamp;
        // printf("key: %d, timestamp: %lu", key, timestamp);

        if (key == 'a') printf("left");
        if (key == 'd') printf("right");
      }
      else if(evt->type == EVENT_QUIT) running = false;
    }
    usleep(166666);
    clear_term();
  }
}
