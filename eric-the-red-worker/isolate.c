#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>

static int MySscanf(char *buffer, int *a, int *b, int *c, int *d)
{
  // count the dots
  char *p1, *p2, *p3; //, *p4;
  p1 = strchr(buffer, '.');
  if(p1 == NULL)
    return 0; // no dot
  p2 = strchr(p1 + 1, '.');
  if(p2 == NULL)
    return 0; // no dot
  p3 = strchr(p2 + 1, '.');
  if(p2 == NULL)
    return 0; // no dot

  *a = atoi(buffer);
  *b = atoi(p1 + 1);
  *c = atoi(p2 + 1);
  *d = atoi(p3 + 1);

  return 4;
}


static char* FindFirstDigit(char *buffer)
{
  char *ptr = buffer;
  while(*ptr > 0 && !isdigit((int)*ptr))
    ptr++;
  return ptr;
}

static char* FindFirstNonIP(char *buffer)
{
  char *ptr = buffer;
  while((int)*ptr > 0 && (isdigit((int) *ptr) || (int)*ptr == '.'))
    ptr++;
  return ptr;
}

int IPIsolate(char *file_name, int *a, int *b, int *c, int *d)
{
  FILE *in_file;
  char *buffer, *ptr1, *ptr2;
  int val1, val2, ptr_val;
  in_file = fopen(file_name, "r");

  if(in_file == NULL)
    return -1000; // download file again in gui. or abort with email for worker.

  // load file into buffer
  fseek(in_file, 0, SEEK_END);
  val1 = ftell(in_file) + 10;
  buffer = (char *)malloc(val1);
  if(buffer == NULL)
    return -1001;
  fseek(in_file, 0, 0);
  val2 = fread(buffer, 1, val1, in_file);
  fclose(in_file);
  // all good
  ptr1 = buffer;
NEXT_TRY:
  ptr1 = FindFirstDigit(ptr1);
  if(ptr1 != NULL)
  {
    ptr2 = FindFirstNonIP(ptr1);
    if(ptr2 != NULL)
    {
      ptr_val = (int) * ptr2;
      *ptr2 = 0;
      if(ptr1 != NULL)
        val1 = MySscanf(ptr1, a, b, c, d);
      else
        return -1002; // cant isolate an IP
      *ptr2 = (char)ptr_val;
      ptr1 = ptr2;
      if(val1 < 4)
        goto NEXT_TRY;
      if(*a < 1 || *a > 255 || *a == 10)
        goto NEXT_TRY;
      if((*a == 192 && *b == 168) || ( *a == 172 && *b == 16))
        goto NEXT_TRY;

      return 0; // got an IP
    }
    else
      return -1002; // cant isolate an IP
  }
  else
    return -1002; // cant isolate an IP
}
