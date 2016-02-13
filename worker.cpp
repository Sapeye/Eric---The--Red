#include "main.h"
#include <FL/fl_ask.H>
#include "isolate.h"
#include "inifile.h"
#include <ctype.h>
#include <malloc.h>

extern int h_errno;
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>


/*  linux errors for gethostbyname
**
**  HOST_NOT_FOUND
**      The specified host is unknown.
**  NO_ADDRESS or NO_DATA
**      The requested name is valid but does not have an IP address.
**  NO_RECOVERY
**      A nonrecoverable name server error occurred.
**  TRY_AGAIN
**      A temporary error occurred on an authoritative name server. Try again later.
*/

/* windows errors for gethostbyname
**  WSAENETDOWN
**     The network subsystem has failed.
**  WSAHOST_NOT_FOUND
**     Authoritative answer host not found.
**  WSATRY_AGAIN
**     Nonauthoritative host not found, or server failure.
**  WSANO_RECOVERY
**     A nonrecoverable error occurred.
**  WSANO_DATA
**     The requested name is valid, but no data of the requested type was found.
**     This error is also returned if the name parameter contains a string representation
**     of an IPv6 address or an illegal IPv4 address.
**     This error should not be interpreted to mean that the name parameter contains a
**     name string that has been validated for a particular protocol (an IP hostname, for example).
**     Since Winsock supports multiple name service providers, a name may potentially be valid for
**     one provider and not accepted by another provider.
**  WSAEINPROGRESS
**     A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.
**  WSAEFAULT
**     The name parameter is not a valid part of the user address space.
**  WSAEINTR
**     A blocking Windows Socket 1.1 call was canceled through WSACancelBlockingCall.
*/

/* declarations  */
/* this lot keeps score for test purposes, 0 = not passed and 1 = passed */

static int smtp_test = 0;
static int port_test = 0;
static int username_test = 0;
static int password_test = 0;
static int subject_ip = 0;
static int subject_web = 0;
static int email_addy = 0;
static int failed_continue = 0;

/* END of this lot keeps score for test purposes */

static char *IP_array[3];

static void StripWhiteSpace(Fl_Input *);

/* END declarations  */

#define _PATH_ "c:/eric-the-red/"

static void WsaStart(void)
{
  int result;
  WSADATA wsaData;
  result = WSAStartup(MAKEWORD(1, 1), &wsaData);
  if(result != 0)
  {
    result = fl_choice("Something has gone wrong starting the TCPIP, Windows will do that sometimes and\n"
                       "the best option is to exit EricTheRed and reboot Windows and try again.\n"
                       "You could check to see if your web browser still works and if it does, try again (only try once).\n"
                       "You could also abandon checking and copy and past a known good SMTP from your email client\n"
                       "or some other verified source (use ctl + v or right mouse click to paste).",
                       "Reboot", /* 0 */
                       "Try again", /* 1 */
                       "Abandon, I can do it" /* 2 */);
    if(result == 0)
      exit(0);
    else if(result == 2)
    {
      failed_continue = 1;
      return;
    }
    WsaStart(); // recursion, can flood the stack if hit to many times, user is warned to use just once
  }
}

static void WsaEnd(void)
{
  WSACleanup();
}


void  NetworkError(void)
{
  int tmp = WSAGetLastError();
  if(tmp)
  {
    switch(tmp)
    {
      case WSAHOST_NOT_FOUND:
        tmp = 0;
        break;
      case WSANO_DATA:  // disconnected cat 5
        tmp = 1;
        break;
      case WSANO_RECOVERY:
        tmp = 2;
        break;
      case WSATRY_AGAIN:
        tmp = 3;
        break;
      default: // catchall else
        tmp = 4;
    }
  }
  /*
    "The specified host is unknown.",  -- HOST_NOT_FOUND and WSAHOST_NOT_FOUND
    "The requested name is valid but does not have an IP address.",  1 -- NO_ADDRESS or NO_DATA and WSANO_DATA
    "A nonrecoverable name server error occurred.", 2 -- NO_RECOVERY and WSANO_RECOVERY
    "A temporary error occurred on an authoritative name server. Try again later."  3 -- TRY_AGAIN or WSATRY_AGAIN
  */

}


void MemoryAllocationFail(void)
{
  fl_alert("Memory allocation failed, must shut down.");
  exit(0);
}

static void SplitUserData(Fl_Widget *o, unsigned int *upper, unsigned int *lower)
{
  unsigned int user_data = (unsigned int)o->user_data();
  unsigned int high, low;
  if(!user_data)
  {
    *upper = *lower = 0;
    return;
  }
  high = ((user_data & 0xffff0000) >> 16);
  low = user_data & 0x0000ffff;
  *upper = high;
  *lower = low;
}


int main(void)
{
  char *ini_path;
  WsaStart();
  atexit(WsaEnd);
  IP_array[0] = (char *)malloc(24);
  IP_array[1] = (char *)malloc(24);
  IP_array[2] = (char *)malloc(24);
  if(IP_array[0] != NULL && IP_array[1] != NULL && IP_array[2] != NULL)
  {
    IP_array[0][0] = 0;
    IP_array[1][0] = 0;
    IP_array[2][0] = 0;
  }
  else
    MemoryAllocationFail();
  Fl::scheme("GTK+");
  main_window = MainWindow();
  help_window = HelpWindow();
  // read in from ini file
  ini_path = (char *)malloc(128);
  if(ini_path == NULL)
    MemoryAllocationFail();
  sprintf(ini_path, "%seric-the-red.ini", _PATH_);
  ReadIniFile(ini_path);
  free(ini_path);
  main_window->show();
  Fl::run();
}


void ShowGroupCB(Fl_Button *o, void *u)
{
  unsigned int u_data, high_word;
  help_window->hide();
  SplitUserData(o, &high_word, &u_data);
  for(int loop = 0; loop < 4; ++loop)
  {
    button[loop]->labelcolor(FL_BLACK);
    group[loop]->hide();
  }
  // hide help button
  if(u_data < 3)
    help_button->activate();
  else
    help_button->deactivate();
  button[u_data]->labelcolor((Fl_Color)59);
  group[u_data]->show();
  if(o == button[3]) // the results of the IP extraction from web pages
  {
    if(!strcmp((const char*)IP_array[0], (const char*)IP_array[1]) &&
        !strcmp((const char*)IP_array[0], (const char*)IP_array[2]))
      save_done->activate();
    else if(strcmp((const char*)IP_array[0], (const char*)IP_array[1]) ||
            strcmp((const char*)IP_array[0], (const char*)IP_array[2]) ||
            strcmp((const char*)IP_array[1], (const char*)IP_array[2]))
      save_done->deactivate();
    else if(strcmp((const char*)IP_array[0], (const char*)IP_array[1]) &&
            strcmp((const char*)IP_array[0], (const char*)IP_array[2]) &&
            strcmp((const char*)IP_array[1], (const char*)IP_array[2]))
      save_done->deactivate();
    resolved[0]->value(IP_array[0]);
    resolved[1]->value(IP_array[1]);
    resolved[2]->value(IP_array[2]);
  }
  if(u_data == 1)
  {
    smtp->do_callback();
    smtp_port->do_callback();
    users_name->do_callback();
    users_password->do_callback();
    if(smtp_test == 1)
    {
      sad->hide();
      happy->show();
    }
    else
    {
      happy->hide();
      sad->show();
    }
  }
  else if(u_data == 2)
  {
    email_address->do_callback();
    subject_IP_address->do_callback();
    subject_bad_server->do_callback();
  }
}


void DownloadTest(Fl_Button * o, void *u)
{
  unsigned int item, high_word;
  SplitUserData(o, &high_word, &item);
  char str[180];
  char str2[16];
  StripWhiteSpace(web[item]);
  strcpy(str, web[item]->value());
  if(str[4]  == 's' || str[4]  == 'S')
  {
    fl_choice("Do not use a web address with \"htpps://\",\n"
              "if the security certificate is invalid\n"
              "\"wget\" complains and will not download.",
              "OK", NULL, NULL);
    web[item]->take_focus();
    return;
  }
  sprintf(str, "%swget %s -O %s%d.htm", _PATH_, web[item]->value(), _PATH_, item);
  int val = system(str);
  if(!val)
  {
    if(item < 2)
    {
      web[item + 1]->activate();
      test[item + 1]->activate();
    }
    else
      button[1]->activate();
    sprintf(str, "%s%d.htm", _PATH_, item);
    int aa, bb, cc, dd;
    val = IPIsolate(str, &aa, &bb, &cc, &dd);
    if(val == -1001)
      MemoryAllocationFail();
    else if(val == -1000)
    {
      // failed to open file
      fl_alert("Download test failed, edit the address.");
      web[item]->take_focus();
      return;
    }
    if(!val)
    {
      //success
      sprintf(IP_array[item], "%d.%d.%d.%d", aa, bb, cc, dd);
      // hide every thing
      go[0]->hide();
      go[1]->hide();
      go[2]->hide();
      stop[0]->hide();
      stop[1]->hide();
      stop[2]->hide();
      amber[0]->hide();
      amber[1]->hide();
      amber[2]->hide();
      // 3 ip retrieved
      if((strlen(IP_array[0]) && strlen(IP_array[1]) && strlen(IP_array[2])))
      {
        // all different ip
        if(strcmp(IP_array[0], IP_array[1]) && strcmp(IP_array[0], IP_array[2]) && strcmp(IP_array[1], IP_array[2]))
        {
          stop[0]->show();
          stop[1]->show();
          stop[2]->show();
          goto ExIT;
        }
        // one odd man out, third man
        if(strcmp(IP_array[0], IP_array[1]) == 0 && strcmp(IP_array[0], IP_array[2]))
        {
          stop[2]->show();
          go[0]->show();
          go[1]->show();
          goto ExIT;
        }
        // one odd man out, second man
        if(strcmp(IP_array[0], IP_array[2]) == 0 && strcmp(IP_array[0], IP_array[1]))
        {
          stop[1]->show();
          go[0]->show();
          go[2]->show();
          goto ExIT;
        }
        // one odd man out, first man
        if(strcmp(IP_array[0], IP_array[1]) && strcmp(IP_array[1], IP_array[2]) == 0)
        {
          stop[2]->show();
          go[0]->show();
          go[1]->show();
          goto ExIT;
        }
        else // so all ip must be the same
        {
          go[0]->show();
          go[1]->show();
          go[2]->show();
          goto ExIT;
        }
      }
      // end of all different ip
      // one ip retrieved, this can only be with the second and third man not retrieved
      if(strlen(IP_array[0]) && strlen(IP_array[1]) == 0 && strlen(IP_array[2]) == 0)
      {
        go[0]->show();
        goto ExIT;
      }
      // end of one ip retrieved
      // 2 ip retrieved, this can only be with the third man not retrieved
      else if(strlen(IP_array[0]) && strlen(IP_array[1]) && strlen(IP_array[2]) == 0)
      {
        if(!strcmp(IP_array[0], IP_array[1]))
        {
          go[0]->show();
          go[1]->show();
        }
        else
        {
          go[0]->show();
          amber[1]->show();
        }
        goto ExIT;
      }
      // end one ip retrieved
    }
ExIT:
    unlink(str);
    return;
  }
}


void QuitCB(Fl_Button * o, void *u)
{
  exit(0);
}


void SmtpValid(void)
{
  if((smtp_test || failed_continue) && port_test)
  {
    if(password_group->visible())
    {
      if(username_test && password_test)
        goto GOOD;
      else
        goto BAD;
    }
    else
      goto GOOD;
  }
  else
    goto BAD;
GOOD:
  button[2]->activate();
  return;
BAD:
  button[2]->deactivate();
}
void ConnectionSecutityCB(Fl_Choice * o, void *u)
{
  if(o->value() == 0)
    password_group->hide();
  else
    password_group->show();
  SmtpValid();
}


static const char *codes[] = {".ac", ".ad", ".ae", ".af", ".ag", ".ai", ".al", ".am", ".ao", ".aq", ".ar", ".as", ".at", ".au",
                              ".aw", ".ax", ".az", ".ba", ".bb", ".bd", ".be", ".bf", ".bg", ".bh", ".bi", ".bj", ".bl", ".bm",
                              ".bn", ".bo", ".bq", ".br", ".bs", ".bt", ".bv", ".bw", ".by", ".bz", ".ca", ".cc", ".cd", ".cf",
                              ".cg", ".ch", ".ci", ".ck", ".cl", ".cm", ".cn", ".co", ".cr", ".cu", ".cv", ".cw", ".cx", ".cy",
                              ".ec", ".ee", ".eg", ".eh", ".er", ".es", ".et", ".eu", ".fj", ".fi", ".fk", ".fm", ".fo", ".fr",
                              ".ga", ".uk", ".gd", ".ge", ".gf", ".gg", ".gh", ".gi", ".gl", ".gm", ".gn", ".gp", ".gq", ".gr",
                              ".gs", ".gt", ".gu", ".gw", ".gy", ".hk", ".hm", ".hn", ".hr", ".ht", ".hu", ".id", ".ie", ".il",
                              ".im", ".in", ".io", ".iq", ".ir", ".is", ".it", ".je", ".jm", ".jo", ".jp", ".ke", ".kg", ".kh",
                              ".ki", ".km", ".kn", ".kp", ".kr", ".kw", ".ky", ".kz", ".la", ".lb", ".lc", ".li", ".lk", ".lr",
                              ".ls", ".lt", ".lu", ".lv", ".ly", ".ma", ".mc", ".md", ".me", ".mg", ".mh", ".mk", ".ml", ".ml",
                              ".mm", ".mn", ".mo", ".mp", ".mq", ".mr", ".ms", ".mt", ".mu", ".mv", ".mw", ".mx", ".my", ".mz",
                              ".na", ".nc", ".ne", ".nf", ".ng", ".ni", ".nl", ".no", ".np", ".nr", ".nu", ".nz", ".om", ".pa",
                              ".pe", ".pf", ".pg", ".ph", ".pk", ".pl", ".pm", ".pn", ".pr", ".ps", ".pt", ".pw", ".py", ".qa",
                              ".re", ".ro", ".rs", ".ru", ".rw", ".sa", ".sb", ".sc", ".sd", ".se", ".sg", ".sh", ".si", ".sj",
                              ".sk", ".sl", ".sm", ".sn", ".so", ".sr", ".ss", ".st", ".sv", ".sx", ".sy", ".sz", ".tc", ".td",
                              ".tf", ".tg", ".th", ".tj", ".tk", ".tl", ".tm", ".tn", ".to", ".tr", ".tt", ".tv", ".tw", ".tz",
                              ".ua", ".ug", ".um", ".us", ".uy", ".uz", ".va", ".vc", ".ve", ".vg", ".vi", ".vn", ".vu", ".wf",
                              ".ws", ".ye", ".yt", ".za", ".zm", ".zw", ".com", ".net", ".org", ".edu", ".eu", ".biz", ".gov",
                              ".mil", ".krd" ".info", NULL

                             };


void TCPIPError(int number)
{
  switch(number)
  {
    case 0:
      break;
    case 1:
      {
        fl_alert("The requested name is valid but does not have an IP address.");
        smtp_test = 0;
        break;
      }
    case 2:
      {
        fl_alert("A nonrecoverable name server (DNS) error occurred.\r"
                 "There is nothing wrong at your end, You could try using\n"
                 "a different name server in your TCP/IP set up.");
        smtp_test = 0;
        break;
      }
    case 3:
      {
        fl_alert("A temporary error occurred on an authoritative name server. Try again later.\n"
                 "There is nothing wrong at your end but you cannot continue until the other end is fixed.");
        smtp_test = 0;
        break;
      }
    case 4:
      fl_alert("An unknown error has has occurred.\nRe booting your computer may help.");
      smtp_test = 0;
      break;
  }
}


unsigned long LookupAddress(const char* host)
{
  unsigned long remote_addr;
  hostent* ptr_hostent = gethostbyname(host);
  if(ptr_hostent == 0)
    return INADDR_NONE;
  remote_addr = *((unsigned long*)ptr_hostent->h_addr_list[0]);
  return remote_addr;
}


void TestSmtpCB(Fl_Input *o, void *u)
{
  char *ptr1, *ptr2;
  StripWhiteSpace(o);
  if(failed_continue == 1) // the user is responsible to get it right so do not test
    return;
  unsigned long lookup_result;
  // test to see if there is enough to stand a chance to test ok
  int tmp;
  ptr1 = (char *)strrchr(o->value(), '.');
  if(ptr1 == NULL)
    return;
  tmp = strlen(ptr1);
  if(tmp < 3) // to short
    return;
  if(tmp > 5) // to long
    return;
  tmp = 0;
  while(codes[tmp] != NULL)
  {
    char str[128];
    strcpy(str, o->value());
    ptr1 = (char *)strrchr(str, '.');
    strlwr(str);
    if(strstr(str, codes[tmp]) == ptr1)
      goto MATCH;
NEXT_ITERATION:
    tmp++;
  }
  smtp_test = 0;
  happy->hide();
  sad->show();
  return;
MATCH:
  lookup_result = LookupAddress(o->value());
  if(lookup_result != INADDR_NONE)
  {
    smtp_test = 1;
    if(smtp_test == 1)
    {
      sad->hide();
      happy->show();
    }
  }
  else
  {
    smtp_test = 0;
    happy->hide();
    sad->show();
  }
  NetworkError();
  SmtpValid();
}


void SmtpServerPortCB(Fl_Int_Input * o, void *u)
{
  StripWhiteSpace(o);
  int port = atoi(o->value());
  if(port)
    port_test = 1;
  else
    port_test = 0;
  SmtpValid();
}


void UsersNameCB(Fl_Input * o, void *u)
{
  if(strlen(o->value()) > 3)
    username_test = 1;
  else
    username_test = 0;
  SmtpValid();
}

void UsersPasswordCB(Fl_Input * o, void *u)
{
  if(strlen(o->value()) > 3)
    password_test = 1;
  else
    password_test = 0;
  SmtpValid();
}


static void TestEmailValid()
{
  if(subject_ip && subject_web && email_addy)
    button[3]->activate();
  else
    button[3]->deactivate();
}


void TestEmailAddress(Fl_Input * o, void *u)
{
  char *ptr;
  if((ptr = (char *)strchr(o->value(), '@')) != NULL)
  {
    if(ptr - o->value() > 4)
    {
      if((ptr = strchr(ptr, '.')) != NULL)
      {
        if(strlen(ptr) > 2)
          email_addy = 1;
      }
    }
  }
  else
    email_addy =  0;
  TestEmailValid();
}


void HasContentCB(Fl_Input *o, void *u)
{
  StripWhiteSpace(o);
  if(strlen(o->value()) > 3)
  {
    if(o == subject_IP_address)
      subject_ip = 1;
    else
      subject_web = 1;
  }
  else
  {
    if(o == subject_IP_address)
      subject_ip = 0;
    else
      subject_web = 0;
  }
  TestEmailValid();
}


static void StripWhiteSpace(Fl_Input *o)
{
  char *str_end;
  char *str = alloca(strlen(o->value()));
  if(str == NULL)
    return;
  char *ptr = str;
  int c;
  strcpy(str, o->value());
  // Trim leading space
  while(isspace(*str))
    str++;
  if(*str == 0)  // All spaces?
  {
    o->value("");
    return;
  }
  // Trim trailing space
  str_end = str + strlen(str) - 1;
  while(str_end > str && isspace(*str_end))
    str_end--;
  // Write new null terminator
  *(str_end + 1) = 0;
  o->value(str);
}


void SaveIniFile(Fl_Button * o, void *u)
{
  char ini_path[128];
  sprintf(ini_path, "%seric-the-red.ini", _PATH_);
  WriteIniFile(ini_path);
  exit(0);
}


void NoDottedQuad(void)
{
  fl_choice("Do not use an IP address because at any time a web\n"
            "server can get a different IP address so we should use\n"
            "DNS to resolve the URL to it's IP address.",
            NULL, NULL, "OK");
}


void  NoHTPPS(void)
{
  fl_choice("\"wget\" does not like invalid security certificates\n"
            "and many web servers doing \"https\" are invalid, so\n"
            "we do not use them.",
            NULL, NULL, "OK");
}


void TestUrlValidCB(Fl_Input *o, void *u)
{
  int len, loop, a, b, c, d;
  unsigned int high_word, low_word;
  char str[128], *ptr1, *ptr2;
  SplitUserData(o, &high_word, &low_word);
  strcpy(str, o->value());
  ptr1 = strrchr(str, '.');
  if(ptr1 == NULL)
    goto NOT_VALID;
  ptr2 = strchr(ptr1, '/');
  if(ptr2 != NULL)
  {
    *ptr2 = 0;
    len = strlen(ptr1);
  }
  else
    len = strlen(ptr1);
  if(len < 3 || len > 5)
    goto NOT_VALID;
  else
  {
    loop = 0;
    strlwr(str);
    while(codes[loop] != NULL)
    {
      if(strstr(str, codes[loop]) == ptr1)
        goto GOOD_SO_FAR;
      loop++;
    }
    // reached the end and no match
    goto NOT_VALID;
  }
GOOD_SO_FAR:
  // test for a dotted quad
  strcpy(str, o->value());
  a = b = c = d = loop = 0;
  char *ptr;
  while(str[loop] != 0)
  {
    len = (int)str[loop];
    if(isdigit(len))
    {
      NoDottedQuad();
      goto NOT_VALID;
    }
  }
  // test for "https://"
  loop = strncasecmp(str, "https://", 8);
  if(!loop)
  {
    NoHTPPS();
    goto NOT_VALID;
  }
  // test for "http://"
  loop = strncasecmp(str, "http://", 7);
  if(loop)
    goto NOT_VALID;
  else
  {
    ptr1 += 6;
    if(LookupAddress(ptr1) == INADDR_NONE)
      goto NOT_VALID;
  }
  test[low_word]->activate();
  return;
NOT_VALID:
  test[low_word]->deactivate();
}


void HelpCB(Fl_Button * o, void *u)
{
  char where[128];
  for(int loop = 0; loop < 4; ++loop)
  {
    if(group[loop]->visible())
    {
      sprintf(where, "%s%d.html", _PATH_, loop);
      help_view->load(where);
      help_window->show();
      return;
    }
  }
}




/*
BLAT supports both AUTH PLAIN and AUTH LOGIN. The switches are:

-u <user-name>
-pw <password>
-plain
-body

*/
