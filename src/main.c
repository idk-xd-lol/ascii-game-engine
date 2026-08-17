#include "engine.h"
#include <unistd.h>

int main()
{
  InputContext * context = input_init();
  InputQueue * event = malloc(sizeof(InputQueue));
  char ch = 0;
  while(1)
  {
    get_input_events(context, event);
    if (event)
    {
      printf("")
    }
    usleep(1666666);
    clear_term();
  }
}
