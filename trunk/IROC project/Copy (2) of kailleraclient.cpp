#include "kailleraclient.h"
#include <windows.h>
#include <time.h>
#include <cstdio>
#include <stdarg.h>
#include <signal.h>
#include <stdlib.h>
#include "string.h"


#include "p2p_ui.h"

#include "common/nThread.h"
#include "common/k_socket.h"


#define KAILLERA_DLLEXP __declspec(dllexport) __stdcall


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


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
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
		p2p_GUI();
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
	}
	void KAILLERA_DLLEXP kailleraShutdown(){
		p2p_core_cleanup();
		k_socket::Cleanup();
		if (gamelist != 0)
			free(gamelist);
		gamelist = 0;
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
				
				if (p2p_SelectServerDlgStep()){
					continue;
				}
				
				
			} 
			game_thread.sleep(1);
		}
		
	}
	
	int  KAILLERA_DLLEXP kailleraModifyPlayValues(void *values, int size){
		if (KSSDFA.state==2) {
			return p2p_modify_play_values(values, size);
		} else return -1;
	}
	void KAILLERA_DLLEXP kailleraChatSend(char *text){
		p2p_ui_chat_send(text);
	}
	void KAILLERA_DLLEXP kailleraEndGame(){
		p2p_EndGame();
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
