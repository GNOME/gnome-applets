#include "asclock.h"

#define GETNEXT fread(&next, 1, sizeof(char), file); if(feof(file)) return FALSE

static char next;
static FILE *file;

static void buferr(void)
{
  fprintf(stderr, "please do not enter numbers bigger than the number of atoms in the universe. thanks\n");
  exit(-1);
}


int read_init(FILE *f)
{

  file = f;

  GETNEXT;

  return TRUE;
}

int read_type(int *type) {
  
  while(1) 
    {
      if(isalpha(next))
	return TRUE;

      GETNEXT;

      if(next=='*')
        while(1) {
	  GETNEXT;
          if(next!='*') continue;
          GETNEXT;
          if(next=='/') break;
        }
      else if(next=='/') 
        while(1) {
	  GETNEXT;
	  if(next=='\n') { GETNEXT; break; }
        }

      if(isalpha(next)) return TRUE;

      GETNEXT;
    }
  return TRUE;
}

int read_token(char *str, int max)
{
  int i;
  i = 0;

  while(isspace(next))
    GETNEXT;

  while( (isalnum(next) || (next=='_')) && (i<max)) 
    {
      str[i++] = next;
      GETNEXT;
    }

  if(i==max) buferr();

  str[i] = '\0';

  return TRUE;
}

int read_assign(void) 
{

  while(next!='=')
    GETNEXT;

  GETNEXT;
  
  return TRUE;
}

int read_int(int *ret)
{
  char buf[64];
  int i;

  while(isspace(next))
    GETNEXT;

  i = 0;
  while(isdigit(next) && (i<64)) {
    buf[i++]=next;
    GETNEXT;
  }

  if(i==64) buferr();

  buf[i] = '\0';

  *ret = atoi(buf);

  return TRUE;
}

int read_semicolon(void) 
{

  while(next!=';')
    GETNEXT;

  GETNEXT;

  return TRUE;
}
