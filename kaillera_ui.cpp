#include "kaillera_ui.h"

#include <windows.h>
#include "uihlp.h"

#include "resource.h"
#include "kailleraclient.h"
#include "common/nSTL.h"
#include "common/k_socket.h"
#include "common/nThread.h"
#include "common/nSettings.h"
#include "stdio.h"


bool KAILLERA_CORE_INITIALIZED = false;




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
HWND kaillera_sdlg_ST_DELAY;
HWND kaillera_sdlg_BTN_GCHAT;
UINT_PTR kaillera_sdlg_sipd_timer;
int kaillera_sdlg_frameno = 0;
int kaillera_sdlg_delay = -1;

//=======================================================================
bool kaillera_RecordingEnabled(){
	return SendMessage(GetDlgItem(kaillera_sdlg, CHK_REC), BM_GETCHECK, 0, 0)==BST_CHECKED;
}
int kaillera_sdlg_MODE;
void kaillera_sdlgGameMode(){
	kaillera_sdlg_MODE = 0;
	ShowWindow(kaillera_sdlg_CHK_REC,SW_SHOW);
	ShowWindow(kaillera_sdlg_RE_GCHAT,SW_SHOW);
	ShowWindow(kaillera_sdlg_TXT_GINP,SW_SHOW);
	ShowWindow(kaillera_sdlg_LV_GULIST.handle,SW_SHOW);
	ShowWindow(kaillera_sdlg_BTN_START,SW_SHOW);
	ShowWindow(kaillera_sdlg_BTN_DROP,SW_SHOW);
	ShowWindow(kaillera_sdlg_BTN_LEAVE,SW_SHOW);
	ShowWindow(kaillera_sdlg_BTN_KICK,SW_SHOW);
	ShowWindow(kaillera_sdlg_ST_SPEED,SW_SHOW);
	ShowWindow(kaillera_sdlg_ST_DELAY,SW_SHOW);
	ShowWindow(kaillera_sdlg_BTN_GCHAT,SW_SHOW);
	ShowWindow(kaillera_sdlg_gameslv.handle,SW_HIDE);
}

void kaillera_sdlgNormalMode(){
	kaillera_sdlg_MODE = 1;
	ShowWindow(kaillera_sdlg_CHK_REC,SW_HIDE);
	ShowWindow(kaillera_sdlg_RE_GCHAT,SW_HIDE);
	ShowWindow(kaillera_sdlg_TXT_GINP,SW_HIDE);
	ShowWindow(kaillera_sdlg_LV_GULIST.handle,SW_HIDE);
	ShowWindow(kaillera_sdlg_BTN_START,SW_HIDE);
	ShowWindow(kaillera_sdlg_BTN_DROP,SW_HIDE);
	ShowWindow(kaillera_sdlg_BTN_LEAVE,SW_HIDE);
	ShowWindow(kaillera_sdlg_BTN_KICK,SW_HIDE);
	ShowWindow(kaillera_sdlg_ST_SPEED,SW_HIDE);
	ShowWindow(kaillera_sdlg_ST_DELAY,SW_HIDE);
	ShowWindow(kaillera_sdlg_BTN_GCHAT,SW_HIDE);
	ShowWindow(kaillera_sdlg_gameslv.handle,SW_SHOW);
}
//===================================

int kaillera_sdlg_gameslvColumn;
int kaillera_sdlg_gameslvColumnTypes[7] = {1, 1, 1, 1, 0, 0, 0};
int kaillera_sdlg_gameslvColumnOrder[7];

int CALLBACK kaillera_sdlg_gameslvCompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort){

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


int kaillera_sdlg_userslvColumn;
int kaillera_sdlg_userslvColumnTypes[7] = {1, 0, 1, 1, 0, 1, 1};
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


void kaillera_goutp(char * line){
	re_append(kaillera_sdlg_RE_GCHAT, line, 0);
}

void __cdecl kaillera_gdebug(char * arg_0, ...) {
	char V8[1024];
	char V88[2084];
	sprintf_s(V8, 1021, "%s\r\n", arg_0);
	va_list args;
	va_start (args, arg_0);
	vsnprintf_s(V88, 2084, 2082, V8, args);
	va_end (args);
	kaillera_goutp(V88);
}

void __cdecl kaillera_core_debug(char * arg_0, ...) {
	char V8[1024];
	char V88[2084];
	sprintf_s(V8, 1021, "%s\r\n", arg_0);
	va_list args;
	va_start (args, arg_0);
	vsnprintf_s(V88, 2084, 2082, V8, args);
	va_end (args);
	re_append(kaillera_sdlg_partchat, V88, 0x333333);
}
void __cdecl kaillera_ui_motd(char * arg_0, ...) {
	char V8[1024];
	char V88[2084];
	sprintf_s(V8, 1021, "%s\r\n", arg_0);
	va_list args;
	va_start (args, arg_0);
	vsnprintf_s(V88, sizeof(V88), 2083, V8, args);
	va_end (args);
	re_append(kaillera_sdlg_partchat, V88, 0x00336633);
}
void __cdecl kaillera_error_callback(char * arg_0, ...) {
	char V8[1024];
	char V88[2084];
	sprintf_s(V8, 1021, "%s\r\n", arg_0);
	va_list args;
	va_start (args, arg_0);
	vsnprintf_s(V88, 2084, 2082, V8, args);
	va_end (args);

	re_append(kaillera_sdlg_partchat, V88, 0x000000FF);
}

void __cdecl kaillera_ui_debug(char * arg_0, ...) {
	char V8[1024];
	char V88[2084];
	sprintf_s(V8, 1021, "%s\r\n", arg_0);
	va_list args;
	va_start (args, arg_0);
	vsnprintf_s(V88, 2084, 2082, V8, args);
	va_end (args);

	re_append(kaillera_sdlg_partchat, V88, 0x00777777);
}

void __cdecl kaillera_outpf(char * arg_0, ...) {
	char V8[1024];
	char V88[2084];
	sprintf_s(V8, 1021, "%s\r\n", arg_0);
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
	kaillera_sdlg_userslv.FillRow(CONNECTION_TYPES[conn], 2, x);
	kaillera_sdlg_userslv.FillRow(USER_STATUS[status], 3, x);
}
void kaillera_game_add_callback(char*gname, unsigned int id, char*emulator, char*owner, char*users, char status){
	int x;
	
	kaillera_sdlg_gameslv.AddRow(gname, id);
	x = kaillera_sdlg_gameslv.Find(id);
	
	kaillera_sdlg_gameslv.FillRow(emulator, 1, x);
	kaillera_sdlg_gameslv.FillRow(owner, 2, x);
	kaillera_sdlg_gameslv.FillRow(GAME_STATUS[status], 3, x);
	kaillera_sdlg_gameslv.FillRow(users, 4, x);
	kaillera_sdlg_gameslvReSort();
}
void kaillera_game_create_callback(char*gname, unsigned int id, char*emulator, char*owner){
	kaillera_game_add_callback(gname, id, emulator, owner, "1/2", 0);
}

void kaillera_chat_callback(char*name, char * msg){
	kaillera_outpf("<%s> %s", name, msg);
}
void kaillera_game_chat_callback(char*name, char * msg){
	kaillera_gdebug("<%s> %s", name, msg);
	if (KSSDFA.state==2 && infos.chatReceivedCallback) {
		infos.chatReceivedCallback(name, msg);
		//l
	}
}
void kaillera_motd_callback(char*name, char * msg){
	kaillera_ui_motd("- %s", msg);
}
void kaillera_user_join_callback(char*name, int ping, unsigned short id, char conn){
	kaillera_user_add_callback(name, ping, 1, id, conn);
	kaillera_ui_debug("* Joins: %s", name);
	kaillera_sdlg_userslvReSort();
}
void kaillera_user_leave_callback(char*name, char*quitmsg, unsigned short id){
	kaillera_sdlg_userslv.DeleteRow(kaillera_sdlg_userslv.Find(id));
	kaillera_ui_debug("* Parts: %s (%s)", name, quitmsg);
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
	kaillera_sdlg_gameslv.FillRow(GAME_STATUS[status], 3, x);
	char users [32];
	wsprintf(users, "%i/%i", players, maxplayers);
	kaillera_sdlg_gameslv.FillRow(users, 4, x);
	kaillera_sdlg_gameslvReSort();
}

void kaillera_user_game_create_callback(){
	kaillera_sdlgGameMode();
	kaillera_sdlg_LV_GULIST.DeleteAllRows();
	SetWindowText(kaillera_sdlg_RE_GCHAT, "");
	EnableWindow(kaillera_sdlg_BTN_KICK, TRUE);
	EnableWindow(kaillera_sdlg_BTN_START, TRUE);
}
void kaillera_user_game_closed_callback(){
	kaillera_sdlgNormalMode();
}

void kaillera_user_game_joined_callback(){
	kaillera_sdlgGameMode();
	kaillera_sdlg_LV_GULIST.DeleteAllRows();
	SetWindowText(kaillera_sdlg_RE_GCHAT, "");
	EnableWindow(kaillera_sdlg_BTN_KICK, FALSE);
	EnableWindow(kaillera_sdlg_BTN_START, FALSE);
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
void kaillera_player_joined_callback(char * username, int ping, unsigned short uid, char connset){
	kaillera_gdebug("* Joins: %s", username);
	kaillera_player_add_callback(username, ping, uid, connset);
}
void kaillera_player_left_callback(char * user, unsigned short id){
	kaillera_gdebug("* Parts: %s", user);
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
	kaillera_gdebug("kaillera_game_callback(%s, %i, %i)", GAME, playerno, numplayers);
	kaillera_gdebug("press \"Drop\" if your emulator fails to load the game");
	KSSDFA.input = KSSDFA_START_GAME;
}
void kaillera_game_netsync_wait_callback(int tx){
	SetWindowText(kaillera_sdlg_ST_SPEED, "waiting for others");
	int secs = tx / 1000;
	int ssecs = (tx % 1000) / 100;
	char xxx[32];
	wsprintf(xxx,"%03i.%is", secs, ssecs);
	SetWindowText(kaillera_sdlg_ST_DELAY, xxx);
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
void kailelra_sdlg_join_selected_game(){
	int sel = kaillera_sdlg_gameslv.SelectedRow();
	if (sel>=0 && sel < kaillera_sdlg_gameslv.RowsCount()) {
		unsigned int id = kaillera_sdlg_gameslv.RowNo(sel);
		char temp[128];
		kaillera_sdlg_gameslv.CheckRow(temp, 128, 3, sel);
		if (strcmp(temp, "Waiting")!=0) {
			kaillera_error_callback("Joining running game is not allowed");
			return;
		}
		kaillera_sdlg_gameslv.CheckRow(temp, 128, 0, sel);
		char * cx = gamelist;
		while (*cx!=0) {
			int ll;
			if (strcmp(cx, temp)==0) {
				strcpy(GAME, temp);
				kaillera_sdlg_gameslv.CheckRow(temp, 128, 1, sel);
				if (strcmp(temp, APP)!= 0) {
					if (MessageBox(kaillera_sdlg, "Emulator/version mismatch and the game may desync.\nDo you want to continue?", "Error", MB_YESNO | MB_ICONEXCLAMATION)!=IDYES)
						return;
				}
				kaillera_join_game(id);
				return;
			}
			cx += (ll=strlen(cx)) + 1;
		}
		kaillera_error_callback("The rom '%s' is not in your list.", temp);
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
		mi.dwTypeData = "Créer";
		InsertMenuItem(mmainmenuu, MF_STRING, (int)mi.dwTypeData, &mi);
		mi.fMask = MIIM_TYPE | MFT_STRING;
		mi.dwTypeData = "Rejoindre";
		AppendMenu(mmainmenuu, MF_STRING , (int)mi.dwTypeData, mi.dwTypeData);
	}
	char * rtgp = (char*)TrackPopupMenu(mmainmenuu, TPM_RETURNCMD,pi.x,pi.y,0, handle, NULL);
	if(rtgp!=0){
		if(strcmp("Rejoindre", rtgp)==0){
			kailelra_sdlg_join_selected_game();
		} else {
			strcpy(GAME, rtgp);
			kaillera_create_game(GAME);
		}
	}
}
void kaillera_sdlg_destroy_games_list_menu(){
	
}

//===========================================================================================

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
			kaillera_sdlg_userslv.AddColumn("Name", 100);
			kaillera_sdlg_userslv.AddColumn("Ping", 35);
			kaillera_sdlg_userslv.AddColumn("Conset", 40);
			kaillera_sdlg_userslv.AddColumn("Status", 30);
			kaillera_sdlg_userslv.FullRowSelect();
			kaillera_sdlg_userslvColumn = 1;
			kaillera_sdlg_userslvColumnOrder[1] = 1;

			kaillera_sdlg_gameslv.handle = GetDlgItem(hDlg, LV_GLIST);
			kaillera_sdlg_gameslv.AddColumn("Name", 300);
			kaillera_sdlg_gameslv.AddColumn("Emulator", 120);
			kaillera_sdlg_gameslv.AddColumn("User", 100);
			kaillera_sdlg_gameslv.AddColumn("Status", 80);
			kaillera_sdlg_gameslv.AddColumn("Users", 50);
			kaillera_sdlg_gameslv.FullRowSelect();
			kaillera_sdlg_gameslvColumn = 3;
			kaillera_sdlg_gameslvColumnOrder[3] = 0;

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
			kaillera_sdlg_ST_DELAY = GetDlgItem(hDlg, ST_DELAY);


			kaillera_sdlg_LV_GULIST.AddColumn("Nick", 100);
			kaillera_sdlg_LV_GULIST.AddColumn("Ping", 60);
			kaillera_sdlg_LV_GULIST.AddColumn("Connection", 60);
			kaillera_sdlg_LV_GULIST.AddColumn("Delay", 60);
			kaillera_sdlg_LV_GULIST.FullRowSelect();

			kaillera_sdlgNormalMode();
			kaillera_sdlg_create_games_list_menu();
			kaillera_core_connect(kaillera_sdlg_ip, kaillera_sdlg_port);

			kaillera_sdlg_sipd_timer = SetTimer(hDlg, 0, 1000, 0);

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
					SetWindowText(kaillera_sdlg_ST_DELAY, xxx);
				}
			}
		}
		break;
	case WM_CLOSE:

		KillTimer(hDlg, kaillera_sdlg_sipd_timer);
		
		kaillera_sdlg_destroy_games_list_menu();

		if (KSSDFA.state != 0)
			kaillera_end_game();

		KSSDFA.state = 0;
		KSSDFA.input = KSSDFA_END_GAME;

		EndDialog(hDlg, 0);

		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
			case IDC_CHAT:
				{
					char buffrr[2024];
					GetWindowText(GetDlgItem(hDlg, TXT_CHAT), buffrr, 2024);
					int l = strlen(buffrr);
					if (l>0) {
						int p = min(l, 127);
						char sbf[128];
						memcpy(sbf, buffrr, p);
						sbf[p] = 0;
						kaillera_chat_send(sbf);
						if (l > p) {
							l -= p;
							memcpy(buffrr, buffrr+p, l+1);
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
					int l = strlen(buffrr);
					if (l>0) {
						int p = min(l, 127);
						char sbf[128];
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
			case BTN_LEAVE:
				kaillera_leave_game();
				kaillera_sdlgNormalMode();
				KSSDFA.input = KSSDFA_END_GAME;
				KSSDFA.state = 0;
				break;
			case BTN_DROP:
				kaillera_game_drop();
				break;
			case BTN_START:
				kaillera_start_game();
				break;
			case BTN_KICK:
				int x = kaillera_sdlg_LV_GULIST.SelectedRow();
				if (x > 0 && x < kaillera_sdlg_LV_GULIST.RowsCount()) {
					kaillera_kick_user(kaillera_sdlg_LV_GULIST.RowNo(x));
				}
				break;
		};
		break;
		case WM_NOTIFY:
			/*
			NMHDR * nParam = (NMHDR*)lParam;
			if (nParam->hwndFrom == p2p_ui_modeseltab && nParam->code == TCN_SELCHANGE){
			nTab tabb;
			tabb.handle = p2p_ui_modeseltab;
			P2PSelectionDialogProcSetMode(hDlg, tabb.SelectedTab()!=0);	
			}
			if (nParam->hwndFrom == p2p_ui_storedusers.handle){
			if (nParam->code == NM_CLICK) {
			KLSListListSelect(hDlg);
			}
			}
				*/
			if(((LPNMHDR)lParam)->code==NM_DBLCLK && ((LPNMHDR)lParam)->hwndFrom==kaillera_sdlg_gameslv.handle){
				kailelra_sdlg_join_selected_game();
			}
			if(((LPNMHDR)lParam)->code==LVN_COLUMNCLICK && ((LPNMHDR)lParam)->hwndFrom==kaillera_sdlg_gameslv.handle){
				kaillera_sdlg_gameslvSort(((NMLISTVIEW*)lParam)->iSubItem);
			}
			if(((LPNMHDR)lParam)->code==LVN_COLUMNCLICK && ((LPNMHDR)lParam)->hwndFrom==kaillera_sdlg_userslv.handle){
				kaillera_sdlg_userslvSort(((NMLISTVIEW*)lParam)->iSubItem);
			}

			if(((LPNMHDR)lParam)->code==NM_RCLICK && (((LPNMHDR)lParam)->hwndFrom==kaillera_sdlg_gameslv.handle)){
				kaillera_sdlg_show_games_list_menu(hDlg, kaillera_sdlg_gameslv.SelectedRow() >= 0 && kaillera_sdlg_gameslv.SelectedRow() < kaillera_sdlg_gameslv.RowsCount());
			}
			
			break;
		};
		return 0;
}





HWND kaillera_ssdlg;
HWND kaillera_ssdlg_conset;

void ConnectToServer(char * ip, int port, HWND pDlg,char * name) {
	KAILLERA_CORE_INITIALIZED = true;

	strcpy(kaillera_sdlg_NAME, name);

	char un[32];
	GetWindowText(GetDlgItem(kaillera_ssdlg, IDC_USRNAME), un, 32);
	un[31]=0;
	char conset = SendMessage(kaillera_ssdlg_conset, CB_GETCURSEL, 0, 0) + 1;
	if (kaillera_core_initialize(0, APP, un, conset)) {
		Sleep(150);
		kaillera_sdlg_port = port;
		strcpy(kaillera_sdlg_ip, ip);
		DialogBox(hx, (LPCTSTR)KAILLERA_SDLG, pDlg, (DLGPROC)KailleraServerDialogProc);
		//disconnect
		char quitmsg[128];
		GetWindowText(GetDlgItem(kaillera_ssdlg, IDC_QUITMSG), quitmsg, 128);
		kaillera_disconnect(quitmsg);
		kaillera_core_cleanup();

		KSSDFA.state = 0;
		KSSDFA.input = KSSDFA_END_GAME;
		//core cleanup
	} else {
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
		KLSListLv.FillRow(pingstr, 1, i);

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


LRESULT CALLBACK AboutDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_CLOSE:
		EndDialog(hDlg, 0);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case BTN_CLOSE:
			EndDialog(hDlg, 0);
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
void ShowMasterSLDialog(HWND hDlg) {
	DialogBox(hx, (LPCTSTR)KAILLERA_MLIST, hDlg, (DLGPROC)MasterSLDialogProc);
}

//////////////////////////////////////////

LRESULT CALLBACK KailleraServerSelectDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_INITDIALOG:
		{
			
			kaillera_ssdlg = hDlg;
			
			SetWindowText(hDlg, "n02 " KAILLERA_VERSION);
			
			nSettings::Initialize("SC");
			
			/*
			
			  SetDlgItemInt(hDlg, IDC_PORT, nSettings::get_int("IDC_PORT", 27886), false);
			  
				nSettings::get_str("IDC_IP", IP,"127.0.0.1:27886");
				SetDlgItemText(hDlg, IDC_IP, IP);
				
				  HWND hxx = GetDlgItem(hDlg, IDC_GAME);
				  if (gamelist != 0) {
				  nSettings::get_str("IDC_GAME", GAME, "");
				  char * xx = gamelist;
				  int p;
				  while ((p=strlen(xx))!= 0){
				  SendMessage(hxx, CB_ADDSTRING, 0, (WPARAM)xx);
				  if (strcmp(GAME, xx)==0) {
				  SetWindowText(hxx, GAME);
				  }
				  xx += p+ 1;
				  }
				  }
			*/
			
			{
				DWORD xxx = 32;
				char USERNAME[32];
				GetUserName(USERNAME, &xxx);
				char un[128];
				nSettings::get_str("USRN", un, USERNAME);
				strncpy(USERNAME, un, 32);
				SetWindowText(GetDlgItem(hDlg, IDC_USRNAME), USERNAME);
			}
			
			{
				DWORD xxx = 32;
				char QUITMSG[128] = "Open Kaillera - n02 " KAILLERA_VERSION;
				char un[128];
				nSettings::get_str("QMSG", un, QUITMSG);
				strncpy(QUITMSG, un, 128);
				SetWindowText(GetDlgItem(hDlg, IDC_QUITMSG), QUITMSG);
			}

			
			kaillera_ssdlg_conset = GetDlgItem(hDlg, CB_CONSET);
			SendMessage(kaillera_ssdlg_conset, CB_ADDSTRING, 0, (WPARAM)"LAN      (60 packets/s)");
			SendMessage(kaillera_ssdlg_conset, CB_ADDSTRING, 0, (WPARAM)"Excelent (30 packets/s)");
			SendMessage(kaillera_ssdlg_conset, CB_ADDSTRING, 0, (WPARAM)"Good     (20 packets/s)");
			SendMessage(kaillera_ssdlg_conset, CB_ADDSTRING, 0, (WPARAM)"Average  (15 packets/s)");
			SendMessage(kaillera_ssdlg_conset, CB_ADDSTRING, 0, (WPARAM)"Low      (12 packets/s)");
			SendMessage(kaillera_ssdlg_conset, CB_ADDSTRING, 0, (WPARAM)"Bad      (10 packets/s)");
			SendMessage(kaillera_ssdlg_conset, CB_SETCURSEL, nSettings::get_int("CNS", 0), 0);

			
			
			
			KLSListLv.handle = GetDlgItem(hDlg, LV_ULIST);
			KLSListLv.AddColumn("Name", 200);
			KLSListLv.AddColumn("IP", 180);
			KLSListLv.FullRowSelect();
			
			
			KLSListLoad();

			
			initialize_mode_cb(GetDlgItem(hDlg, CMB_MODE));
			
		}
		break;
	case WM_CLOSE:
		{
			char tbuf[128];
			GetWindowText(GetDlgItem(hDlg, IDC_QUITMSG), tbuf, 128);
			nSettings::set_str("QMSG", tbuf);
			
			GetWindowText(GetDlgItem(hDlg, IDC_USRNAME), tbuf, 128);
			nSettings::set_str("USRN", tbuf);

			nSettings::set_int("CNS", SendMessage(kaillera_ssdlg_conset, CB_GETCURSEL, 0, 0));
			
			
		}
		
		KLSListSave();
		
		EndDialog(hDlg, 0);
		nSettings::Terminate();
		
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
				KLSListConnect();
				break;
			case BTN_ABOUT:
				ShowAboutDialog(hDlg);
				break;
			case BTN_CUSTOM:
				ShowCustomIPDialog(hDlg);
				break;
			case BTN_MLIST:
				ShowMasterSLDialog(hDlg);
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
				KLSListConnect();
			}
			if(((LPNMHDR)lParam)->code==NM_RCLICK && ((LPNMHDR)lParam)->hwndFrom==KLSListLv.handle){
				KLSListPING();
			}

			
		/*
		NMHDR * nParam = (NMHDR*)lParam;
		if (nParam->hwndFrom == p2p_ui_modeseltab && nParam->code == TCN_SELCHANGE){
		nTab tabb;
		tabb.handle = p2p_ui_modeseltab;
		P2PSelectionDialogProcSetMode(hDlg, tabb.SelectedTab()!=0);	
		}
		if (nParam->hwndFrom == p2p_ui_storedusers.handle){
		if (nParam->code == NM_CLICK) {
		KLSListListSelect(hDlg);
		}
		}
			*/
			break;
		};
		return 0;
}











void kaillera_GUI(){
	INITCOMMONCONTROLSEX icx;
	icx.dwSize = sizeof(icx);
	icx.dwICC = ICC_LISTVIEW_CLASSES | ICC_TAB_CLASSES;
	InitCommonControlsEx(&icx);
	
	HMODULE p2p_riched_hm = LoadLibrary("riched32.dll");
	
	DialogBox(hx, (LPCTSTR)KAILLERA_SSDLG, 0, (DLGPROC)KailleraServerSelectDialogProc);
	
	FreeLibrary(p2p_riched_hm);
}