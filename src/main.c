#include "engine.h"

int main()
{
  InputContext * input = input_init();
  bool running = true;
  Sprite cube = {
    .heigth = 2,
    .width = 2,
    .name = "cube",
    .x = 2,
    .y = 3,
  };
  cube.img = malloc(sizeof(UtfChar *) * cube.heigth);
  for (int i = 0; i < cube.heigth; i++)
  {
    cube.img[i] = malloc(sizeof(UtfChar) * cube.heigth);
  }
  cube.img[0][0] = get_utfchar("🍉");
  cube.img[0][1] = get_utfchar("🍉");
  cube.img[1][0] = get_utfchar("🍉");
  cube.img[1][1] = get_utfchar("🍉");

  while(running)
  {
    int event_count = input_poll(input);
    for (int i = 0; i < event_count; i++)
    {
      Event * evt = input_get_event(input, i);

      if(evt->type == EVENT_QUIT)
      {
        clear_term();
        running = false;
        break;
      }
      else if (evt->type == EVENT_KEY_PRESSED)
      {
        char key = evt->key.raw_char;
        uint64_t timestamp = evt->key.timestamp;
        // printf("key: %d, timestamp: %lu", key, timestamp);

        if (key == 'a') cube.x-=2;
        if (key == 'd') cube.x+=2;
      }
    }
    clear_term();
    draw_sprite(cube);
    usleep(1666);
  }
}
