// Updated by Ownasaurus in June 2010

#include "kaillera_ui.h"

#include <windows.h>
#include "uihlp.h"

#include "resource.h"
#include "kailleraclient.h"
#include "common/nSTL.h"
#include "common/k_socket.h"
#include "common/nThread.h"
#include "common/nSettings.h"
#include <windowsx.h>
#include "stdio.h"

bool hosting;
bool KAILLERA_CORE_INITIALIZED = false;
int flash = 1;
int beep = 1;
int timestamp = 1;
int joinleave = 1;

HWND kaillera_sdlg_TXT_MSG;
HWND kaillera_sdlg_IDC_FLASH;
HWND kaillera_sdlg_IDC_BEEP;
extern HWND kaillera_ssdlg;
extern bool spoofing;
extern int spoofPing;

// Saves the window position AND join message
void SaveWindowPos(HWND hwnd)
{
	RECT rect;
	char msg[256];
	GetWindowRect(hwnd, &rect);
	nSettings::set_int("XPOS",(int)rect.left);
	nSettings::set_int("YPOS",(int)rect.top);
	nSettings::set_int("WIDTH",(int)rect.right-(int)rect.left);
	nSettings::set_int("HEIGHT",(int)rect.bottom-(int)rect.top);
	if(GetWindowText(kaillera_sdlg_TXT_MSG,msg,256))
		nSettings::set_str("JOINMSG",msg);
	else nSettings::set_str("JOINMSG","");
}

// Loads the window position AND join message
void LoadWindowPos(HWND hwnd)
{
	RECT rect;
	GetWindowRect(hwnd, &rect);
	int xpos = nSettings::get_int("XPOS");
	int ypos = nSettings::get_int("YPOS");
	int width = nSettings::get_int("WIDTH");
	int height = nSettings::get_int("HEIGHT");
	
	UINT flags = SWP_SHOWWINDOW;

	if(xpos == -1 || ypos == -1) // ignore x,y
		flags |= SWP_NOREPOSITION;
	if(width == -1 || height == -1) // ignore width,height
		flags |= SWP_NOSIZE;

	char msg[256];
	nSettings::get_str("JOINMSG",msg);
	if(msg)
		SetWindowText(kaillera_sdlg_TXT_MSG,msg);

	SetWindowPos(hwnd,HWND_TOP,xpos,ypos,width,height,flags);
}

// Loads the options in the options menu
// Passing NULL to this function is just a rigged way to
// only load the server-wide settings.  These options should 
// eventually be moved to another options menu and be controlled
// by separate functions.
void LoadOptions(HWND hwnd = NULL)
{
	if(hwnd == NULL)
	{
		flash = nSettings::get_int("FLASH");
		beep = nSettings::get_int("BEEP");
		timestamp = nSettings::get_int("TIMESTAMP");
		joinleave = nSettings::get_int("JOINLEAVE");

		if(flash == -1) flash = 0;
		if(beep == -1) beep = 1;
		if(timestamp == -1) timestamp = 1;
		if(joinleave == -1) joinleave = 1;

		return;
	}

	char str[128];
	int maxplayers = nSettings::get_int("MAXPLAYERS");
	int maxping = nSettings::get_int("MAXPING");
	int samedelay = nSettings::get_int("SAMEDELAY");

	if(maxplayers == -1) maxplayers = 4;
	sprintf(str,"%d",maxplayers);
	SetWindowText(GetDlgItem(hwnd,IDC_MAXPLAYERS),str);

	if(maxping == -1) maxping = 999;
	sprintf(str,"%d",maxping);
	SetWindowText(GetDlgItem(hwnd,IDC_MAXPING),str);

	if(samedelay == -1) samedelay = 0;
	HWND kaillera_samedelay = GetDlgItem(hwnd,IDC_SAMEDELAY);
	SendMessage(kaillera_samedelay, CB_ADDSTRING, 0, (WPARAM)"False");
	SendMessage(kaillera_samedelay, CB_ADDSTRING, 0, (WPARAM)"True");
	SendMessage(kaillera_samedelay, CB_SETCURSEL, samedelay, 0);

	if(flash == -1) flash = 0;
	SendMessage(kaillera_sdlg_IDC_FLASH, CB_ADDSTRING, 0, (WPARAM)"False");
	SendMessage(kaillera_sdlg_IDC_FLASH, CB_ADDSTRING, 0, (WPARAM)"True");
	SendMessage(kaillera_sdlg_IDC_FLASH, CB_SETCURSEL, flash, 0);

	if(beep == -1) beep = 1;
	SendMessage(kaillera_sdlg_IDC_BEEP, CB_ADDSTRING, 0, (WPARAM)"False");
	SendMessage(kaillera_sdlg_IDC_BEEP, CB_ADDSTRING, 0, (WPARAM)"True");
	SendMessage(kaillera_sdlg_IDC_BEEP, CB_SETCURSEL, beep, 0);

	if(timestamp == -1) timestamp = 1;
	SendMessage(GetDlgItem(hwnd,IDC_TIMESTAMP), CB_ADDSTRING, 0, (WPARAM)"False");
	SendMessage(GetDlgItem(hwnd,IDC_TIMESTAMP), CB_ADDSTRING, 0, (WPARAM)"True");
	SendMessage(GetDlgItem(hwnd,IDC_TIMESTAMP), CB_SETCURSEL, timestamp, 0);

	if(joinleave == -1) joinleave = 1;
	SendMessage(GetDlgItem(hwnd,IDC_JOINLEAVE), CB_ADDSTRING, 0, (WPARAM)"False");
	SendMessage(GetDlgItem(hwnd,IDC_JOINLEAVE), CB_ADDSTRING, 0, (WPARAM)"True");
	SendMessage(GetDlgItem(hwnd,IDC_JOINLEAVE), CB_SETCURSEL, joinleave, 0);
}

// Saves the options in the options menu
void SaveOptions(HWND hwnd)
{
	char str[128];

	GetWindowText(GetDlgItem(hwnd,IDC_MAXPLAYERS),str,sizeof(str));
	int maxplayers = atoi(str);

	GetWindowText(GetDlgItem(hwnd,IDC_MAXPING),str,sizeof(str));
	int maxping = atoi(str);

	int samedelay = SendMessage(GetDlgItem(hwnd,IDC_SAMEDELAY), CB_GETCURSEL, 0, 0);
	flash = SendMessage(kaillera_sdlg_IDC_FLASH, CB_GETCURSEL, 0, 0);
	beep = SendMessage(kaillera_sdlg_IDC_BEEP, CB_GETCURSEL, 0, 0);
	timestamp = SendMessage(GetDlgItem(hwnd,IDC_TIMESTAMP), CB_GETCURSEL, 0, 0);
	joinleave = SendMessage(GetDlgItem(hwnd,IDC_JOINLEAVE), CB_GETCURSEL, 0, 0);

	nSettings::set_int("MAXPLAYERS",maxplayers);
	nSettings::set_int("MAXPING",maxping);
	nSettings::set_int("SAMEDELAY",samedelay);
	nSettings::set_int("FLASH",flash);
	nSettings::set_int("BEEP",beep);
	nSettings::set_int("TIMESTAMP",timestamp);
	nSettings::set_int("JOINLEAVE",joinleave);
}

// "Executes" the options in the options menu
void ExecuteOptions()
{
	if(hosting)
	{
		char str[128];
		int maxplayers = nSettings::get_int("MAXPLAYERS");
		int maxping = nSettings::get_int("MAXPING");
		int samedelay = nSettings::get_int("SAMEDELAY");

		if(maxplayers != -1)
		{
			sprintf(str,"/maxusers %d",maxplayers);
			kaillera_game_chat_send(str);
		}

		if(maxping != -1)
		{
			sprintf(str,"/maxping %d",maxping);
			kaillera_game_chat_send(str);
		}

		if(samedelay == 1)
			kaillera_game_chat_send("/samedelay true");
	}

	flash = nSettings::get_int("FLASH");
	beep = nSettings::get_int("BEEP");
	timestamp = nSettings::get_int("TIMESTAMP");
	joinleave = nSettings::get_int("JOINLEAVE");
}


bool kaillera_SelectServerDlgStep(){
	if (KAILLERA_CORE_INITIALIZED) {
		kaillera_step();
		return true;
	}
	Sleep(100);
	return true;
	return false;
}

char * CONNECTION_TYPES [] = 
{
	"",
	"LAN",
	"Exc",
	"Good",
	"Avg",
	"Low",
	"Bad"
};

char * USER_STATUS [] =
{
	"Playing",
	"Idle",
	"Playing"
};
char * GAME_STATUS [] = 
	{
		"Waiting",
		"Playing",
		"Playing",
	};

//===========================================================================
//===========================================================================
//===========================================================================
//===========================================================================
HWND kaillera_sdlg;
char kaillera_sdlg_NAME[128];
int kaillera_sdlg_port;
char kaillera_sdlg_ip[128];
nLVw kaillera_sdlg_gameslv;
nLVw kaillera_sdlg_userslv;
HWND kaillera_sdlg_partchat;

HWND kaillera_sdlg_CHK_REC;
HWND kaillera_sdlg_RE_GCHAT;
HWND kaillera_sdlg_TXT_GINP;
nLVw kaillera_sdlg_LV_GULIST;
HWND kaillera_sdlg_BTN_START;
HWND kaillera_sdlg_BTN_DROP;
HWND kaillera_sdlg_BTN_LEAVE;
HWND kaillera_sdlg_BTN_KICK;
HWND kaillera_sdlg_ST_SPEED;
//HWND kaillera_sdlg_ST_DELAY; // removed this because I can!
HWND kaillera_sdlg_BTN_GCHAT;
HWND kaillera_sdlg_BTN_LAGSTAT;
HWND kaillera_sdlg_BTN_LAGRESET;
HWND kaillera_sdlg_BTN_OPTIONS;
HWND kaillera_sdlg_BTN_ADVERTISE;
UINT_PTR kaillera_sdlg_sipd_timer;
int kaillera_sdlg_frameno = 0;
int kaillera_sdlg_delay = -1;

bool workaround[] = {false,false};

//=======================================================================
bool kaillera_RecordingEnabled(){
	return SendMessage(GetDlgItem(kaillera_sdlg, CHK_REC), BM_GETCHECK, 0, 0)==BST_CHECKED;
}
int kaillera_sdlg_MODE;
bool kaillera_sdlg_toggle = false;

// Called when you join or create a game
// The toggle flag "true" is passed when the "Swap" button is pressed
void kaillera_sdlgGameMode(bool toggle = false){

	if(!toggle)
	{
		kaillera_sdlg_MODE = 0;
		SetWindowText(GetDlgItem(kaillera_sdlg,IDC_CREATE),"Swap");
	}
	else kaillera_sdlg_toggle = false;

	ShowWindow(kaillera_sdlg_gameslv.handle,SW_HIDE);
	ShowWindow(kaillera_sdlg_CHK_REC,SW_SHOW);
	ShowWindow(kaillera_sdlg_RE_GCHAT,SW_SHOW);
	ShowWindow(kaillera_sdlg_TXT_GINP,SW_SHOW);
	ShowWindow(kaillera_sdlg_LV_GULIST.handle,SW_SHOW);
	ShowWindow(kaillera_sdlg_BTN_START,SW_SHOW);
	ShowWindow(kaillera_sdlg_BTN_DROP,SW_SHOW);
	ShowWindow(kaillera_sdlg_BTN_LEAVE,SW_SHOW);
	ShowWindow(kaillera_sdlg_BTN_KICK,SW_SHOW);
	ShowWindow(kaillera_sdlg_ST_SPEED,SW_SHOW);
	//ShowWindow(kaillera_sdlg_ST_DELAY,SW_SHOW);
	ShowWindow(kaillera_sdlg_BTN_OPTIONS,SW_SHOW);
	ShowWindow(kaillera_sdlg_BTN_ADVERTISE,SW_SHOW);
	ShowWindow(kaillera_sdlg_BTN_GCHAT,SW_SHOW);
	ShowWindow(kaillera_sdlg_BTN_LAGSTAT,SW_SHOW);
	ShowWindow(kaillera_sdlg_BTN_LAGRESET,SW_SHOW);
	ShowWindow(kaillera_sdlg_TXT_MSG,SW_SHOW);

	//EnableWindow(GetDlgItem(kaillera_sdlg,IDC_CREATE),false);
}

// Called when you leave a game
// The toggle flag "true" is passed when the "Swap" button is pressed
void kaillera_sdlgNormalMode(bool toggle = false){

	if(!toggle)
	{
		kaillera_sdlg_MODE = 1;
		hosting = false;
		SetWindowText(GetDlgItem(kaillera_sdlg,IDC_CREATE),"Create");
	}
	else kaillera_sdlg_toggle = true;

	ShowWindow(kaillera_sdlg_CHK_REC,SW_HIDE);
	ShowWindow(kaillera_sdlg_RE_GCHAT,SW_HIDE);
	ShowWindow(kaillera_sdlg_TXT_GINP,SW_HIDE);
	ShowWindow(kaillera_sdlg_LV_GULIST.handle,SW_HIDE);
	ShowWindow(kaillera_sdlg_BTN_START,SW_HIDE);
	ShowWindow(kaillera_sdlg_BTN_DROP,SW_HIDE);
	ShowWindow(kaillera_sdlg_BTN_LEAVE,SW_HIDE);
	ShowWindow(kaillera_sdlg_BTN_KICK,SW_HIDE);
	ShowWindow(kaillera_sdlg_ST_SPEED,SW_HIDE);
	//ShowWindow(kaillera_sdlg_ST_DELAY,SW_HIDE);
	ShowWindow(kaillera_sdlg_BTN_OPTIONS,SW_HIDE);
	ShowWindow(kaillera_sdlg_BTN_ADVERTISE,SW_HIDE);
	ShowWindow(kaillera_sdlg_BTN_GCHAT,SW_HIDE);
	ShowWindow(kaillera_sdlg_BTN_LAGSTAT,SW_HIDE);
	ShowWindow(kaillera_sdlg_BTN_LAGRESET,SW_HIDE);
	ShowWindow(kaillera_sdlg_TXT_MSG,SW_HIDE);
	ShowWindow(kaillera_sdlg_gameslv.handle,SW_SHOW);

	//EnableWindow(GetDlgItem(kaillera_sdlg,IDC_CREATE),true);
}
//===================================

int kaillera_sdlg_gameslvColumn;

// 1 represents a string type in the corresponding column
// 0 represents an integer type in the corresponding column
int kaillera_sdlg_gameslvColumnTypes[7] = {1, 0, 1, 1, 1, 0, 0};

// This controlls the column sorting information
int kaillera_sdlg_gameslvColumnOrder[7];

// Original comparator function
/*int CALLBACK kaillera_sdlg_gameslvCompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort){

	int ind1 = kaillera_sdlg_gameslv.Find(lParam1);
	int ind2 = kaillera_sdlg_gameslv.Find(lParam2);
	if (ind1 == -1 || ind2 == -1)
		return 0;

	char ItemText1[128];
	char ItemText2[128];

	
	kaillera_sdlg_gameslv.CheckRow(ItemText1, 128, lParamSort, ind1);
	kaillera_sdlg_gameslv.CheckRow(ItemText2, 128, lParamSort, ind2);

	if (kaillera_sdlg_gameslvColumnTypes[lParamSort]) {
		if (kaillera_sdlg_gameslvColumnOrder[lParamSort])
			return strcmp(ItemText1, ItemText2);
		else
			return -1*strcmp(ItemText1, ItemText2);
	} else {
		ind1 = atoi(ItemText1);
		ind2 = atoi(ItemText2);

		if (kaillera_sdlg_gameslvColumnOrder[lParamSort])
			return (ind1==ind2? 0 : (ind1>ind2? 1 : -1));
		else
			return (ind1==ind2? 0 : (ind1>ind2? -1 : 1));

	}
}*/

// Owna's comparator function
// Modified to try and always have newer games on top
// Done by assuming the highest game ID is the newest game
int CALLBACK kaillera_sdlg_gameslvCompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort){

	int ind1 = kaillera_sdlg_gameslv.Find(lParam1);
	int ind2 = kaillera_sdlg_gameslv.Find(lParam2);
	if (ind1 == -1 || ind2 == -1)
		return 0;

	char ItemText1[128];
	char ItemText2[128];

	
	kaillera_sdlg_gameslv.CheckRow(ItemText1, 128, lParamSort, ind1);
	kaillera_sdlg_gameslv.CheckRow(ItemText2, 128, lParamSort, ind2);

	if (kaillera_sdlg_gameslvColumnTypes[lParamSort]) { // string compare
		int tempresult = strcmp(ItemText1, ItemText2);
		if(tempresult == 0) // the strings are equal
		{
			// let's keep it ordered by game ID then
			kaillera_sdlg_gameslv.CheckRow(ItemText1, 128, 1, ind1);
			kaillera_sdlg_gameslv.CheckRow(ItemText2, 128, 1, ind2);

			ind1 = atoi(ItemText1);
			ind2 = atoi(ItemText2);

			if (kaillera_sdlg_gameslvColumnOrder[1]) // gameID column order
				return (ind1==ind2? 0 : (ind1>ind2? 1 : -1));
			else
				return (ind1==ind2? 0 : (ind1>ind2? -1 : 1));
		}

		if (kaillera_sdlg_gameslvColumnOrder[lParamSort])
			return strcmp(ItemText1, ItemText2);
		else
			return -1*strcmp(ItemText1, ItemText2);
	} else { // number compare
		ind1 = atoi(ItemText1);
		ind2 = atoi(ItemText2);

		if (kaillera_sdlg_gameslvColumnOrder[lParamSort])
			return (ind1==ind2? 0 : (ind1>ind2? 1 : -1));
		else
			return (ind1==ind2? 0 : (ind1>ind2? -1 : 1));

	}
}

void kaillera_sdlg_gameslvReSort() {
	ListView_SortItems(kaillera_sdlg_gameslv.handle, kaillera_sdlg_gameslvCompareFunc, kaillera_sdlg_gameslvColumn);
}

void kaillera_sdlg_gameslvSort(int column) {
	kaillera_sdlg_gameslvColumn = column;

	if (kaillera_sdlg_gameslvColumnOrder[column])
		kaillera_sdlg_gameslvColumnOrder[column] = 0;
	else
		kaillera_sdlg_gameslvColumnOrder[column] = 5;

	kaillera_sdlg_gameslvReSort();
}
//========================

//**Please see above for information on what the following variables signify**

int kaillera_sdlg_userslvColumn;
int kaillera_sdlg_userslvColumnTypes[7] = {1, 0, 0, 1, 1, 0, 0};
int kaillera_sdlg_userslvColumnOrder[7];


int CALLBACK kaillera_sdlg_userslvCompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort){
	int ind1 = kaillera_sdlg_userslv.Find(lParam1);
	int ind2 = kaillera_sdlg_userslv.Find(lParam2);
	if (ind1 == -1 || ind2 == -1)
		return 0;
	char ItemText1[128];
	char ItemText2[128];
	kaillera_sdlg_userslv.CheckRow(ItemText1, 128, lParamSort, ind1);
	kaillera_sdlg_userslv.CheckRow(ItemText2, 128, lParamSort, ind2);
	if (kaillera_sdlg_userslvColumnTypes[lParamSort]) {
		if (kaillera_sdlg_userslvColumnOrder[lParamSort])
			return strcmp(ItemText1, ItemText2);
		else
			return -1* strcmp(ItemText1, ItemText2);
	} else {
		ind1 = atoi(ItemText1);
		ind2 = atoi(ItemText2);
		if (kaillera_sdlg_userslvColumnOrder[lParamSort])
			return (ind1==ind2? 0 : (ind1>ind2? 1 : -1));
		else
			return (ind1==ind2? 0 : (ind1>ind2? -1 : 1));
	}
}

void kaillera_sdlg_userslvReSort() {
	ListView_SortItems(kaillera_sdlg_userslv.handle, kaillera_sdlg_userslvCompareFunc, kaillera_sdlg_userslvColumn);
}

void kaillera_sdlg_userslvSort(int column) {
	kaillera_sdlg_userslvColumn = column;

	if (kaillera_sdlg_userslvColumnOrder[column])
		kaillera_sdlg_userslvColumnOrder[column] = 0;
	else
		kaillera_sdlg_userslvColumnOrder[column] = 5;

	kaillera_sdlg_userslvReSort();
}

//====================

void kaillera_outp(char * line){
	re_append(kaillera_sdlg_partchat, line, 0);
}

//void kaillera_goutp(char * line, COLORREF color = 1) REQ: FOOLIN
void kaillera_goutp(char * line, COLORREF color=1){
	re_append(kaillera_sdlg_RE_GCHAT, line, color);
}

// The following functions can be modified if you want to change the colors
// TODO: Allow the user to customize the colors!

void __cdecl kaillera_gdebug(char * arg_0, ...) {
	char V8[1024];
	char V88[2084];
	sprintf_s(V8, 1020, "%s\r\n", arg_0);
	va_list args;
	va_start (args, arg_0);
	vsnprintf_s(V88, 2084, 2082, V8, args);
	va_end (args);
	kaillera_goutp(V88);
}

void __cdecl kaillera_gdebug_green(char * arg_0, ...) {
	char V8[1024];
	char V88[2084];
	sprintf_s(V8, 1020, "%s\r\n", arg_0);
	va_list args;
	va_start (args, arg_0);
	vsnprintf_s(V88, 2084, 2082, V8, args);
	va_end (args);
	kaillera_goutp(V88, 0x00009900);
}

void __cdecl kaillera_gdebug_gray(char * arg_0, ...) {
	char V8[1024];
	char V88[2084];
	sprintf_s(V8, 1020, "%s\r\n", arg_0);
	va_list args;
	va_start (args, arg_0);
	vsnprintf_s(V88, 2084, 2082, V8, args);
	va_end (args);
	kaillera_goutp(V88, 0x660000);
}

void __cdecl kaillera_core_debug(char * arg_0, ...) {
	char V8[1024];
	char V88[2084];
	sprintf_s(V8, 1020, "%s\r\n", arg_0);
	va_list args;
	va_start (args, arg_0);
	vsnprintf_s(V88, 2084, 2082, V8, args);
	va_end (args);
	re_append(kaillera_sdlg_partchat, V88, 0x33333333);
}
void __cdecl kaillera_ui_motd(char * arg_0, ...) {
	char V8[1024];
	char V88[2084];
	sprintf_s(V8, 1020, "%s\r\n", arg_0);
	va_list args;
	va_start (args, arg_0);
	vsnprintf_s(V88, 2084, 2082, V8, args);
	va_end (args);
	re_append(kaillera_sdlg_partchat, V88, 0x00336633);
}
void __cdecl kaillera_error_callback(char * arg_0, ...) {
	char V8[1024];
	char V88[2084];
	sprintf_s(V8, 1020, "%s\r\n", arg_0);
	va_list args;
	va_start (args, arg_0);
	vsnprintf_s(V88, 2084, 2082, V8, args);
	va_end (args);

	re_append(kaillera_sdlg_partchat, V88, 0x000000FF);
}

void __cdecl kaillera_ui_debug(char * arg_0, ...) {
	char V8[1024];
	char V88[2084];
	sprintf_s(V8, 1020, "%s\r\n", arg_0);
	va_list args;
	va_start (args, arg_0);
	vsnprintf_s(V88, 2084, 2082, V8, args);
	va_end (args);

	re_append(kaillera_sdlg_partchat, V88, /*0x00777777*/0x660000);
}

void __cdecl kaillera_outpf(char * arg_0, ...) {
	char V8[1024];
	char V88[2084];
	sprintf_s(V8, 1020, "%s\r\n", arg_0);
	va_list args;
	va_start (args, arg_0);
	vsnprintf_s(V88, 2084, 2082, V8, args);
	va_end (args);

	re_append(kaillera_sdlg_partchat, V88, 0x00000000);
}


void kaillera_user_add_callback(char*name, int ping, int status, unsigned short id, char conn){
	char bfx[500];
	int x;
	kaillera_sdlg_userslv.AddRow(name, id);
	x = kaillera_sdlg_userslv.Find(id);
	wsprintf(bfx, "%i", ping);
	kaillera_sdlg_userslv.FillRow(bfx, 1, x);
	kaillera_sdlg_userslv.FillRow(CONNECTION_TYPES[conn], 4, x);
	kaillera_sdlg_userslv.FillRow(USER_STATUS[status], 3, x);
	wsprintf(bfx, "%i", id);
	kaillera_sdlg_userslv.FillRow(bfx, 2, x);
}
void kaillera_game_add_callback(char*gname, unsigned int id, char*emulator, char*owner, char*users, char status){
	int x;
	char buff[8];

	kaillera_sdlg_gameslv.AddRow(gname, id);
	x = kaillera_sdlg_gameslv.Find(id);

	wsprintf(buff,"%u",id);
	kaillera_sdlg_gameslv.FillRow(buff, 1, x);
	kaillera_sdlg_gameslv.FillRow(emulator, 2, x);
	kaillera_sdlg_gameslv.FillRow(owner, 3, x);
	kaillera_sdlg_gameslv.FillRow(GAME_STATUS[status], 4, x);
	kaillera_sdlg_gameslv.FillRow(users, 5, x);
	kaillera_sdlg_gameslvReSort();
}
void kaillera_game_create_callback(char*gname, unsigned int id, char*emulator, char*owner){
	kaillera_game_add_callback(gname, id, emulator, owner, "1/2", 0);
}

void kaillera_chat_callback(char*name, char * msg){
	kaillera_outpf("<%s> %s", name, msg);
}
void kaillera_game_chat_callback(char*name, char * msg){
	if(!strncmp(name,"Server",6))
		kaillera_gdebug_green("<%s> %s", name, msg);
	else kaillera_gdebug("<%s> %s", name, msg);
	if (KSSDFA.state==2 && infos.chatReceivedCallback) {
		infos.chatReceivedCallback(name, msg);
	}
}
void kaillera_motd_callback(char*name, char * msg){
	kaillera_ui_motd("- %s", msg);
}
void kaillera_user_join_callback(char*name, int ping, unsigned short id, char conn){ // server join
	kaillera_user_add_callback(name, ping, 1, id, conn);
	if(joinleave) kaillera_ui_debug("* Joins: %s (Ping: %d)", name, ping);
	kaillera_sdlg_userslvReSort();
}
void kaillera_user_leave_callback(char*name, char*quitmsg, unsigned short id){
	kaillera_sdlg_userslv.DeleteRow(kaillera_sdlg_userslv.Find(id));
	if(joinleave) kaillera_ui_debug("* Parts: %s (%s)", name, quitmsg);
}
void kaillera_game_close_callback(unsigned int id){
	kaillera_sdlg_gameslv.DeleteRow(kaillera_sdlg_gameslv.Find(id));
}
void kaillera_game_status_change_callback(unsigned int id, char status, int players, int maxplayers){
	char * GAME_STATUS [] = 
	{
		"Waiting",
		"Playing",
		"Playing",
	};
	int x = kaillera_sdlg_gameslv.Find(id);

	kaillera_sdlg_gameslv.FillRow(GAME_STATUS[status], 4, x);
	char users [32];
	wsprintf(users, "%i/%i", players, maxplayers);
	kaillera_sdlg_gameslv.FillRow(users, 5, x);
	kaillera_sdlg_gameslvReSort();
}

void kaillera_user_game_create_callback(){
	kaillera_sdlgGameMode();
	kaillera_sdlg_LV_GULIST.DeleteAllRows();
	SetWindowText(kaillera_sdlg_RE_GCHAT, "");

	// Owna's additions
	hosting = true;
	ExecuteOptions();
	workaround[1] = false;

	// This is a rigged way to "reset" the scroll bar
	// You will see this code in other spots...
	SetScrollRange(kaillera_sdlg_RE_GCHAT,SB_VERT,0,0,false);
}
void kaillera_user_game_closed_callback(){
	kaillera_sdlgNormalMode();
}

void kaillera_user_game_joined_callback(){
	kaillera_sdlgGameMode();
	kaillera_sdlg_LV_GULIST.DeleteAllRows();
	SetWindowText(kaillera_sdlg_RE_GCHAT, "");
	workaround[1] = false;

	// ... like right here! (see above comments)
	SetScrollRange(kaillera_sdlg_RE_GCHAT,SB_VERT,0,0,false);
}

void kaillera_player_add_callback(char *name, int ping, unsigned short id, char conn){
	char bfx[32];
	kaillera_sdlg_LV_GULIST.AddRow(name, id);
	int x = kaillera_sdlg_LV_GULIST.Find(id);
	wsprintf(bfx, "%i", ping);
	kaillera_sdlg_LV_GULIST.FillRow(bfx, 1, x);	
	kaillera_sdlg_LV_GULIST.FillRow(CONNECTION_TYPES[conn], 2, x);
	int thrp = (ping * 60 / 1000 / conn) + 2;
	wsprintf(bfx, "%i frames", thrp * conn - 1);
	kaillera_sdlg_LV_GULIST.FillRow(bfx, 3, x);
}
void kaillera_player_joined_callback(char * username, int ping, unsigned short uid, char connset){ // join game
	if(joinleave) kaillera_gdebug_gray("* Joins: %s", username);
	if(beep) MessageBeep(MB_OK);
	kaillera_player_add_callback(username, ping, uid, connset);
	char msg[256];
	if(hosting && GetWindowText(kaillera_sdlg_TXT_MSG,msg,256))
		kaillera_game_chat_send(msg);
	
	/*if(spoofing) REQ: No spoof message. To even screw with this myself would be obnoxious and pointless.
	{
		// encrypted ping spoof notifier
		// just wanted to see if Smasher could hex this release
		// after being encrypted with this ADVANCED cesar cypher -_-
		// ph33r the rudimentary encryption
		// the decrypted string is "<-- is ping spoofing"
		char tempbuff[] = "B33&oy&votm&yvuulotm'";
		for(int x = 0;x < strlen(tempbuff);x++)
		{
			tempbuff[x]-=6;
		}
		kaillera_game_chat_send(tempbuff);
	}*/

	// Flash the window... not the camera :D
	if(flash)
	{
		FLASHWINFO fwi;
		fwi.cbSize = sizeof(fwi);
		fwi.hwnd = kaillera_sdlg;
		fwi.dwFlags = FLASHW_TIMERNOFG | FLASHW_TRAY;
		fwi.dwTimeout = 0;
		FlashWindowEx(&fwi);
	}
}
void kaillera_player_left_callback(char * user, unsigned short id){
	// The if statements make an easy to way to disable join / part messages
	// TODO: the ability to control join / part messages separately
	// TODO: the ability to control server and in-game messages separately
	if(joinleave) kaillera_gdebug_gray("* Parts: %s", user);
	kaillera_sdlg_LV_GULIST.DeleteRow (kaillera_sdlg_LV_GULIST.Find(id));
}
void kaillera_user_kicked_callback(){
	kaillera_error_callback("* You have been kicked out of the game");
	KSSDFA.input = KSSDFA_END_GAME;
	KSSDFA.state = 0;
	kaillera_sdlgNormalMode();
}
void kaillera_login_stat_callback(char*lsmsg){
	kaillera_core_debug("* %s", lsmsg);
}
void kaillera_player_dropped_callback(char * user, int gdpl){
	kaillera_gdebug("* Dropped: %s (Player %i)", user, gdpl);
	if (infos.clientDroppedCallback)
		infos.clientDroppedCallback(user,gdpl);
	if (gdpl == playerno) {
		KSSDFA.input = KSSDFA_END_GAME;
		KSSDFA.state = 0;
	}
}
void kaillera_game_callback(char * game, char player, char players){
	if (game!= 0)
		strcpy(GAME, game);
	playerno = player;
	numplayers = players;
	//kaillera_gdebug("kaillera_game_callback(%s, %i, %i)", GAME, playerno, numplayers);
	kaillera_gdebug("Press \"Drop\" if your emulator fails to load the game.");
	KSSDFA.input = KSSDFA_START_GAME;
}
void kaillera_game_netsync_wait_callback(int tx){
	SetWindowText(kaillera_sdlg_ST_SPEED, "waiting for others");
	int secs = tx / 1000;
	int ssecs = (tx % 1000) / 100;
	char xxx[32];
	wsprintf(xxx,"%03i.%is", secs, ssecs);
	//SetWindowText(kaillera_sdlg_ST_DELAY, xxx);
	kaillera_sdlg_delay = -1;
}
void kaillera_end_game_callback(){
	KSSDFA.input = KSSDFA_END_GAME;
	KSSDFA.state = 0;
}
//=============================================================

HMENU kaillera_sdlg_CreateGamesMenu = 0;
int kaillera_sdlg_GamesCount = 0;
void kaillera_sdlg_create_games_list_menu() {
	{
		kaillera_sdlg_GamesCount = 0;
		char * xx = gamelist;
		int p;
		while ((p=strlen(xx))!= 0){
			xx += p+ 1;
			kaillera_sdlg_GamesCount++;
		}
	}
	MENUITEMINFO mi;
	char * cx = gamelist;
	HMENU ht = kaillera_sdlg_CreateGamesMenu = CreatePopupMenu();
	char last_char = 0;
	char new_char_buffer[4];
	mi.fMask = MIIM_ID | MIIM_TYPE | MFT_STRING;
	mi.fType = MFT_STRING;
	int counter = 1;
	while ( *cx != 0) {
		mi.wID = counter;
		mi.dwTypeData = cx;
		mi.dwItemData = (unsigned long)/*(ULONG_PTR)*/cx;
		if (*cx != last_char && kaillera_sdlg_GamesCount > 20){
			new_char_buffer[0] = *cx;
			new_char_buffer[1] = 0;
			ht = CreatePopupMenu();
			AppendMenu(kaillera_sdlg_CreateGamesMenu, MF_POPUP, (UINT_PTR) ht, new_char_buffer);
			last_char = *cx;
		}
		AppendMenu(ht, MF_STRING, (UINT_PTR)cx, cx);
		cx += strlen(cx) + 1;
	}
}
void kaillera_sdlg_join_selected_game(){
	int sel = kaillera_sdlg_gameslv.SelectedRow();
	if (sel>=0 && sel < kaillera_sdlg_gameslv.RowsCount()) {
		unsigned int id = kaillera_sdlg_gameslv.RowNo(sel);
		char temp[128];
		kaillera_sdlg_gameslv.CheckRow(temp, 128, 3, sel);
		/* Owna removed this code since:
		1) it was undesired bugs
		2) the server rejects joining running games anyway!

		if (strcmp(temp, "Waiting")!=0) {
			kaillera_error_callback("Joining running game is not allowed");
			return;
		}*/
		kaillera_sdlg_gameslv.CheckRow(temp, 128, 0, sel);
		char * cx = gamelist;
		while (*cx!=0) {
			int ll;
			if (strcmp(cx, temp)==0) {
				strcpy(GAME, temp);
				kaillera_sdlg_gameslv.CheckRow(temp, 128, 2, sel);
				if (strcmp(temp, APP)!= 0) {
					if (MessageBox(kaillera_sdlg, "Emulator/version mismatch and the game may desync.\nDo you want to continue?", "Error", MB_YESNO | MB_ICONEXCLAMATION)!=IDYES)
						return;
				}
				kaillera_join_game(id);
				// This was used to disable the Create button when you are in a game
				// Now, it changes into the Swap button instead
				// EnableWindow(GetDlgItem(kaillera_sdlg,IDC_CREATE),false);
				return;
			}
			cx += (ll=strlen(cx)) + 1;
		}
		//kaillera_error_callback("The rom '%s' is not in your list.", temp); REQ: HMM. NO.
		//kaillera_error_callback("This rom is not in your list bro."); DOUBLE REQ
		if (MessageBox(kaillera_sdlg, "You don't have this rom. Join?", "Error", MB_YESNO | MB_ICONEXCLAMATION)!=IDYES)
			return;
		kaillera_join_game(id);
	}
}
void kaillera_sdlg_show_games_list_menu(HWND handle, bool incjoin = false){
	POINT pi;
	GetCursorPos(&pi);
	HMENU mmainmenuu = kaillera_sdlg_CreateGamesMenu;
	if(incjoin){
		mmainmenuu = CreatePopupMenu();
		MENUITEMINFO mi;
		mi.cbSize = sizeof(MENUITEMINFO);
		mi.fMask = MIIM_TYPE | MFT_STRING | MIIM_SUBMENU;
		mi.fType = MFT_STRING;
		mi.hSubMenu = kaillera_sdlg_CreateGamesMenu;
		mi.dwTypeData = "Create";
		InsertMenuItem(mmainmenuu, MF_STRING, (int)mi.dwTypeData, &mi);
		mi.fMask = MIIM_TYPE | MFT_STRING;
		mi.dwTypeData = "Join";
		AppendMenu(mmainmenuu, MF_STRING , (int)mi.dwTypeData, mi.dwTypeData);
	}
	char * rtgp = (char*)TrackPopupMenu(mmainmenuu, TPM_RETURNCMD,pi.x,pi.y,0, handle, NULL);
	if(rtgp!=0){
		if(strcmp("Join", rtgp)==0){
			kaillera_sdlg_join_selected_game();
		} else {
			strcpy(GAME, rtgp);
			kaillera_create_game(GAME);
		}
		SetFocus(GetDlgItem(kaillera_sdlg, TXT_GINP));
		//EnableWindow(GetDlgItem(kaillera_sdlg,IDC_CREATE),false); // disable create button
	}
}

// Controls the message that appears when you're sending a PM
LRESULT CALLBACK PM_Proc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	static char uid[32];
	switch (uMsg) {
		case WM_INITDIALOG:
			kaillera_sdlg_userslv.CheckRow(uid,32,2,kaillera_sdlg_userslv.SelectedRow());
			SetFocus(GetDlgItem(hDlg,PM_MESSAGE));
			break;
		case WM_CLOSE:
			EndDialog(hDlg, 0);
			break;
		case WM_COMMAND:
			if (LOWORD(wParam)==PM_SEND){
				//char message[512], text[512]; REQ FIX?
				char message [1038], text[1001];
				GetWindowText(GetDlgItem(hDlg,PM_MESSAGE),text,1000); //reqchange
				// send the PM with kaillera_chat_send
				sprintf_s(message, 1038, "/msg %s %s",uid,text);
				kaillera_chat_send(message);
				EndDialog(hDlg, 0);
			}
			break;
	};
	return 0;
}

// The users' right click menu!!!
void kaillera_sdlg_show_user_menu(HWND handle){
	POINT pi;
	GetCursorPos(&pi);
	HMENU mmainmenuu = CreatePopupMenu(), silence = CreatePopupMenu(), ban = CreatePopupMenu(), kick = CreatePopupMenu(), tempelevated = CreatePopupMenu(), tempadmin = CreatePopupMenu();
	MENUITEMINFO mi;
	mi.cbSize = sizeof(MENUITEMINFO);
	mi.fMask = MIIM_TYPE | MFT_STRING;
	mi.fType = MFT_STRING;

	mi.dwTypeData = "Finduser";
	AppendMenu(mmainmenuu, MF_STRING , 1, mi.dwTypeData);
	mi.dwTypeData = "Ignore";
	AppendMenu(mmainmenuu, MF_STRING , 2, mi.dwTypeData);
	mi.dwTypeData = "Unignore";
	AppendMenu(mmainmenuu, MF_STRING , 3, mi.dwTypeData);
	mi.dwTypeData = "4";
	AppendMenu(silence, MF_STRING, 41, mi.dwTypeData);
	mi.dwTypeData = "15";
	AppendMenu(silence, MF_STRING, 42, mi.dwTypeData);
	mi.dwTypeData = "30";
	AppendMenu(silence, MF_STRING, 43, mi.dwTypeData);
	mi.dwTypeData = "60";
	AppendMenu(silence, MF_STRING, 44, mi.dwTypeData);
	mi.dwTypeData = "120";
	AppendMenu(silence, MF_STRING, 45, mi.dwTypeData);
	mi.dwTypeData = "180";
	AppendMenu(silence, MF_STRING, 46, mi.dwTypeData);
	mi.dwTypeData = "Silence";
	AppendMenu(mmainmenuu, MF_STRING | MF_POPUP, (int)silence, mi.dwTypeData);
	mi.dwTypeData = "4";
	AppendMenu(ban, MF_STRING, 51, mi.dwTypeData);
	mi.dwTypeData = "15";
	AppendMenu(ban, MF_STRING, 52, mi.dwTypeData);
	mi.dwTypeData = "30";
	AppendMenu(ban, MF_STRING, 53, mi.dwTypeData);
	mi.dwTypeData = "60";
	AppendMenu(ban, MF_STRING, 54, mi.dwTypeData);
	mi.dwTypeData = "120";
	AppendMenu(ban, MF_STRING, 55, mi.dwTypeData);
	mi.dwTypeData = "180";
	AppendMenu(ban, MF_STRING, 56, mi.dwTypeData);
	mi.dwTypeData = "Ban";
	AppendMenu(mmainmenuu, MF_STRING | MF_POPUP, (int)ban, mi.dwTypeData);
	mi.dwTypeData = "Kick";
	AppendMenu(mmainmenuu, MF_STRING | MF_POPUP, 6, mi.dwTypeData);
	mi.dwTypeData = "4";
	AppendMenu(tempelevated, MF_STRING, 71, mi.dwTypeData);
	mi.dwTypeData = "15";
	AppendMenu(tempelevated, MF_STRING, 72, mi.dwTypeData);
	mi.dwTypeData = "30";
	AppendMenu(tempelevated, MF_STRING, 73, mi.dwTypeData);
	mi.dwTypeData = "60";
	AppendMenu(tempelevated, MF_STRING, 74, mi.dwTypeData);
	mi.dwTypeData = "120";
	AppendMenu(tempelevated, MF_STRING, 75, mi.dwTypeData);
	mi.dwTypeData = "180";
	AppendMenu(tempelevated, MF_STRING, 76, mi.dwTypeData);
	mi.dwTypeData = "Tempelevated";
	AppendMenu(mmainmenuu, MF_STRING | MF_POPUP, (int)tempelevated, mi.dwTypeData);
	mi.dwTypeData = "4";
	AppendMenu(tempadmin, MF_STRING, 81, mi.dwTypeData);
	mi.dwTypeData = "15";
	AppendMenu(tempadmin, MF_STRING, 82, mi.dwTypeData);
	mi.dwTypeData = "30";
	AppendMenu(tempadmin, MF_STRING, 83, mi.dwTypeData);
	mi.dwTypeData = "60";
	AppendMenu(tempadmin, MF_STRING, 84, mi.dwTypeData);
	mi.dwTypeData = "120";
	AppendMenu(tempadmin, MF_STRING, 85, mi.dwTypeData);
	mi.dwTypeData = "180";
	AppendMenu(tempadmin, MF_STRING, 86, mi.dwTypeData);
	mi.dwTypeData = "Tempadmin";
	AppendMenu(mmainmenuu, MF_STRING | MF_POPUP, (int)tempadmin, mi.dwTypeData);
	mi.dwTypeData = "Send PM";
	AppendMenu(mmainmenuu, MF_STRING, 9, mi.dwTypeData);

	int selection = (int)TrackPopupMenu(mmainmenuu, TPM_RETURNCMD,pi.x,pi.y,0, handle, NULL);
	char name[32], command[64];
	switch(selection)
	{
		case 1: //Finduser
			kaillera_sdlg_userslv.CheckRow(name,32,0,kaillera_sdlg_userslv.SelectedRow());
			sprintf(command,"/finduser %s",name);
			kaillera_chat_send(command);
			break;
		case 2: //Ignore
			kaillera_sdlg_userslv.CheckRow(name,32,2,kaillera_sdlg_userslv.SelectedRow());
			sprintf(command,"/ignore %d",atoi(name));
			kaillera_chat_send(command);
			break;
		case 3: //Unignore
			kaillera_sdlg_userslv.CheckRow(name,32,2,kaillera_sdlg_userslv.SelectedRow());
			sprintf(command,"/unignore %d",atoi(name));
			kaillera_chat_send(command);
			break;
		case 41: //Silence
			kaillera_sdlg_userslv.CheckRow(name,32,2,kaillera_sdlg_userslv.SelectedRow());
			sprintf(command,"/silence %d 4",atoi(name));
			kaillera_chat_send(command);
			break;
		case 42:
			kaillera_sdlg_userslv.CheckRow(name,32,2,kaillera_sdlg_userslv.SelectedRow());
			sprintf(command,"/silence %d 15",atoi(name));
			kaillera_chat_send(command);
			break;
		case 43:
			kaillera_sdlg_userslv.CheckRow(name,32,2,kaillera_sdlg_userslv.SelectedRow());
			sprintf(command,"/silence %d 30",atoi(name));
			kaillera_chat_send(command);
			break;
		case 44:
			kaillera_sdlg_userslv.CheckRow(name,32,2,kaillera_sdlg_userslv.SelectedRow());
			sprintf(command,"/silence %d 60",atoi(name));
			kaillera_chat_send(command);
			break;
		case 45:
			kaillera_sdlg_userslv.CheckRow(name,32,2,kaillera_sdlg_userslv.SelectedRow());
			sprintf(command,"/silence %d 120",atoi(name));
			kaillera_chat_send(command);
			break;
		case 46:
			kaillera_sdlg_userslv.CheckRow(name,32,2,kaillera_sdlg_userslv.SelectedRow());
			sprintf(command,"/silence %d 180",atoi(name));
			kaillera_chat_send(command);
			break;
		case 51: //Ban
			kaillera_sdlg_userslv.CheckRow(name,32,2,kaillera_sdlg_userslv.SelectedRow());
			sprintf(command,"/ban %d 4",atoi(name));
			kaillera_chat_send(command);
			break;
		case 52:
			kaillera_sdlg_userslv.CheckRow(name,32,2,kaillera_sdlg_userslv.SelectedRow());
			sprintf(command,"/ban %d 15",atoi(name));
			kaillera_chat_send(command);
			break;
		case 53:
			kaillera_sdlg_userslv.CheckRow(name,32,2,kaillera_sdlg_userslv.SelectedRow());
			sprintf(command,"/ban %d 30",atoi(name));
			kaillera_chat_send(command);
			break;
		case 54:
			kaillera_sdlg_userslv.CheckRow(name,32,2,kaillera_sdlg_userslv.SelectedRow());
			sprintf(command,"/ban %d 60",atoi(name));
			kaillera_chat_send(command);
			break;
		case 55:
			kaillera_sdlg_userslv.CheckRow(name,32,2,kaillera_sdlg_userslv.SelectedRow());
			sprintf(command,"/ban %d 120",atoi(name));
			kaillera_chat_send(command);
			break;
		case 56:
			kaillera_sdlg_userslv.CheckRow(name,32,2,kaillera_sdlg_userslv.SelectedRow());
			sprintf(command,"/ban %d 180",atoi(name));
			kaillera_chat_send(command);
			break;
		case 6: //Kick
			kaillera_sdlg_userslv.CheckRow(name,32,2,kaillera_sdlg_userslv.SelectedRow());
			sprintf(command,"/kick %d",atoi(name));
			kaillera_chat_send(command);
			break;
		case 71: //Tempelevate
			kaillera_sdlg_userslv.CheckRow(name,32,2,kaillera_sdlg_userslv.SelectedRow());
			sprintf(command,"/tempelevated %d 4",atoi(name));
			kaillera_chat_send(command);
			break;
		case 72:
			kaillera_sdlg_userslv.CheckRow(name,32,2,kaillera_sdlg_userslv.SelectedRow());
			sprintf(command,"/tempelevated %d 15",atoi(name));
			kaillera_chat_send(command);
			break;
		case 73:
			kaillera_sdlg_userslv.CheckRow(name,32,2,kaillera_sdlg_userslv.SelectedRow());
			sprintf(command,"/tempelevated %d 30",atoi(name));
			kaillera_chat_send(command);
			break;
		case 74:
			kaillera_sdlg_userslv.CheckRow(name,32,2,kaillera_sdlg_userslv.SelectedRow());
			sprintf(command,"/tempelevated %d 60",atoi(name));
			kaillera_chat_send(command);
			break;
		case 75:
			kaillera_sdlg_userslv.CheckRow(name,32,2,kaillera_sdlg_userslv.SelectedRow());
			sprintf(command,"/tempelevated %d 120",atoi(name));
			kaillera_chat_send(command);
			break;
		case 76:
			kaillera_sdlg_userslv.CheckRow(name,32,2,kaillera_sdlg_userslv.SelectedRow());
			sprintf(command,"/tempelevated %d 180",atoi(name));
			kaillera_chat_send(command);
			break;
		case 81: //Tempadmin
			kaillera_sdlg_userslv.CheckRow(name,32,2,kaillera_sdlg_userslv.SelectedRow());
			sprintf(command,"/tempadmin %d 4",atoi(name));
			kaillera_chat_send(command);
			break;
		case 82:
			kaillera_sdlg_userslv.CheckRow(name,32,2,kaillera_sdlg_userslv.SelectedRow());
			sprintf(command,"/tempadmin %d 15",atoi(name));
			kaillera_chat_send(command);
			break;
		case 83:
			kaillera_sdlg_userslv.CheckRow(name,32,2,kaillera_sdlg_userslv.SelectedRow());
			sprintf(command,"/tempadmin %d 30",atoi(name));
			kaillera_chat_send(command);
			break;
		case 84:
			kaillera_sdlg_userslv.CheckRow(name,32,2,kaillera_sdlg_userslv.SelectedRow());
			sprintf(command,"/tempadmin %d 60",atoi(name));
			kaillera_chat_send(command);
			break;
		case 85:
			kaillera_sdlg_userslv.CheckRow(name,32,2,kaillera_sdlg_userslv.SelectedRow());
			sprintf(command,"/tempadmin %d 120",atoi(name));
			kaillera_chat_send(command);
			break;
		case 86:
			kaillera_sdlg_userslv.CheckRow(name,32,2,kaillera_sdlg_userslv.SelectedRow());
			sprintf(command,"/tempadmin %d 180",atoi(name));
			kaillera_chat_send(command);
			break;
		case 9:
			DialogBox(hx, (LPCTSTR)KAILLERA_PM, /*pDlg*/handle, (DLGPROC)PM_Proc);
			break;
	}

	SetFocus(GetDlgItem(kaillera_sdlg, TXT_GINP));
}
void kaillera_sdlg_destroy_games_list_menu(){
	
}

// Controls the Options menu
LRESULT CALLBACK Options_Proc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg)
	{
		case WM_INITDIALOG:
			
			kaillera_sdlg_IDC_FLASH = GetDlgItem(hDlg, IDC_FLASH);
			kaillera_sdlg_IDC_BEEP = GetDlgItem(hDlg, IDC_BEEP);

			//load stuff
			LoadOptions(hDlg);
			break;
		case WM_CLOSE:// case WM_QUIT: case WM_DESTROY:
			EndDialog(hDlg, 0);
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_CANCEL:
					EndDialog(hDlg, 0);
					break;
				case IDC_OK:
					//save stuff
					SaveOptions(hDlg);
					ExecuteOptions();

					EndDialog(hDlg, 0);
					break;
			}
			break;
	}

	return 0;
}

//===========================================================================================

// Controls the main kaillera server window
LRESULT CALLBACK KailleraServerDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_INITDIALOG:
		{
			kaillera_sdlg = hDlg;
			{
				char xx[256];
				wsprintf(xx, "Connecting to %s", kaillera_sdlg_NAME);
				SetWindowText(hDlg, xx);
			}
			kaillera_sdlg_userslv.handle = GetDlgItem(hDlg, LV_ULIST);
			kaillera_sdlg_userslv.AddColumn("Name", 80);
			kaillera_sdlg_userslv.AddColumn("Ping", 33);
			kaillera_sdlg_userslv.AddColumn("UID", 50);
			kaillera_sdlg_userslv.AddColumn("Status", 50);
			kaillera_sdlg_userslv.AddColumn("Conset", 38);
			kaillera_sdlg_userslv.FullRowSelect();
			kaillera_sdlg_userslvColumn = 1;
			kaillera_sdlg_userslvColumnOrder[1] = 1;

			kaillera_sdlg_gameslv.handle = GetDlgItem(hDlg, LV_GLIST);
			kaillera_sdlg_gameslv.AddColumn("Name", 300);
			kaillera_sdlg_gameslv.AddColumn("Game ID", 50);
			kaillera_sdlg_gameslv.AddColumn("Emulator", 120);
			kaillera_sdlg_gameslv.AddColumn("User", 100);
			kaillera_sdlg_gameslv.AddColumn("Status", 80);
			kaillera_sdlg_gameslv.AddColumn("Users", 50);
			kaillera_sdlg_gameslv.FullRowSelect();
			kaillera_sdlg_gameslvColumn = 4; //1
			kaillera_sdlg_gameslvColumnOrder[4] = 0;
			kaillera_sdlg_gameslvColumnOrder[1] = 0;

			kaillera_sdlg_partchat = GetDlgItem(hDlg, RE_PART);

			//Sleep(100);
			
			kaillera_sdlg_CHK_REC = GetDlgItem(hDlg, CHK_REC);
			kaillera_sdlg_RE_GCHAT = GetDlgItem(hDlg, RE_GCHAT);
			kaillera_sdlg_TXT_GINP = GetDlgItem(hDlg, TXT_GINP);
			kaillera_sdlg_LV_GULIST.handle = GetDlgItem(hDlg, LV_GULIST);
			kaillera_sdlg_BTN_START = GetDlgItem(hDlg, BTN_START);
			kaillera_sdlg_BTN_DROP = GetDlgItem(hDlg, BTN_DROP);
			kaillera_sdlg_BTN_LEAVE = GetDlgItem(hDlg, BTN_LEAVE);
			kaillera_sdlg_BTN_KICK = GetDlgItem(hDlg, BTN_KICK);
			kaillera_sdlg_ST_SPEED = GetDlgItem(hDlg, ST_SPEED);
			kaillera_sdlg_BTN_GCHAT = GetDlgItem(hDlg, BTN_GCHAT);
			//kaillera_sdlg_ST_DELAY = GetDlgItem(hDlg, ST_DELAY);
			kaillera_sdlg_BTN_LAGSTAT = GetDlgItem(hDlg, BTN_LAGSTAT);
			kaillera_sdlg_BTN_LAGRESET = GetDlgItem(hDlg, BTN_LAGRESET);
			kaillera_sdlg_BTN_OPTIONS = GetDlgItem(hDlg, BTN_OPTIONS);
			kaillera_sdlg_BTN_ADVERTISE = GetDlgItem(hDlg, BTN_ADVERTISE);
			kaillera_sdlg_TXT_MSG = GetDlgItem(hDlg, TXT_MSG);

			kaillera_sdlg_LV_GULIST.AddColumn("Nick", 100);
			kaillera_sdlg_LV_GULIST.AddColumn("Ping", 60);
			kaillera_sdlg_LV_GULIST.AddColumn("Connection", 60);
			kaillera_sdlg_LV_GULIST.AddColumn("Delay", 60);
			kaillera_sdlg_LV_GULIST.FullRowSelect();

			kaillera_sdlgNormalMode();
			kaillera_sdlg_create_games_list_menu();
			kaillera_core_connect(kaillera_sdlg_ip, kaillera_sdlg_port);

			kaillera_sdlg_sipd_timer = SetTimer(hDlg, 0, 1000, 0);

			// Enable auto URL detection
			SendMessage(kaillera_sdlg_partchat,EM_AUTOURLDETECT,true,0);
			SendMessage(kaillera_sdlg_RE_GCHAT,EM_AUTOURLDETECT,true,0);
			SendMessage(kaillera_sdlg_partchat,EM_SETEVENTMASK,0,ENM_LINK);
			SendMessage(kaillera_sdlg_RE_GCHAT,EM_SETEVENTMASK,0,ENM_LINK);

			LoadOptions();

			workaround[0] = workaround[1] = hosting = false;

			SetFocus(GetDlgItem(hDlg,TXT_CHAT));

			LoadWindowPos(hDlg);

			return 0;
			
		}
	case WM_TIMER:
		{
			if (kaillera_sdlg_MODE==1) {
				char xx[256];
				if (kaillera_is_connected())
					wsprintf(xx, "Connected to %s (%i users & %i games)", kaillera_sdlg_NAME, kaillera_sdlg_userslv.RowsCount(), kaillera_sdlg_gameslv.RowsCount());
				else
					wsprintf(xx, "Connecting to %s", kaillera_sdlg_NAME);
				SetWindowText(kaillera_sdlg, xx);
			} else {
				char xx[256];
				wsprintf(xx, "Game %s", GAME);
				SetWindowText(kaillera_sdlg, xx);

				int jf;
				if ((jf = kaillera_get_frames_count()) != kaillera_sdlg_frameno) {
					char xxx[32];
					wsprintf(xxx,"%i fps", jf - kaillera_sdlg_frameno);
					SetWindowText(kaillera_sdlg_ST_SPEED, xxx);
					kaillera_sdlg_frameno = jf;
				}
				if ((jf = kaillera_get_delay()) != kaillera_sdlg_delay) {
					char xxx[32];
					wsprintf(xxx,"%i frames", kaillera_sdlg_delay = jf);
					//SetWindowText(kaillera_sdlg_ST_DELAY, xxx);
				}
			}
		}
		break;
	case WM_CLOSE: case WM_QUIT: case WM_DESTROY:

		KillTimer(hDlg, kaillera_sdlg_sipd_timer);
		
		kaillera_sdlg_destroy_games_list_menu();

		if (KSSDFA.state != 0)
			kaillera_end_game();

		KSSDFA.state = 0;
		KSSDFA.input = KSSDFA_END_GAME;

		SaveWindowPos(hDlg);

		EndDialog(hDlg, 0);
		SetFocus(GetDlgItem(kaillera_ssdlg,CMB_MODE));

		break;
	case WM_SIZE:
		// Owna's crazy math for a dynamically resizing window!!!
		if (wParam == SIZE_RESTORED || wParam == SIZE_MAXIMIZED) // what we care about
		{
			int iWidth, iHeight;
			iWidth = LOWORD(lParam);
			iHeight = HIWORD(lParam);
			float hPerc = 0.70, vPerc = 0.66; // relative percentages
			int hTxtBox = 20; // absolute pixels
			int wButton = 37;

			// top half
			SetWindowPos(kaillera_sdlg_partchat,HWND_TOP,0,0,(int)(hPerc*iWidth),(int)(vPerc*iHeight)-hTxtBox,0);
			SetWindowPos(kaillera_sdlg_userslv.handle,HWND_TOP,(int)(hPerc*iWidth),0,(int)((1-hPerc)*iWidth),(int)(vPerc*iHeight),0);
			//SetWindowPos(GetDlgItem(hDlg, TXT_CHAT),HWND_TOP,0,(int)(vPerc*iHeight)-hTxtBox,(int)(hPerc*iWidth)-2*wButton,hTxtBox,0);
			SetWindowPos(GetDlgItem(hDlg, TXT_CHAT),HWND_TOP,0,(int)(vPerc*iHeight)-hTxtBox,(int)(hPerc*iWidth)-1.30*wButton,hTxtBox,1);
			SetWindowPos(GetDlgItem(hDlg, IDC_CHAT),HWND_TOP,(int)(hPerc*iWidth)-2*wButton,(int)(vPerc*iHeight)-hTxtBox,0,0,SWP_NOSIZE);
			SetWindowPos(GetDlgItem(hDlg, IDC_CREATE),HWND_TOP,(int)(hPerc*iWidth)-wButton,(int)(vPerc*iHeight)-hTxtBox,0,0,SWP_NOSIZE);
			SetWindowPos(GetDlgItem(hDlg, TXT_HOST),HWND_TOP,225,(int)(vPerc*iHeight)-hTxtBox,(int)(hPerc*iWidth)-3.50*wButton,hTxtBox,1);

			//300 + 120 = 420 pixels wide
			// bottom half
			SetWindowPos(kaillera_sdlg_gameslv.handle,HWND_TOP,0,(int)(vPerc*iHeight),iWidth,(int)((1-vPerc)*iHeight),0);
			SetWindowPos(kaillera_sdlg_RE_GCHAT,HWND_TOP,0,(int)(vPerc*iHeight),iWidth-420,(int)((1-vPerc)*iHeight)-hTxtBox,0);
			SetWindowPos(kaillera_sdlg_TXT_GINP,HWND_TOP,0,iHeight-hTxtBox,iWidth-420-wButton,hTxtBox,0);
			SetWindowPos(kaillera_sdlg_BTN_GCHAT,HWND_TOP,iWidth-420-wButton,iHeight-hTxtBox,0,0,SWP_NOSIZE);
			SetWindowPos(GetDlgItem(hDlg, LV_GULIST),HWND_TOP,iWidth-420,(int)(vPerc*iHeight),300,(int)((1-vPerc)*iHeight)-hTxtBox,0);
			
			SetWindowPos(GetDlgItem(hDlg, IDC_JOINMSG),HWND_TOP,iWidth-420+4,iHeight-hTxtBox+2,300,hTxtBox,SWP_NOSIZE);
			SetWindowPos(kaillera_sdlg_TXT_MSG,HWND_TOP,iWidth-420+52,iHeight-hTxtBox,300-52,hTxtBox,0);
			// annoying crap buttons in the bottom half
			SetWindowPos(kaillera_sdlg_BTN_START,HWND_TOP,iWidth-118,(int)(vPerc*iHeight),0,0,SWP_NOSIZE);
			SetWindowPos(kaillera_sdlg_BTN_DROP,HWND_TOP,iWidth-58,(int)(vPerc*iHeight),0,0,SWP_NOSIZE);

			SetWindowPos(kaillera_sdlg_BTN_LEAVE,HWND_TOP,iWidth-118,(int)(vPerc*iHeight)+22,0,0,SWP_NOSIZE);
			SetWindowPos(kaillera_sdlg_BTN_KICK,HWND_TOP,iWidth-58,(int)(vPerc*iHeight)+22,0,0,SWP_NOSIZE);

			SetWindowPos(kaillera_sdlg_BTN_LAGSTAT,HWND_TOP,iWidth-115,(int)(vPerc*iHeight)+45,0,0,SWP_NOSIZE);
			SetWindowPos(kaillera_sdlg_BTN_LAGRESET,HWND_TOP,iWidth-115,(int)(vPerc*iHeight)+70,0,0,SWP_NOSIZE);

			SetWindowPos(kaillera_sdlg_CHK_REC,HWND_TOP,iWidth-110,(int)(vPerc*iHeight)+95,0,0,SWP_NOSIZE);

			SetWindowPos(kaillera_sdlg_ST_SPEED,HWND_TOP,iWidth-110,(int)(vPerc*iHeight)+117,0,0,SWP_NOSIZE);
			//SetWindowPos(kaillera_sdlg_ST_DELAY,HWND_TOP,iWidth-110,(int)(vPerc*iHeight)+139,0,0,SWP_NOSIZE);
			SetWindowPos(kaillera_sdlg_BTN_OPTIONS,HWND_TOP,iWidth-110,(int)(vPerc*iHeight)+139,0,0,SWP_NOSIZE);
			SetWindowPos(kaillera_sdlg_BTN_ADVERTISE,HWND_TOP,iWidth-110,(int)(vPerc*iHeight)+163,0,0,SWP_NOSIZE);

			// force redraw to eliminate artifacts
			InvalidateRect(hDlg,NULL,false);
		}
		break;
	case WM_COMMAND:
		int x;
		switch (LOWORD(wParam)) {
			case IDC_CHAT:
				{
					char buffrr[2024];
					GetWindowText(GetDlgItem(hDlg, TXT_CHAT), buffrr, 2024);

					 // intercept for custom command
					if((strlen(buffrr) == 8 && !strncmp(buffrr,"/refresh",8)) || (strlen(buffrr) == 6 && !strncmp(buffrr,"/clear",6)))
					{
						// clear stuff
						SetWindowText(GetDlgItem(hDlg, RE_PART), "");
						SetWindowText(GetDlgItem(hDlg, TXT_CHAT), "");
						workaround[0] = false;
						SetScrollRange(kaillera_sdlg_partchat,SB_VERT,0,0,false);
						break;
					}
					int l = strlen(buffrr);
					if (l>0) {
						int p = min(l, 2011); //Req bump the limit
						char sbf[2011]; //Req: bump
						memcpy(sbf, buffrr, p);
						sbf[p] = 0;
						kaillera_chat_send(sbf);
						if (l > p) {
							l -= p;
							//memcpy(buffrr, buffrr+p, l+1);
						} else
							buffrr[0] = 0;
						SetWindowText(GetDlgItem(hDlg, TXT_CHAT), buffrr);
						break;
					}
					
				}				
			case BTN_GCHAT:
				{
					char buffrr[2024];
					GetWindowText(kaillera_sdlg_TXT_GINP, buffrr, 2024);

					 // intercept for custom command
					if((strlen(buffrr) == 8 && !strncmp(buffrr,"/refresh",8)) || (strlen(buffrr) == 6 && !strncmp(buffrr,"/clear",6))) // intercept for custom command
					{
						// clear stuff
						SetWindowText(GetDlgItem(hDlg, RE_GCHAT), "");
						SetWindowText(GetDlgItem(hDlg, TXT_GINP), "");
						workaround[1] = false;
						SetScrollRange(kaillera_sdlg_RE_GCHAT,SB_VERT,0,0,false);
						break;
					}
					int l = strlen(buffrr);
					if (l>0) {
						int p = min(l, 10000);
						char sbf[10010];
						memcpy(sbf, buffrr, p);
						sbf[p] = 0;
						kaillera_game_chat_send(sbf);
						if (l > p) {
							l -= p;
							memcpy(buffrr, buffrr+p, l+1);
						} else
							buffrr[0] = 0;
						SetWindowText(kaillera_sdlg_TXT_GINP, buffrr);
					}
					break;
				}
			case BTN_LAGSTAT:
				// ZOMG THIS WAS SO HARD TO CODE :P
				kaillera_game_chat_send("/lagstat");
				break;
			case BTN_LAGRESET:
				kaillera_game_chat_send("/lagreset");
				break;
			case BTN_OPTIONS:
				DialogBox(hx, (LPCTSTR)KAILLERA_OPTIONS, hDlg, (DLGPROC)Options_Proc);
				SetFocus(GetDlgItem(hDlg,TXT_CHAT));
				break;
			case BTN_ADVERTISE:
				char ad[512];
				int maxplayers;
				char hostname[128];
				maxplayers = kaillera_sdlg_LV_GULIST.RowsCount();
				kaillera_sdlg_LV_GULIST.CheckRow(hostname,sizeof(hostname),0,0);

				// Handles the advertising format
				if(hosting)
					sprintf(ad,"%s - %d player(s)",GAME,maxplayers);
				else sprintf(ad,"<%s> | %s - %d player(s)",hostname,GAME,maxplayers);

				kaillera_chat_send(ad);
				break;
			case BTN_LEAVE:
				kaillera_leave_game();
				kaillera_sdlgNormalMode();
				KSSDFA.input = KSSDFA_END_GAME;
				KSSDFA.state = 0;
				SetFocus(GetDlgItem(hDlg,TXT_CHAT));
				break;
			case BTN_DROP:
				kaillera_game_drop();
				break;
			case BTN_START:
				kaillera_start_game();
				break;
			case BTN_KICK:
				x = kaillera_sdlg_LV_GULIST.SelectedRow();
				if (x > 0 && x < kaillera_sdlg_LV_GULIST.RowsCount()) {
					kaillera_kick_user(kaillera_sdlg_LV_GULIST.RowNo(x));
				}
				break;
			case IDC_CREATE:
				// IMPORTANT: This button is the Create button AND the Swap button
				// so be careful to handle this case appropriately
				if(kaillera_sdlg_MODE == 1) // normal mode
				{
					//unsigned short buffrr[2024];
					char buffrr[2024];
					if (GetWindowText(GetDlgItem(hDlg, TXT_HOST), buffrr, 2024));
					{
					
					GetWindowText(GetDlgItem(hDlg, TXT_HOST), buffrr, 2024);

					 // intercept for custom command
					if((strlen(buffrr) == 8 && !strncmp(buffrr,"/refresh",8)) || (strlen(buffrr) == 6 && !strncmp(buffrr,"/clear",6)))
					{
						// clear stuff
						SetWindowText(GetDlgItem(hDlg, RE_PART), "");
						SetWindowText(GetDlgItem(hDlg, TXT_HOST), "");
						workaround[0] = false;
						SetScrollRange(kaillera_sdlg_partchat,SB_VERT,0,0,false);
						break;
					}
					//int l = strlen(buffrr);
					//if (l>0) {
					//	int p = min(l, 127);
					//	char sbf[2000];
					//	memcpy(sbf, buffrr, p);
					//	sbf[p] = 0;
					//	kaillera_chat_send(sbf);
					//	if (l > p) {
					//		l -= p;
					//		//memcpy(buffrr, buffrr+p, l+1);
					//	} else
					//		buffrr[0] = 0;
					//	SetWindowText(GetDlgItem(hDlg, TXT_HOST), buffrr);
					//	break;
					//}
					if (strlen(buffrr) > 0 && strlen(buffrr) < 2000) 
					{
					kaillera_create_game(buffrr);
					//kaillera_kick_user(buffrr);
					}
					if (GetWindowText(GetDlgItem(hDlg, TXT_HOST), buffrr, 2024) && strlen(buffrr) <= 0);
					kaillera_sdlg_show_games_list_menu(hDlg);
				}
				}
				else // = 0 --> game mode
				{
					if(!kaillera_sdlg_toggle)
						kaillera_sdlgNormalMode(true);
					else kaillera_sdlgGameMode(true);
				}
				break;
		};
		break;
		case WM_NOTIFY:

			// URL handling code
			// Parts borrowed from SupraClient IIRC
			NMHDR* pNmhdr = (NMHDR*)lParam;
			ENLINK * tEN = (ENLINK *)pNmhdr;
			if(pNmhdr->code==EN_LINK && tEN->msg == WM_LBUTTONDOWN)
			{
				TEXTRANGE g;
				char tLink[1024];

				g.chrg = tEN->chrg;
				g.lpstrText = tLink;

				SendMessage(((LPNMHDR)lParam)->hwndFrom, EM_GETTEXTRANGE, 0, (LPARAM) &g);
				ShellExecute(((LPNMHDR)lParam)->hwndFrom, NULL, tLink, NULL, NULL, SW_SHOWNORMAL);
				tEN->msg = 0;
			}

			if(pNmhdr->code==NM_DBLCLK && pNmhdr->hwndFrom==kaillera_sdlg_gameslv.handle){
				if(hosting == false)
				{
				kaillera_sdlg_join_selected_game();
				SetFocus(GetDlgItem(kaillera_sdlg, TXT_GINP));
				}
			}
			if(pNmhdr->code==NM_DBLCLK && pNmhdr->hwndFrom==kaillera_sdlg_userslv.handle){
				DialogBox(hx, (LPCTSTR)KAILLERA_PM, hDlg, (DLGPROC)PM_Proc);
			}
			if(pNmhdr->code==LVN_COLUMNCLICK && pNmhdr->hwndFrom==kaillera_sdlg_gameslv.handle){
				kaillera_sdlg_gameslvSort(((NMLISTVIEW*)lParam)->iSubItem);
			}
			if(pNmhdr->code==LVN_COLUMNCLICK && pNmhdr->hwndFrom==kaillera_sdlg_userslv.handle){
				kaillera_sdlg_userslvSort(((NMLISTVIEW*)lParam)->iSubItem);
			}

			if(pNmhdr->code==NM_RCLICK && (pNmhdr->hwndFrom==kaillera_sdlg_gameslv.handle)){
				kaillera_sdlg_show_games_list_menu(hDlg, kaillera_sdlg_gameslv.SelectedRow() >= 0 && kaillera_sdlg_gameslv.SelectedRow() < kaillera_sdlg_gameslv.RowsCount());
			}

			if(pNmhdr->code==NM_RCLICK && (pNmhdr->hwndFrom==GetDlgItem(hDlg, LV_ULIST))){
				kaillera_sdlg_show_user_menu(hDlg);
			}
			
			break;
		};
		return 0;
}





HWND kaillera_ssdlg;
HWND kaillera_ssdlg_conset;

void ConnectToServer(char * ip, int port, HWND pDlg,char * name) {
	KAILLERA_CORE_INITIALIZED = true;

	char tbuf[128];
	GetWindowText(GetDlgItem(kaillera_ssdlg, IDC_PINGSPOOF), tbuf, 128);
	if(tbuf[0])
	{
		spoofPing = atoi(tbuf);

		if(spoofPing >= 0 && spoofPing < 1000) // protection against extreme numbers
			spoofing = true;
	}
	else spoofing = false;

	strcpy(kaillera_sdlg_NAME, name);

	char un[32];
	GetWindowText(GetDlgItem(kaillera_ssdlg, IDC_USRNAME), un, 32);
	un[31]=0;
	char conset = SendMessage(kaillera_ssdlg_conset, CB_GETCURSEL, 0, 0) + 1;
	char buffrr[2024];

	if ((GetWindowText(GetDlgItem(kaillera_ssdlg, IDC_APPNAME), buffrr, 2024)) != NULL)
	{
		if (kaillera_core_initialize(0, buffrr, un, conset)) {
		Sleep(150);
		kaillera_sdlg_port = port;
		strcpy(kaillera_sdlg_ip, ip);
		ShowWindow(kaillera_ssdlg, SW_HIDE);
		DialogBox(hx, (LPCTSTR)KAILLERA_SDLG, /*pDlg*/NULL, (DLGPROC)KailleraServerDialogProc);
		ShowWindow(kaillera_ssdlg, SW_SHOW);
		ShowWindow(pDlg,SW_SHOW);
		//disconnect
		char quitmsg[128];
		GetWindowText(GetDlgItem(kaillera_ssdlg, IDC_QUITMSG), quitmsg, 128);
		kaillera_disconnect(quitmsg);
		kaillera_core_cleanup();

		KSSDFA.state = 0;
		KSSDFA.input = KSSDFA_END_GAME;
		//core cleanup
	}
	}
	if ((GetWindowText(GetDlgItem(kaillera_ssdlg, IDC_APPNAME), buffrr, 2024)) == NULL) //REQ
	{
	if (kaillera_core_initialize(0, APP, un, conset)) {
		Sleep(150);
		kaillera_sdlg_port = port;
		strcpy(kaillera_sdlg_ip, ip);
		ShowWindow(kaillera_ssdlg, SW_HIDE);
		DialogBox(hx, (LPCTSTR)KAILLERA_SDLG, /*pDlg*/NULL, (DLGPROC)KailleraServerDialogProc);
		ShowWindow(kaillera_ssdlg, SW_SHOW);
		ShowWindow(pDlg,SW_SHOW);
		//disconnect
		char quitmsg[128];
		GetWindowText(GetDlgItem(kaillera_ssdlg, IDC_QUITMSG), quitmsg, 128);
		kaillera_disconnect(quitmsg);
		kaillera_core_cleanup();

		KSSDFA.state = 0;
		KSSDFA.input = KSSDFA_END_GAME;
		//core cleanup
	}
	} 

	else {
		MessageBox(pDlg, "Core Initialization Failed", 0, 0);
	}
	KAILLERA_CORE_INITIALIZED = false;
}
//===============================================================================
//===============================================================================
//===============================================================================

typedef struct {
	char servname[32];
	char hostname[128];
}KLSNST;

KLSNST KLSNST_temp;
odlist<KLSNST, 32> KLSList;
nLVw KLSListLv;

void KLSListDisplay(){
	KLSListLv.DeleteAllRows();
	for (int x = 0; x< KLSList.length; x++){
		KLSListLv.AddRow(KLSList[x].servname, x);
		KLSListLv.FillRow(KLSList[x].hostname,1, x);
	}
}





LRESULT CALLBACK KLSListModifyDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (uMsg==WM_INITDIALOG){
		SetWindowText(GetDlgItem(hDlg, IDC_NAME), KLSNST_temp.servname);
		SetWindowText(GetDlgItem(hDlg, IDC_IP), KLSNST_temp.hostname);
		if (lParam)	SetWindowText(hDlg, "Edit");
		else SetWindowText(hDlg, "Add");
	} else if (uMsg==WM_CLOSE){
		EndDialog(hDlg, 0);
	} else if (uMsg==WM_COMMAND){
		if (LOWORD(wParam)==IDOK){
			GetWindowText(GetDlgItem(hDlg, IDC_NAME), KLSNST_temp.servname,32);
			GetWindowText(GetDlgItem(hDlg, IDC_IP), KLSNST_temp.hostname,128);
			EndDialog(hDlg, 1);
		} else if (LOWORD(wParam)==IDCANCEL)
			EndDialog(hDlg, 0);
	}
	return 0;
}


void KLSListEdit(){
	int i = KLSListLv.SelectedRow();
	if (i>=0&&i<KLSList.length) {
		KLSNST_temp = KLSList[i];
		if (DialogBoxParam(hx, (LPCTSTR)P2P_ITEM_EDIT, kaillera_ssdlg, (DLGPROC)KLSListModifyDlgProc, 1)){
			KLSList.set(KLSNST_temp, i);
		}
	}
	KLSListDisplay();
}


void KLSListAdd(){
	memset(&KLSNST_temp, 0, sizeof(KLSNST));
	if (DialogBoxParam(hx, (LPCTSTR)P2P_ITEM_EDIT, kaillera_ssdlg, (DLGPROC)KLSListModifyDlgProc, 0)){
		if(strcmp(KLSNST_temp.hostname,"")!=0 && strcmp(KLSNST_temp.servname,"")!=0)
			KLSList.add(KLSNST_temp);
	}
	KLSListDisplay();
}


void KLSListAdd(char * name, char * hostt){
	strncpy(KLSNST_temp.servname, name, 127);
	strncpy(KLSNST_temp.hostname, hostt, 127);
	KLSList.add(KLSNST_temp);
	KLSListDisplay();
}


void KLSListPING(){
	int i = KLSListLv.SelectedRow();
	if (i>=0&&i<KLSList.length) {
		
		KLSNST xx = KLSList[i];
		
		char * host = xx.hostname;
		int port = 27888;
		while (*++host != ':' && *host != 0);
		if (*host == ':') {
			*host++ = 0x00;
			port = atoi(host);
			port = port==0?27886:port;
		}
		host = xx.hostname;

		int ping = kaillera_ping_server(host, port);
		char pingstr[128];
		wsprintf(pingstr, "%ims", ping);
		KLSListLv.FillRow(pingstr, 2, i);

		//CreateProcess(0, "cmd /k tracert", 0, 0, 0, 0, 0, 0, 0, 0);
	}
}




void KLSListDelete(){
	int i = KLSListLv.SelectedRow();
	if (i>=0&&i<KLSList.length)
		KLSList.removei(i);
	KLSListDisplay();
}

void KLSListConnect(){
	int i = KLSListLv.SelectedRow();
	if (i>=0&&i<KLSList.length) {
		KLSNST xx = KLSList[i];
		
		char * host = xx.hostname;
		int port = 27888;
		while (*++host != ':' && *host != 0);
		if (*host == ':') {
			*host++ = 0x00;
			port = atoi(host);
			port = port==0?27888:port;
		}
		host = xx.hostname;

		ConnectToServer(host, port, kaillera_ssdlg, xx.servname);
	}
}


void KLSListSelect(HWND hDlg){
	int i = KLSListLv.SelectedRow();
	if (i>=0&&i<KLSList.length)
		SetDlgItemText(hDlg, IDC_IP, KLSList[i].hostname);
}

void KLSListDblClick(HWND hDlg){
	int i = KLSListLv.SelectedRow();
	if (i>=0&&i<KLSList.length)
		SetDlgItemText(hDlg, IDC_IP, KLSList[i].hostname);
}

void KLSListLoad(){
	KLSList.clear();
	int count = nSettings::get_int("SLC", 0);
	for (int x=1;x<=count;x++){
		char idt[32];
		KLSNST sx;
		wsprintf(idt, "SLS%i", x);
		nSettings::get_str(idt,sx.servname, "UserName");
		wsprintf(idt, "SLH%i", x);
		nSettings::get_str(idt,sx.hostname, "127.0.0.1");
		KLSList.add(sx);
	}
	KLSListDisplay();
}

void KLSListSave(){
	nSettings::set_int("SLC",KLSList.length);
	for (int x=0;x<KLSList.length;x++){
		char idt[32];
		KLSNST sx = KLSList[x];
		wsprintf(idt, "SLS%i", x+1);
		nSettings::set_str(idt,sx.servname);
		wsprintf(idt, "SLH%i", x+1);
		nSettings::set_str(idt,sx.hostname);
	}
}
//---------------------------------------

// Controls the About window
LRESULT CALLBACK AboutDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_CLOSE:
		EndDialog(hDlg, 0);
		SetFocus(GetDlgItem(kaillera_ssdlg,CMB_MODE));
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case BTN_CLOSE:
			EndDialog(hDlg, 0);
			SetFocus(GetDlgItem(kaillera_ssdlg,CMB_MODE));
			break;
		case BTN_LICENSE:
			MessageBox(0,"insert license URL here",0,0);
			break;
		case BTN_OKAI:
			MessageBox(0,"insert okai URL here",0,0);
			break;
		case BTN_USEAGE:
			MessageBox(0,"insert useage URL here",0,0);
			break;
		};
		break;
	};
	return 0;
}
void ShowAboutDialog(HWND hDlg) {
	DialogBox(hx, (LPCTSTR)KAILLERA_ABOUT, hDlg, (DLGPROC)AboutDialogProc);
}



//////////////

// Controls the custom IP window
LRESULT CALLBACK CustomIPDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
		case WM_CLOSE:
			EndDialog(hDlg, 0);
			break;
		case WM_COMMAND:
			if (LOWORD(wParam)==BTN_CONNECT){
				char * host = KLSNST_temp.hostname;
				GetWindowText(GetDlgItem(hDlg, IDC_IP), host, 128);
				int port = 27888;
				while (*++host != ':' && *host != 0);
				if (*host == ':') {
					*host++ = 0x00;
					port = atoi(host);
					port = port==0?27888:port;
				}
				host = KLSNST_temp.hostname;
				
				ConnectToServer(host, port, kaillera_ssdlg,host);
				EndDialog(hDlg, 0);
			}
			break;
	};
	return 0;
}
void ShowCustomIPDialog(HWND hDlg) {
	DialogBox(hx, (LPCTSTR)KAILLERA_CUSTOMIP, hDlg, (DLGPROC)CustomIPDialogProc);
}

//////////////////////////////////////////
LRESULT CALLBACK MasterSLDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK AntiSLDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WaitingDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
void ShowMasterSLDialog(HWND hDlg, int type = 0) {
	if(type == 1)
		DialogBox(hx, (LPCTSTR)KAILLERA_MLIST, /*hDlg*/NULL, (DLGPROC)AntiSLDialogProc);
	else if(type == 0)
		DialogBox(hx, (LPCTSTR)KAILLERA_MLIST, /*hDlg*/NULL, (DLGPROC)MasterSLDialogProc);
	else if(type == 2)
		DialogBox(hx, (LPCTSTR)KAILLERA_MLIST, /*hDlg*/NULL, (DLGPROC)WaitingDialogProc);
}

//////////////////////////////////////////

// Controls the Kaillera server select window
LRESULT CALLBACK KailleraServerSelectDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	HBRUSH g_hbrBackground = CreateSolidBrush(RGB(175, 0, 0));
	switch (uMsg) {
	case WM_INITDIALOG:
		{
			kaillera_ssdlg = hDlg;
			
			SetWindowText(hDlg, "n02 " KAILLERA_VERSION);
			
			nSettings::Initialize("SC");
			
			DWORD xxx = 32;
			char USERNAME[33];
			GetUserName(USERNAME, &xxx);
			char un[128];
			nSettings::get_str("USRN", un, USERNAME);
			strncpy(USERNAME, un, 34);
			SetWindowText(GetDlgItem(hDlg, IDC_USRNAME), USERNAME);

			//DWORD xxx = 32;
			char QUITMSG[128] = "Open Kaillera - n02 " KAILLERA_VERSION;
			//char un[128];
			nSettings::get_str("QMSG", un, QUITMSG);
			strncpy(QUITMSG, un, 128);
			SetWindowText(GetDlgItem(hDlg, IDC_QUITMSG), QUITMSG);
			
			kaillera_ssdlg_conset = GetDlgItem(hDlg, CB_CONSET);
			SendMessage(kaillera_ssdlg_conset, CB_ADDSTRING, 0, (WPARAM)"LAN      (60 packets/s)");
			SendMessage(kaillera_ssdlg_conset, CB_ADDSTRING, 0, (WPARAM)"Sexcellent(30 packets/s)"); //REQ :d
			SendMessage(kaillera_ssdlg_conset, CB_ADDSTRING, 0, (WPARAM)"Good     (20 packets/s)");
			SendMessage(kaillera_ssdlg_conset, CB_ADDSTRING, 0, (WPARAM)"Average  (15 packets/s)");
			SendMessage(kaillera_ssdlg_conset, CB_ADDSTRING, 0, (WPARAM)"Low      (12 packets/s)");
			/*SendMessage(kaillera_ssdlg_conset, CB_ADDSTRING, 0, (WPARAM)"Bad      (10 packets/s)"); REQ :d */
			SendMessage(kaillera_ssdlg_conset, CB_ADDSTRING, 0, (WPARAM)"Mexican      (10 packets/s)");
			SendMessage(kaillera_ssdlg_conset, CB_SETCURSEL, nSettings::get_int("CNS", 0), 0);	
			
			KLSListLv.handle = GetDlgItem(hDlg, LV_ULIST);
			KLSListLv.AddColumn("Name", 150);
			KLSListLv.AddColumn("IP", 100);
			KLSListLv.AddColumn("Ping", 100);
			KLSListLv.FullRowSelect();			
			
			KLSListLoad();
			
			initialize_mode_cb(GetDlgItem(hDlg, CMB_MODE));

			SetFocus(GetDlgItem(hDlg,CMB_MODE));
		}
		break;
		case WM_CTLCOLORDLG:
		return (LONG)g_hbrBackground;

	case WM_CTLCOLORSTATIC:
    {
        HDC hdcStatic = (HDC)wParam;
        SetTextColor(hdcStatic, RGB(0, 0, 0));
        SetBkMode(hdcStatic, TRANSPARENT);
        return (LONG)g_hbrBackground;
    }
	case WM_CLOSE: case WM_QUIT: case WM_DESTROY: // does not crash from the code in here
		{
			char tbuf[128];
			GetWindowText(GetDlgItem(hDlg, IDC_QUITMSG), tbuf, 128);
			nSettings::set_str("QMSG", tbuf);
			
			GetWindowText(GetDlgItem(hDlg, IDC_USRNAME), tbuf, 128);
			nSettings::set_str("USRN", tbuf);

			nSettings::set_int("CNS", SendMessage(kaillera_ssdlg_conset, CB_GETCURSEL, 0, 0));
					
			KLSListSave();
			
			EndDialog(hDlg, 0);
			nSettings::Terminate();

		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
			case BTN_ADD:
				KLSListAdd();
				break;
			case BTN_EDIT:
				KLSListEdit();
				break;
			case BTN_DELETE:
				KLSListDelete();
				break;
			case BTN_PING:
				KLSListPING();
				break;
			case BTN_CONNECT:
				// save stuff early
				char tbuf[128];
				GetWindowText(GetDlgItem(hDlg, IDC_QUITMSG), tbuf, 128);
				nSettings::set_str("QMSG", tbuf);
				GetWindowText(GetDlgItem(hDlg, IDC_USRNAME), tbuf, 128);
				nSettings::set_str("USRN", tbuf);
				nSettings::set_int("CNS", SendMessage(kaillera_ssdlg_conset, CB_GETCURSEL, 0, 0));
				KLSListSave();

				KLSListConnect();
				break;
			case BTN_ABOUT:
				ShowAboutDialog(hDlg);
				break;
			case BTN_CUSTOM:
				ShowCustomIPDialog(hDlg);
				break;
			case BTN_MLIST:
				ShowMasterSLDialog(hDlg,0);
				break;
			case BTN_MLIST2:
				ShowMasterSLDialog(hDlg,1);
				break;
			case BTN_MLIST3:
				ShowMasterSLDialog(hDlg,2);
				break;
			case CMB_MODE:
				if (HIWORD(wParam)==CBN_SELCHANGE) {
					if (activate_mode(SendMessage(GetDlgItem(hDlg, CMB_MODE), CB_GETCURSEL, 0, 0))){
						SendMessage(hDlg, WM_CLOSE, 0, 0);
					}
				}
				break;
		};
		break;
		case WM_NOTIFY:

			if(((LPNMHDR)lParam)->code==NM_DBLCLK && ((LPNMHDR)lParam)->hwndFrom==KLSListLv.handle){
				// save stuff early
				char tbuf[128];
				GetWindowText(GetDlgItem(hDlg, IDC_QUITMSG), tbuf, 128);
				nSettings::set_str("QMSG", tbuf);
				GetWindowText(GetDlgItem(hDlg, IDC_USRNAME), tbuf, 128);
				nSettings::set_str("USRN", tbuf);
				nSettings::set_int("CNS", SendMessage(kaillera_ssdlg_conset, CB_GETCURSEL, 0, 0));
				KLSListSave();

				KLSListConnect();
			}
			if(((LPNMHDR)lParam)->code==NM_RCLICK && ((LPNMHDR)lParam)->hwndFrom==KLSListLv.handle){
				KLSListPING();
			}

			break;
		};
		return 0;
}

//char * callback_checkSize(char * msg )
//{
//	//char lim[2085];
//	//lim[2085] = '\0';
//	//strncpy(lim, msg, 2085);
//	//char * point = lim[0];
//
//	//return point;
//}



void kaillera_GUI(){
	INITCOMMONCONTROLSEX icx;
	icx.dwSize = sizeof(icx);
	icx.dwICC = ICC_LISTVIEW_CLASSES | ICC_TAB_CLASSES;
	InitCommonControlsEx(&icx);
	
	HMODULE p2p_riched_hm = LoadLibrary("riched32.dll");
	
	DialogBox(hx, (LPCTSTR)KAILLERA_SSDLG, 0, (DLGPROC)KailleraServerSelectDialogProc);
	
	FreeLibrary(p2p_riched_hm);
}