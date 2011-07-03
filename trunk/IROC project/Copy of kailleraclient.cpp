#include "kailleraclient.h"
#include <windows.h>
#include <time.h>
#include <cstdio>
#include <stdarg.h>
#include <signal.h>
#include <stdlib.h>
#include "string.h"
#include "core/p2p_core.h"
#include "common/nThread.h"
#include "common/k_socket.h"
#include "richedit.h"


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

void OutputHex(char * outb, const void * inb, int len, int res1, bool usespace){
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

HWND riched;
void outp(char * line){
	//kprintf(line);
	int i = strlen(line);
	CHARRANGE cr;
	GETTEXTLENGTHEX gtx;
	gtx.codepage = CP_ACP;
	gtx.flags = GTL_PRECISE;
	cr.cpMin = GetWindowTextLength(riched);//SendMessage(riched, EM_GETTEXTLENGTHEX, (WPARAM)&gtx, 0);
	cr.cpMax = cr.cpMin;//+i;
	SendMessage(riched, EM_EXSETSEL, 0, (LPARAM)&cr);
	SendMessage(riched, EM_REPLACESEL, FALSE, (LPARAM)line);
	SendMessage(riched, WM_VSCROLL, SB_BOTTOM, 0);
}
void __cdecl p2p_core_debug(char * arg_0, ...) {
	char V8[1024];
	char V88[2084];
	sprintf(V8, "%s\r\n", arg_0);
	va_list args;
	va_start (args, arg_0);
	vsprintf (V88, V8, args);
	va_end (args);
	outp(V88);
}

void __cdecl outpf(char * arg_0, ...) {
	char V8[1024];
	char V88[2084];
	sprintf(V8, "%s\r\n", arg_0);
	va_list args;
	va_start (args, arg_0);
	vsprintf (V88, V8, args);
	va_end (args);
	outp(V88);
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



int PACKETLOSSCOUNT;
int PACKETMISOTDERCOUNT;
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

bool HOST;
char GAME[128];
char APP[128];
char IP[128];
char USERNAME[128];
int playerno;
int numplayers;
HWND hdl;
int PORT;
bool COREINIT = false;



void CDialogProc_Init(HWND hDlg){
	
	hdl = hDlg;
	
	if (p2p_core_initialize(HOST, PORT, APP, GAME, USERNAME)){
		
		ShowWindow(GetDlgItem(hDlg, IDC_ADDDELAY), HOST? SW_SHOW:SW_HIDE);
		
		if (HOST) {
			outpf("Hosting %s on port %i", GAME, p2p_core_get_port());
		} else {

			char * host;
			host = IP;
			int port = 27886;
			while ((*++host != ':' || *++host != ' ') && *host != 0);
			if (*host == ':' || *host == ' ') {
				*host++ = 0x00;
				port = atoi(host);
				port = port==0?27886:port;
			}
			host = IP;
			
			outpf("Connecting to %s:%i", host, port);

			if (!p2p_core_connect(host, port)){

				MessageBox(hDlg, "Error connecting to specified host/port", host, 0);
				EndDialog(hDlg, 0);
				return;
			}

		}
		
		COREINIT = true;
	} else outpf("Error initializing sockets");
	


}

int PING_TIME;


void CDialogProc_PrintCoreStat(){
	
	outpf("============= Core status begin ===============");

	p2p_print_core_status();

	unsigned int sta =  KSSDFA.state;
	unsigned int inp = KSSDFA.input;
	outpf("KSSDFA { state: %i, input: %i }", sta, inp);
	outpf("PACKETLOSSCOUNT=%u", PACKETLOSSCOUNT);
	outpf("PACKETMISOTDERCOUNT=%u", PACKETMISOTDERCOUNT);
	outpf("============ Core status end =================");
	
}


void CDialogProc_SendChat(char * xxx);


bool BDialogProc_Exit(){
	return p2p_disconnect();
}




void CDialogDroPGame(){
	p2p_drop_game();
}













///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
HWND cctl;
LRESULT CALLBACK CDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_INITDIALOG:
		{
			riched = GetDlgItem(hDlg, IDC_RICHEDIT2);
			cctl = GetDlgItem(hDlg, IDC_CHATI);
			CDialogProc_Init(hDlg);			
		}
		
		break;
	case WM_CLOSE:
		//kprintf(__FILE__ ":%i", __LINE__);//localhost:27888
		if (BDialogProc_Exit()){
			kprintf(__FILE__ ":%i", __LINE__);
			EndDialog(hDlg, 0);
			p2p_core_cleanup();
		}
		break;
	case WM_COMMAND:
		////kprintf(__FILE__ ":%i", __LINE__);//localhost:27888
		switch (LOWORD(wParam)) {
		case IDC_CHAT:
			{
				char xxx[256];
				GetWindowText(cctl, xxx, 255);
				CDialogProc_SendChat(xxx);
				SetWindowText(cctl, "");
			}
			break;
		case IDC_RETR:
			{
				CDialogProc_SendChat(":retr");
			}
			break;
		case IDC_PING:
			{
				CDialogProc_SendChat(":ping");
			}
			break;
		case IDC_DROPGAME:
			CDialogDroPGame();
			break;
		case IDC_READY:
			{
				p2p_set_ready(SendMessage(GetDlgItem(hDlg, IDC_READY), BM_GETCHECK, 0, 0)==BST_CHECKED);
			}
			break;
		};
		break;
	};	
	return 0;
}


HMODULE hm;

void StratC(HWND hDlg, bool host);
LRESULT CALLBACK DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);


class GuiThread : public nThread {
public:
	bool running;
	void run(void) {
		DialogBox(hx, (LPCTSTR)MAIN_DIALOG, 0, (DLGPROC)DialogProc);
		KSSDFA.state = 3;
		//*/
	}
	void st(void * parent) {
		KSSDFA.state = create()!=0? 0:3;
	}
} gui_thread;
extern "C" {
	
	typedef struct {
		char *appName;
		char *gameList;
		int (__stdcall *gameCallback)(char *game, int player, int numplayers);
		void (__stdcall *chatReceivedCallback)(char *nick, char *text);
		void (__stdcall *clientDroppedCallback)(char *nick, int playernb);
		void (__stdcall *moreInfosCallback)(char *gamename);
	}kailleraInfos;
	
	kailleraInfos infos;
	
};

void CDialogProc_SendChat(char * xxx){
	if (strcmp(xxx, ":pcs")==0) {
		CDialogProc_PrintCoreStat();
		return;
	}
	if (p2p_is_connected()) {
		if (*xxx == ':') {
			xxx++;
			if (strcmp(xxx, "ping")==0) {
				p2p_ping();
			} else if (strcmp(xxx, "retr")==0) {
				p2p_retransmit();
			} else {
				outpf("Unknown command \"%s\"", xxx);
				p2p_send_chat(xxx-1);
			}
		} else {
			p2p_send_chat(xxx);
		}
	}
}







void p2p_chat_callback(char * nick, char * msg){
	outpf("<%s> %s",nick, msg);
	if (KSSDFA.state==2 && infos.chatReceivedCallback) {
		infos.chatReceivedCallback(nick, msg);
	}
}

bool p2p_add_delay_callback(){
	return SendMessage(GetDlgItem(hdl, IDC_ADDDELAY), BM_GETCHECK, 0, 0)==BST_CHECKED;
	//return true;
}

void p2p_game_callback(char * game, int playerno, int maxplayers){
	KSSDFA.input = KSSDFA_START_GAME;
}

void p2p_end_game_callback(){
	KSSDFA.input = KSSDFA_END_GAME;
	KSSDFA.state = 0;
	SendMessage(GetDlgItem(hdl, IDC_READY), BM_SETCHECK, BST_UNCHECKED, 0);
}

void p2p_client_dropped_callback(char * nick, int playerno){
	if (infos.clientDroppedCallback) {
		infos.clientDroppedCallback(nick, playerno);
	}
}

void HandleChat(char * nick, char * msg){
	outpf("<%s> %s",nick, msg);
	if (KSSDFA.state == 2 && infos.chatReceivedCallback) {
		infos.chatReceivedCallback(nick, msg);
	}
}

extern "C" {
	
	
	void KAILLERA_DLLEXP kailleraGetVersion(char *version){
		memcpy(version, "0.9", 4);
		//kprintf("kailleraGetVersion");
	}
	void KAILLERA_DLLEXP kailleraInit(){
		//kprintf("kailleraInit");
		k_socket::Initialize();
	}
	void KAILLERA_DLLEXP kailleraShutdown(){
		//kprintf("kailleraShutdown");
		p2p_core_cleanup();
		k_socket::Cleanup();
		if (gamelist != 0)
			free(gamelist);
		gamelist = 0;
	}
	void KAILLERA_DLLEXP kailleraSetInfos(kailleraInfos *infos_){
		//kprintf("kailleraSetInfos");
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
		//kprintf("kailleraSelectServerDialog");
		KSSDFA.state = 0;
		KSSDFA.input = 0;
		
		gui_thread.st(parent);
		
		nThread game_thread;
		game_thread.capture();
		game_thread.sleep(1);
		
		while (KSSDFA.state != 3){
			KSSDFA.state = KSSDFAST[((KSSDFA.state)<<2) | KSSDFA.input];
			if (KSSDFA.state == 0 && KSSDFA.input == KSSDFA_END_GAME)
				p2p_retransmit();
			KSSDFA.input = 0;
			if (KSSDFA.state == 2){
				MSG message;
				while (PeekMessage(&message, 0, 0, 0, PM_REMOVE)){
					TranslateMessage(&message);
					DispatchMessage(&message);
				}
				//Sleep(100);
			} else if (KSSDFA.state == 1){
				kprintf("call gamecallback");
				KSSDFA.state = 2;
				infos.gameCallback(GAME, playerno, numplayers);
			} else if (KSSDFA.state == 0) {
				//outp(".");
				MSG message;
				while (PeekMessage(&message, 0, 0, 0, PM_REMOVE)){
					TranslateMessage(&message);
					DispatchMessage(&message);
				}
				
				if (COREINIT) {
					p2p_step();
					continue;
				}
				
			} 
			game_thread.sleep(1);
		}
		
	}
	
	
	int  KAILLERA_DLLEXP kailleraModifyPlayValues(void *values, int size){
		//kprintf("kailleraModifyPlayValues");
		if (KSSDFA.state==2) {
			return p2p_modify_play_values(values, size);
		} else return -1;
	}
	void KAILLERA_DLLEXP kailleraChatSend(char *text){
		p2p_send_chat(text);
	}
	void KAILLERA_DLLEXP kailleraEndGame(){
		outpf("dropping game");
		p2p_drop_game();
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
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

