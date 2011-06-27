// Updated by Ownasaurus in June 2010

#include "kailleraclient.h"
#include <windows.h>
#include <time.h>
#include <cstdio>
#include <stdarg.h>
#include <signal.h>
#include <stdlib.h>
#include "string.h"


#include "p2p_ui.h"
#include "kaillera_ui.h"
#include "player.h"

#include "common/nThread.h"
#include "common/k_socket.h"


#define KAILLERA_DLLEXP __declspec(dllexport) __stdcall

#define KAILLERA

#define RECORDER

#define WINVER 0x0500
#define _WIN32_WINNT 0x0500

HINSTANCE hx;

void __cdecl kprintf(char * arg_0, ...) {
	char V8[1024];
	time_t V4 = time(0);
	tm * ecx = localtime(&V4);
	sprintf(V8, "%02d/%02d/%02d-%02d:%02d:%02d> %s\r\n",ecx->tm_mday,ecx->tm_mon,ecx->tm_year % 0x64,ecx->tm_hour,ecx->tm_min,ecx->tm_sec, arg_0);
	va_list args;
	va_start (args, arg_0);
	vprintf (V8, args);
	va_end (args);
}

void OutputHexx(const void * inb, int len, bool usespace){
	char outbb[2000];
	char * outb = outbb;
	char HEXDIGITS [] = {
		'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
	};
	char * xx = (char*)inb;
	for (int x = 0; x <	len; x++) {
		int dx = *xx++;
		int hib = (dx & 0xF0)>>4;
		int lob = (dx & 0x0F);
		*outb++ = HEXDIGITS[hib];
		*outb++ = HEXDIGITS[lob];
		if (usespace)
			*outb++ = ' ';
	}
	*outb = 0;
	printf(outbb);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

kailleraInfos infos_copy;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#ifdef KAILLERA
typedef struct {
	void (*GUI)();
	bool (*SSDSTEP)();
	int  (*MPV)(void*,int);
	void (*ChatSend)(char*);
	void (*EndGame)();
	bool (*RecordingEnabled)();
}n02_MODULE;

n02_MODULE active_mod;

n02_MODULE mod_kaillera;
n02_MODULE mod_p2p;
n02_MODULE mod_playback;
int mod_active_no = -1;
volatile bool mod_rerun;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#ifdef RECORDER
char convert[256];
HFILE out = HFILE_ERROR;

class RecordingBufferC {
public:
	char buffer[500];

	char* position;

	void reset(){
		position = buffer;
	}

	int len(){
		return position - buffer;
	}
	void put_char(char x){
		*position++ = x;
	}

	void put_short(short x){
		*(short*)position = x;
		position += 2;
	}

	void put_bytes(char* x, int len){
		for (int t=0;t<len;t++){
			*position++ = *x++;
		}
	}
	void write(){
		_lwrite(out, buffer, len());
		reset();
	}
}RecordingBuffer;

struct {
	






} recst;

void initialize_mode_cb(HWND hDlg){
	SendMessage(hDlg, CB_ADDSTRING, 0, (WPARAM)"1. P2P");
	SendMessage(hDlg, CB_ADDSTRING, 0, (WPARAM)"2. Client");
	SendMessage(hDlg, CB_ADDSTRING, 0, (WPARAM)"3. Playback");
	SendMessage(hDlg, CB_SETCURSEL, mod_active_no, 0);
}

bool activate_mode(int mode){
	if (mode != mod_active_no) {
		mod_active_no = mode;
		mod_rerun = true;

		if (mode == 0)
			active_mod = mod_p2p;
		else if (mode == 1)
			active_mod = mod_kaillera;
		else
			active_mod = mod_playback;

		return true;
	}
	return false;
}


int WINAPI _gameCallback(char *game, int player, int numplayers){

	if (out!=HFILE_ERROR) {
		_lclose(out);
		out = HFILE_ERROR;
	}

	if (active_mod.RecordingEnabled()) {

		RecordingBuffer.reset();
		
		//Setup Filename
		CreateDirectory(".\\records", 0);
		
		char FileName[500];
		wsprintf(FileName,".\\records\\%s[%i].krec", game, time(NULL));
		
		//StatusOutput.out("started recording new file...");
		//StatusOutput.out(FileName);		
		
		
		for(unsigned int x=0; x < strlen(FileName); x++){
			FileName[x] = convert[FileName[x]];
		}
		
		//open file
		OFSTRUCT of;
		out = OpenFile(FileName, &of, OF_WRITE|OF_CREATE);
		
		if(out==HFILE_ERROR){
			//StatusOutput.out("Failed creating file");
		}
		
		
		union {
			
			char GameName[128];
			int timee;
			
		};
		//GameName[128] = '\0'; //rs
		strncpy(GameName, game, 127); //rs 
		
		_lwrite(out, "KRC0", 4);
		_lwrite(out, infos_copy.appName, 128);
		_lwrite(out, GameName, 128);
		timee = time(NULL);
		_lwrite(out, (char*)&timee, 4);
		_lwrite(out, (char*)&player, 4);
		_lwrite(out, (char*)&numplayers, 4);

	}

	return infos_copy.gameCallback(game, player, numplayers);
	
}

void WINAPI _chatReceivedCallback(char *nick, char *text){
	if (out != HFILE_ERROR) {
		RecordingBuffer.put_char(8);
		RecordingBuffer.put_bytes(nick, strlen(nick)+1);
		RecordingBuffer.put_bytes(text, strlen(text)+1);
	}
	if (infos_copy.chatReceivedCallback)
		infos_copy.chatReceivedCallback(nick, text);
}
void WINAPI _clientDroppedCallback(char *nick, int playernb){
	if (out != HFILE_ERROR) {
		RecordingBuffer.put_char(20);
		RecordingBuffer.put_bytes(nick, strlen(nick)+1);
		RecordingBuffer.put_bytes((char*)&playernb, 4);
	}
	if (infos_copy.clientDroppedCallback)
		infos_copy.clientDroppedCallback(nick, playernb);
}




#endif
#endif
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
char * gamelist = 0;




int KSSDFAST [16] = {
	0, 0, 1, 1,
		1, 0, 1, 0,
		2, 0, 2, 0,
		3, 3, 3, 3
};

KSSDFA_ KSSDFA;


class GuiThread : public nThread {
public:
	bool running;
	void run(void) {
		
		
#ifdef KAILLERA

		/*

		BYTE kst[256];
		GetKeyboardState(kst);
		
		if (kst[VK_CAPITAL]==0)
			active_mod = mod_p2p;
		else
			active_mod = mod_kaillera;

		*/

		mod_rerun = true;

		while (mod_rerun) {
			mod_rerun = false;
			active_mod.GUI();
		}

#else
		p2p_GUI();
#endif
		
		//*** THIS EDIT STOPPED MOST CRASHES FOR SOME REASON***
		Sleep(100);
		// TODO: Find the real reason the crashes are occuring and fix it!
		// It is most likely a thread synchronization issue given what this function does
		// This is also probably the key to fixing the bug where you have to "end task"

		KSSDFA.state = 3;
		//*/
	}
	void st(void * parent) {
		KSSDFA.state = create()!=0? 0:3;
	}
} gui_thread;



extern "C" {
	kailleraInfos infos;
	
	void KAILLERA_DLLEXP kailleraGetVersion(char *version){
		memcpy(version, "0.9", 4);
	}
	void KAILLERA_DLLEXP kailleraInit(){
		k_socket::Initialize();
		
#ifdef KAILLERA

		mod_playback.MPV = player_MPV;
		mod_playback.GUI = player_GUI;
		mod_playback.EndGame = player_EndGame;
		mod_playback.SSDSTEP = player_SSDSTEP;
		mod_playback.ChatSend = player_ChatSend;
		mod_playback.RecordingEnabled = player_RecordingEnabled;
		
		mod_kaillera.GUI = kaillera_GUI;
		mod_kaillera.SSDSTEP = kaillera_SelectServerDlgStep;
		mod_kaillera.MPV = kaillera_modify_play_values;
		mod_kaillera.ChatSend = kaillera_game_chat_send;
		mod_kaillera.EndGame = kaillera_end_game;
		mod_kaillera.RecordingEnabled = kaillera_RecordingEnabled;
		
		mod_p2p.GUI = p2p_GUI;
		mod_p2p.SSDSTEP = p2p_SelectServerDlgStep;
		mod_p2p.MPV = p2p_modify_play_values;
		mod_p2p.ChatSend = p2p_send_chat;
		mod_p2p.EndGame = p2p_EndGame;
		mod_p2p.RecordingEnabled = p2p_RecordingEnabled;

		activate_mode(1);

#ifdef RECORDER


		{
			for(int x = 0 ; x < 256; x++){
				convert[x] = '_';
			}
		}
		{
			for(char x = 'A' ; x <= 'Z'; x++){
				convert[x] = x;
			}
		}
		{
			for(char x = '0' ; x <= '9'; x++){
				convert[x] = x;
			}
		}
		{
			for(char x = 'a' ; x <= 'z'; x++){
				convert[x] = x;
			}
		}
			convert['['] = '[';
			convert[']'] = ']';
			convert['.'] = '.';
			convert['\\'] = '\\';



#endif



#endif
		
		
	}
	void KAILLERA_DLLEXP kailleraShutdown(){
		
		//p2p_core_cleanup();
		k_socket::Cleanup();
		if (gamelist != 0)
			free(gamelist);
		gamelist = 0;
#ifdef KAILLERA
#ifdef RECORDER
		if (out!=HFILE_ERROR) {
			_lclose(out);
			out = HFILE_ERROR;
#endif
#endif
	}
		
	}
	void KAILLERA_DLLEXP kailleraSetInfos(kailleraInfos *infos_){
		infos = *infos_;
		strncpy(APP, infos.appName, 127);
		
		if (gamelist != 0)
			free(gamelist);
		gamelist = 0;
		
		char * xx = infos.gameList;
		int l = 0;
		if (xx != 0) {
			int p;
			while ((p=strlen(xx))!= 0){
				l += p + 1;
				xx += p+ 1;
			}
			l++;
			gamelist = (char*)malloc(l);
			memcpy(gamelist, infos.gameList, l);
		}
		infos.gameList = gamelist;
		infos_copy = infos;
#ifdef KAILLERA
#ifdef RECORDER
		infos.chatReceivedCallback = _chatReceivedCallback;
		infos.clientDroppedCallback = _clientDroppedCallback;
		infos.gameCallback = _gameCallback;
#endif
#endif
	}
	void KAILLERA_DLLEXP kailleraSelectServerDialog(void* parent){
		KSSDFA.state = 0;
		KSSDFA.input = 0;
		
		gui_thread.st(parent);
		
		nThread game_thread;
		game_thread.capture();
		game_thread.sleep(1);
		
		while (KSSDFA.state != 3){
			KSSDFA.state = KSSDFAST[((KSSDFA.state)<<2) | KSSDFA.input];
			KSSDFA.input = 0;
			if (KSSDFA.state == 2){
				MSG message;
				while (PeekMessage(&message, 0, 0, 0, PM_REMOVE)){
					TranslateMessage(&message);
					DispatchMessage(&message);
				}
			} else if (KSSDFA.state == 1){
				kprintf("call gamecallback");
				KSSDFA.state = 2;
				infos.gameCallback(GAME, playerno, numplayers);
			} else if (KSSDFA.state == 0) {
				MSG message;
				while (PeekMessage(&message, 0, 0, 0, PM_REMOVE)){
					TranslateMessage(&message);
					DispatchMessage(&message);
				}
#ifndef KAILLERA
				if (p2p_SelectServerDlgStep()){
#else
				if (active_mod.SSDSTEP()){
#endif
					continue;
				}
				
			} 
			game_thread.sleep(1);
		}
	}
	
	int  KAILLERA_DLLEXP kailleraModifyPlayValues(void *values, int size){
		if (KSSDFA.state==2) {
#ifdef KAILLERA

#ifdef RECORDER

			short siz = active_mod.MPV(values, size);

			if (out != HFILE_ERROR) {
				RecordingBuffer.put_char(0x12);
				RecordingBuffer.put_short(siz);
				RecordingBuffer.put_bytes((char*)values, siz);
				RecordingBuffer.write();
			}

			return siz;

#else
			return active_mod.MPV(values, size);
#endif
#else
			return p2p_modify_play_values(values, size);
#endif
		} else return -1;
	}
	void KAILLERA_DLLEXP kailleraChatSend(char *text){
#ifdef KAILLERA
		active_mod.ChatSend(text);
#else
		p2p_ui_chat_send(text);
#endif
	}
	void KAILLERA_DLLEXP kailleraEndGame(){
#ifdef KAILLERA
		active_mod.EndGame();
#else
		p2p_EndGame();
#endif
		
	}
};


BOOL APIENTRY DllMain (HINSTANCE hInst,
	DWORD reason,
	LPVOID){
	if(reason==DLL_PROCESS_ATTACH)
		hx = hInst;
	return TRUE;
}


extern "C" {
	__declspec(dllexport) int z00_With = 0;
	__declspec(dllexport) int z01_stupidity = 0;
	__declspec(dllexport) int z02_even = 0;
	__declspec(dllexport) int z03_the = 0;
	__declspec(dllexport) int z04_gods = 0;
	__declspec(dllexport) int z05_contend = 0;
	__declspec(dllexport) int z06_in = 0;
	__declspec(dllexport) int z07_vain = 0;
}
