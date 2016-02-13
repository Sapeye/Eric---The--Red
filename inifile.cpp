// read and writes eric-the-red.ini there linux version or windows version, it works with both.

// this is NOT a windows ini file but looks like one. all the read and write routines are here.

#include "main.h"
#include <FL/fl_ask.H>
#include <malloc.h>
#include <stdio.h>
#include <errno.h>
extern int errno;

#define LINES_BUFFER 180

extern void MemoryAllocationFail(void);

static struct _lines_
{
  int line;
  char composed[LINES_BUFFER];
  char value[LINES_BUFFER];
} _LINES_;

typedef struct _lines_ LINES;

static void ComposeLine(struct _lines_ *com)
{
  sprintf(com->composed, "%d=%s\n", com->line, com->value);
}

static void DeComposeLine(struct _lines_ *decom)
{
  char *p1, *p2;
  decom->line = atoi(decom->composed);
  p1 = strchr(decom->composed, '=');
  if(p1 == NULL)
    sprintf(decom->value, "");
  else
  {
    p1++;
    p2 = strchr(p1, '\n');
    if(p2 != NULL)
      *p2 = 0;
    sprintf(decom->value, "%s", p1);
  }
}


void WriteIniFile(char *ini_path)
{
  LINES *com;
  FILE *f;
  com = (struct _lines_ *)alloca(sizeof(LINES));
  f = fopen(ini_path, "w");
  if(f == NULL)
  {
    int a = errno;
    int b = errno;
  }
  for(int loop = 0; loop < 12; ++loop)
  {
    int a = loop + 1;
    switch(a)
    {
      case 1:
        strcpy(com->value, web[0]->value());
        break;
      case 2:
        strcpy(com->value, web[1]->value());
        break;
      case 3:
        strcpy(com->value, web[2]->value());
        break;
      case 4:
        strcpy(com->value, smtp->value());
        break;
      case 5:
        strcpy(com->value, smtp_port->value());
        break;
      case 6:
        if(connection_security->value() == 0)
          strcpy(com->value, "0");
        else
          strcpy(com->value, "1");
        break;
      case 7:
        strcpy(com->value, users_name->value());
        break;
      case 8:
        strcpy(com->value, users_password->value());
        break;
      case 9:
        strcpy(com->value, email_address->value());
        break;
      case 10:
        strcpy(com->value, subject_IP_address->value());
        break;
      case 11:
        strcpy(com->value, subject_bad_server->value());
        break;
      case 12:
        strcpy(com->value, "0.0.0.0");
        break;
    }
    com->line = a;
    ComposeLine(com);
    fputs(com->composed, f);
  }
  fclose(f);
}


static void CrashAndBurn()
{
  fl_choice("EricTheRedConfig has not been installed correctly.\n"
            "Install as documented in the readme file for the operating\n"
            "system you are using.\n\nEricTheRedConfig must shut down."
            , NULL, NULL, "OK");
  exit(0);
}


static void NotAllowed()
{
  fl_alert("You do not have required permission, fix it and try again.");
  exit(0);
}



void ReadIniFile(char *ini_path)
{
  LINES *decom;
  FILE *f;
  int read[12];
  char *got;
  memset((int *)read, 0, (sizeof(int) * 12));
  decom = (struct _lines_ *)alloca(sizeof(struct _lines_));
  if(decom == NULL)
    MemoryAllocationFail();
  f = fopen(ini_path, "r");
  if(f == NULL)
  {
    if(errno == ENOENT)
      goto GET_NEW_INI_SETTINGS;
   //   CrashAndBurn();
    if(errno == EACCES)
      NotAllowed();
  }
GET_NEW_INI_SETTINGS:
  do
  {
    got = fgets(decom->composed, LINES_BUFFER, f);
    if(got != NULL)
    {
      DeComposeLine(decom);
      // initialize as each is read from ini file
      switch(decom->line)
      {
        case 1:
          web[0]->value(decom->value);
          break;
        case 2:
          web[1]->value(decom->value);
          break;
        case 3:
          web[2]->value(decom->value);
          break;
        case 4:
          smtp->value(decom->value);
          break;
        case 5:
          smtp_port->value(decom->value);
          break;
        case 6:
          connection_security->value(atoi(decom->value));
          break;
        case 7:
          users_name->value(decom->value);
          break;
        case 8:
          users_password->value(decom->value);
          break;
        case 9:
          email_address->value(decom->value);
        case 10:
          subject_IP_address->value(decom->value);
          break;
        case 11:
          subject_bad_server->value(decom->value);
          break;
        default:
          break;
      }
      read[decom->line - 1] = 1;
    }
    else
      break;
  }
  while(1);
  if(f != NULL)
    fclose(f);
  // initialize any that were not read out from ini file
NO_INI_FILE:
  for(int loop = 0; loop < 12; ++loop)
  {
    if(read[loop] == 0)
    {
      int a = loop + 1;
      switch(a)
      {
        case 1:
          web[0]->value("http://www.ipchicken.com/");
          break;
        case 2:
          web[1]->value("http://whatismyip.org/");
          break;
        case 3:
          web[2]->value("http://www.yougetsignal.com/what-is-my-ip-address/");
          break;
        case 4:
          smtp->value("");
          break;
        case 5:
          smtp_port->value("25");
          break;
        case 6:
          connection_security->value(0);
          break;
        case 7:
          users_name->value("");
          break;
        case 8:
          users_password->value("");
          break;
        case 9:
          email_address->value("");
        case 10:
          subject_IP_address->value("ALERT your remote IP address has changed");
          break;
        case 11:
          subject_bad_server->value("ALARM one web server is not resolving your IP");
          break;
        default:
          break;
      }
    }
  }
  // do the necessary callbacks
  smtp_port->do_callback();
  smtp->do_callback();
  users_name->do_callback();
  users_password->do_callback();
  email_address->do_callback();
  subject_IP_address->do_callback();
  subject_bad_server->do_callback();
  connection_security->do_callback();
  button[1]->deactivate();
}
/*
1=web[0]
2=web[1]
3=web[2]
4=smtp
5=smtp_port
6=connection_security
7=users_name
8=users_password
9=email_address
10=subject_IP_address
11=subject_bad_server
12=last_IP_address
*/
