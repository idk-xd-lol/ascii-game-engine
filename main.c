#ifndef ENGINE_H
#define ENGINE_H

#include <stdio.h>
#include <stdlib.h>

#define CLEAR "\e[1;1H\e[2J"
#define SCRIPTS_BUCKET 32 

struct Script;

typedef struct 
{
  char * name;
  int heigth, width;
  int x, y;
  int ** img;
  int ** col_mask;
  bool has_colison;
  Script * scripts;
} Sprite;

typedef struct Script {
  char * key;
  char * type;
  union {int ival; float fval; char cval; char * sval;} value;
  struct Script *next;
} Script;

typedef struct
{
  Script *buckets[SCRIPTS_BUCKET];
  int count;
} ScriptMap;


#endif // !ENGINE_H

