// generated by Fast Light User Interface Designer (fluid) version 1.0303

#ifndef main_h
#define main_h
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
extern Fl_Double_Window *main_window;
#include <FL/Fl_Button.H>
extern void ShowGroupCB(Fl_Button*, void*);
extern Fl_Button *button[4];
extern void SaveIniFile(Fl_Button*, void*);
extern Fl_Button *save_done;
extern void HelpCB(Fl_Button*, void*);
extern Fl_Button *help_button;
extern void QuitCB(Fl_Button*, void*);
#include <FL/Fl_Group.H>
#include <FL/Fl_Input.H>
extern void TestUrlValidCB(Fl_Input*, void*);
extern void DownloadTest(Fl_Button*, void*);
#include <FL/Fl_Box.H>
extern Fl_Input *web[3];
extern Fl_Button *test[3];
extern Fl_Box *go[3];
extern Fl_Box *stop[3];
extern Fl_Box *amber[3];
extern void TestSmtpCB(Fl_Input*, void*);
extern Fl_Input *smtp;
#include <FL/Fl_Int_Input.H>
extern void SmtpServerPortCB(Fl_Int_Input*, void*);
extern Fl_Int_Input *smtp_port;
extern Fl_Box *happy;
extern Fl_Box *sad;
#include <FL/Fl_Choice.H>
extern void ConnectionSecutityCB(Fl_Choice*, void*);
extern Fl_Choice *connection_security;
extern Fl_Group *password_group;
extern void UsersNameCB(Fl_Input*, void*);
extern Fl_Input *users_name;
extern void UsersPasswordCB(Fl_Input*, void*);
extern Fl_Input *users_password;
extern void TestEmailAddress(Fl_Input*, void*);
extern Fl_Input *email_address;
extern void HasContentCB(Fl_Input*, void*);
extern Fl_Input *subject_IP_address;
extern Fl_Input *subject_bad_server;
extern Fl_Group *group[4];
#include <FL/Fl_Output.H>
extern Fl_Output *resolved[3];
Fl_Double_Window* MainWindow(void);
extern Fl_Double_Window *help_window;
#include <FL/Fl_Help_View.H>
extern Fl_Help_View *help_view;
Fl_Double_Window* HelpWindow();
#define RIGHT_CLICK 65536
#endif