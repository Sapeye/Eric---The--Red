#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "isolate.h"

#define NOTIFY_NEW_IP     0
#define NOTIFY_BAD_SERVER 1
#define NOTIFY_PANIC      2
#define NOTIFY_FILE_READ  3
#define NOTIFY_FILE_EXIST  4
#define NOTIFY_FILE_PERMISSION  5
#define NOTIFY_MEMORY_ALLOCATION  6
#define NOTIFY_BAD_INI  7


void Notify(int, char *);

extern int errno;

#define LINES_BUFFER 180

struct ini
{
  char web[3][80];
  char smtp[40];
  int smtp_port;
  int connection_security;
  char users_name[40];
  char users_password[40];
  char email_address[40];
  char subject_IP_address[80];
  char subject_bad_server[80];
  char last_ip[20];
  char retieved_ip[3][20];
  int content[12];
};

struct ini ini;

struct _lines_
{
  int line;
  char composed[LINES_BUFFER];
  char value[LINES_BUFFER];
};

void DeComposeLine(struct _lines_ *decom)
{
  char *p1, *p2;
  p1 = strchr(decom->composed, '\n');
  if(p1 != NULL)
    p1 = 0;
  decom->line = atoi(decom->composed);
  p1 = strchr(decom->composed, '=');
  if(p1 == NULL)
    decom->value[0] = 0;
  else
  {
    p1++;
    p2 = strchr(p1, '\n');
    if(p2 != NULL)
      *p2 = 0;
    sprintf(decom->value, "%s", p1);
  }
}


void IniFileRead(FILE *f)
{
  struct _lines_ decom;
  int read[12];
  char *got;
  memset((int *)read, 0, (sizeof(int) * 12));
  do
  {
    got = fgets(decom.composed, LINES_BUFFER, f);
    if(got != NULL)
    {
      DeComposeLine(&decom);
      // initialize as each is read from ini file
      switch(decom.line)
      {
        case 1:
          strncpy(ini.web[0], decom.value, 79);
          ini.web[0][79] = 0;
          break;
        case 2:
          strncpy(ini.web[1], decom.value, 79);
          ini.web[1][79] = 0;
          break;
        case 3:
          strncpy(ini.web[2], decom.value, 79);
          ini.web[2][79] = 0;
          break;
        case 4:
          strncpy(ini.smtp, decom.value, 39);
          ini.smtp[39] = 0;
          break;
        case 5:
          ini.smtp_port = atoi(decom.value);
          break;
        case 6:
          ini.connection_security = atoi(decom.value);
          break;
        case 7:
          strncpy(ini.users_name, decom.value, 39);
          ini.users_name[39] = 0;
          break;
        case 8:
          strncpy(ini.users_password, decom.value, 39);
          ini.users_password[39] = 0;
          break;
        case 9:
          strncpy(ini.email_address, decom.value, 39);
          ini.email_address[39] = 0;
          break;
        case 10:
          strncpy(ini.subject_IP_address, decom.value, 79);
          ini.subject_IP_address[79] = 0;
          break;
        case 11:
          strncpy(ini.subject_bad_server, decom.value, 79);
          ini.subject_bad_server[79] = 0;
          break;
        case 12:
          strncpy(ini.last_ip, decom.value, 19);
          ini.last_ip[19] = 0;
          break;
      }
      if(strlen(decom.value))
        ini.content[decom.line - 1] = 1;
      else
        ini.content[decom.line - 1] = 0;
    }
    else
      break;
  }
  while(1);
  if(f != NULL)
    fclose(f);
}


void ReadIniFile(void)
{
  FILE *f;
  char path[128];

  f = fopen("c:/eric-the-red/eric-the-red.ini", "r");
  strcpy(path, "c:/eric-the-red/eric-the-red.ini");

  if(f == NULL)
  {
    if(errno == ENOENT)
      Notify(NOTIFY_FILE_EXIST, path);

    else if(errno == EACCES)
      Notify(NOTIFY_FILE_PERMISSION, path);
    exit(0);
  }
// the file is opened successfully the file can be read in
  IniFileRead(f);
}


void CheckIniFileContent(void)
{
  int loop;
  int got = 0;
  for(loop = 0; loop < 12; loop++)
  {
    if(ini.content[loop])
      got++;
  }
  if(got == 12)
    return;
  exit(0);
}


void  UpdateIni(char *new_ip)
{
  char com[160];
  int loop;

  FILE *f = fopen("c:/eric-the-red/eric-the-red.ini", "w");

  if(f == NULL)
  {
    Notify(NOTIFY_FILE_READ, "eric-the-red.ini");
    exit(0);
  }
  for(loop = 1; loop <= 12; ++loop)
  {
    switch(loop)
    {
      case 1:
        sprintf(com, "%d=%s\n", loop, ini.web[0]);
        break;
      case 2:
        sprintf(com, "%d=%s\n", loop, ini.web[1]);
        break;
      case 3:
        sprintf(com, "%d=%s\n", loop, ini.web[2]);
        break;
      case 4:
        sprintf(com, "%d=%s\n", loop, ini.smtp);
        break;
      case 5:
        sprintf(com, "%d=%d\n", loop, ini.smtp_port);
        break;
      case 6:
        sprintf(com, "%d=%d\n", loop, ini.connection_security);
        break;
      case 7:
        sprintf(com, "%d=%s\n", loop, ini.users_name);
        break;
      case 8:
        sprintf(com, "%d=%s\n", loop, ini.users_password);
        break;
      case 9:
        sprintf(com, "%d=%s\n", loop, ini.email_address);
        break;
      case 10:
        sprintf(com, "%d=%s\n", loop, ini.subject_IP_address);
        break;
      case 11:
        sprintf(com, "%d=%s\n", loop, ini.subject_bad_server);
        break;
      case 12:
        sprintf(com, "%d=%s\n", loop, new_ip);
        break;
    }
    fputs(com, f);
  }
  fclose(f);
}


void Notify(int reason, char *who)
{
  char *subject;
  char msg[256];
  char cmd_ln[512];
  switch(reason)
  {
    case NOTIFY_NEW_IP:
      {
        subject = ini.subject_IP_address;
        sprintf(msg, "The new ip address is %s", who);
      }
      break;
    case NOTIFY_BAD_SERVER:
      {
        subject = ini.subject_bad_server;
        sprintf(msg,
            "The web address \"%s\" is giving bad information. Find a new web server to resolve the IP and "
            "make a new \"eric-the-red.ini\" with \"EricTheRedConfig\" to replace this one. "
            "Then upload to the remote site.", who);
      }
      break;
    case NOTIFY_PANIC:
      {
        subject = "PANIC PANIC PANIC";
        sprintf(msg,
            "All three web server addresses are giving different and conflicting information."
            "Find 3 new web servers to resolve IPs and make a new \"eric-the-red.ini\" with \"EricTheRedConfig\""
            "to replace this one. Then upload to the remote site.");
        break;
      }
    case NOTIFY_FILE_READ:
      {
        subject = "PANIC EricTheRed PANIC";
        sprintf(msg,
            "Could not open \"%s\" for reading,\n"
            "This is a fatal error and \"EricTheRedWorker\" cannot continue.", who);
        break;
      }
    case NOTIFY_FILE_EXIST:
      {
        sprintf(msg, "The file %s does not exist.\n"
            "This is a fatal error and \"EricTheRedWorker\" cannot continue.", who);
        subject = "PANIC EricTheRed PANIC";
      }
    case NOTIFY_FILE_PERMISSION:
      {
        sprintf(msg, "You do not have permission to access the file %s.\n"
            "This is a fatal error and \"EricTheRedWorker\" cannot continue.", who);
        subject = "PANIC EricTheRed PANIC";
      }
    case NOTIFY_MEMORY_ALLOCATION:
      {
        sprintf(msg, "A memory allocation failure has occoured.\n"
            "This is a fatal error and \"EricTheRedWorker\" cannot continue.");
        subject = "PANIC EricTheRed has run out of memory";
      }
    case NOTIFY_BAD_INI:
      {
        sprintf(msg, "\"eric-the-red.ini\" has values missing and needs to be rebuilt.\n"
            "Run \"EricTheRedConfig\" to correct this.\n"
            "Then try \"EricTheRedWorker\" again.");
        subject = "PANIC EricTheRed ini file problems";
      }
    default:
      break;
  }

  /* -server <server_name>' and '-f <your_email_address> */
  sprintf(cmd_ln,
      "%sblat.exe -server %s -port %d -to %s -f %s  -i %s -subject \"%s\" -body \"%s\"",
      "c:/eric-the-red/", ini.smtp, ini.smtp_port, ini.email_address, ini.email_address,
      ini.email_address, subject, msg);
  if(ini.connection_security)
  {
    strcat(cmd_ln, "-u ");
    strcat(cmd_ln, ini.users_name);
    strcat(cmd_ln, "-pw ");
    strcat(cmd_ln, ini.users_password);
  }
  system(cmd_ln);
}


void GetWebPages(void)
{
  int val;
  int item;
  int aa, bb, cc, dd;
  char str[512];
  for(val = 0; val < 3; ++val)
  {
    memset(str, 0, sizeof(str));

    sprintf(str, "%s/wget %s -O %s%d.htm", "c:/eric-the-red", ini.web[val], "c:/eric-the-red/", val);
    if(system(str))
    {
      Notify(NOTIFY_BAD_SERVER, ini.web[val]);
      exit(0);
    }
    sprintf(str, "%s%d.htm", "c:/eric-the-red/", val);

    item = IPIsolate(str, &aa, &bb, &cc, &dd);
    unlink(str);
    if(item == -1001)
    {
      Notify(NOTIFY_MEMORY_ALLOCATION, NULL);
      exit(0);
    }
    if(item == -1000)
    {
// failed to open file
      Notify(NOTIFY_BAD_SERVER, ini.web[val]);
      exit(0);
    }
    if(!item)
//success
      sprintf(ini.retieved_ip[val], "%d.%d.%d.%d", aa, bb, cc, dd);
  }
}


int main(void)
{
  int val;
  ReadIniFile();
  CheckIniFileContent();
  GetWebPages();
  if(!strcmp(ini.retieved_ip[0], ini.retieved_ip[1]) &&
      !strcmp(ini.retieved_ip[1], ini.retieved_ip[2]))
  {
    // all match, all is good
    if(!strcmp(ini.retieved_ip[0], ini.last_ip))
      // nothing has changed, nothing to do
      return 0;
    else
    {
      Notify(NOTIFY_NEW_IP, ini.retieved_ip[0]);
      // update ini file and we are done
      UpdateIni(ini.retieved_ip[0]);
      return 0;
    }
  }
  if(strcmp(ini.retieved_ip[0], ini.retieved_ip[1]) &&
      strcmp(ini.retieved_ip[1], ini.retieved_ip[2]) &&
      strcmp(ini.retieved_ip[0], ini.retieved_ip[1]))
  {
    // no server returns the same ip
    Notify(NOTIFY_PANIC, NULL);
    return 0;
  }
  // one server is returning an odd ip
  if(strcmp(ini.retieved_ip[0], ini.retieved_ip[1]) &&
      strcmp(ini.retieved_ip[0], ini.retieved_ip[2]))
    // web[0] if the odd man out
  {
    val = 0;
    if(strcmp(ini.retieved_ip[2], ini.retieved_ip[1]) &&
        strcmp(ini.retieved_ip[2], ini.retieved_ip[0]))
      // web[2] is the odd man out
      val = 2;
    else
      // by elimination web[1] is the odd man out
      val = 1;
    Notify(NOTIFY_BAD_SERVER, ini.web[val]);
    // update ini file and we are done
    ++val; // find a good ip to use
    if(val > 2)
      val = 0;
    Notify(NOTIFY_NEW_IP, ini.retieved_ip[val]);
    UpdateIni(ini.retieved_ip[val]);
    return 0;
  }
  return 0;
}
