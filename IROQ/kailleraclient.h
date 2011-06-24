#pragma once
#include <windows.h>
#include "common/k_framecache.h"

extern int PACKETLOSSCOUNT;



typedef struct {
	unsigned state:2;
	unsigned input:2;
}KSSDFA_;

extern KSSDFA_ KSSDFA;

#define KSSDFA_START_GAME 2
#define KSSDFA_END_GAME 1

/* KSSDFAST:
 + | 00 | 01 | 11 | 10 |
---+----+----+----+----|
00 | 00 | 00 | 01 | 01 |
01 | 01 | 00 | 00 | 01 |
11 | 11 | 11 | 11 | 11 |
10 | 10 | 00 | 00 | 10 |
*/

extern int KSSDFAST [16];


#include "resource.h"

extern HINSTANCE hx;

void __cdecl kprintf(char * arg_0, ...);
extern char * gamelist;


extern "C" {
	
	typedef struct {
		char *appName;
		char *gameList;
		int (__stdcall *gameCallback)(char *game, int player, int numplayers);
		void (__stdcall *chatReceivedCallback)(char *nick, char *text);
		void (__stdcall *clientDroppedCallback)(char *nick, int playernb);
		void (__stdcall *moreInfosCallback)(char *gamename);
	}kailleraInfos;
	
	extern kailleraInfos infos;
	
};

//extern char GAME[128];
extern char GAME[150];
extern char APP[128];
extern int playerno;
extern int numplayers;

void initialize_mode_cb(HWND hDlg);
bool activate_mode(int mode);
