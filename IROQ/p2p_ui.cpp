// Updated by Ownasaurus in June 2010

#include "kailleraclient.h"

#include "common/nSettings.h"
#include "p2p_ui.h"
#include <windows.h>
#include <stdarg.h>
#include "string.h"
#include "common/nSTL.h"


#include "uihlp.h"

extern HINSTANCE hx;

void LoadWindowPos(HWND hwnd);
void SaveWindowPos(HWND hwnd);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void outp(char * line);
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
bool HOST;
//char GAME[128];
char GAME[150];
char APP[128];
int playerno;
int numplayers;
char IP[128];
char USERNAME[128];
int PORT;
///////////////////////////////////////////////////////////////////////////////
bool COREINIT = false;
int PING_TIME;
///////////////////////////////////////////////////////////////////////////////
HWND p2p_ui_connection_dlg;
HWND p2p_ui_con_chatinp;
HWND p2p_ui_con_richedit;
HWND p2p_ui_modeseltab;
nLVw p2p_ui_storedusers;
HWND p2p_ui_ss_dlg;
///////////////////////////////////////////////////////////////////////////////
HWND p2p_ui_con_delay;






void p2p_ui_pcs(){
	
	outpf("============= Core status begin ===============");

	p2p_print_core_status();

	unsigned int sta =  KSSDFA.state;
	unsigned int inp = KSSDFA.input;
	outpf("KSSDFA { state: %i, input: %i }", sta, inp);
	outpf("PACKETLOSSCOUNT=%u", PACKETLOSSCOUNT);
	outpf("PACKETMISOTDERCOUNT=%u", PACKETMISOTDERCOUNT);
	outpf("============ Core status end =================");
	
}




void p2p_ping_callback(int PING){
	char buf[200];
	wsprintf(buf, "ping: %i ms pl: %i", PING, PACKETLOSSCOUNT);
	SetWindowText(GetDlgItem(p2p_ui_connection_dlg, SA_PST), buf);
}



void p2p_chat_callback(char * nick, char * msg){
	outpf("<%s> %s",nick, msg);
	if (KSSDFA.state==2 && infos.chatReceivedCallback) {
		infos.chatReceivedCallback(nick, msg);
	}
}

bool p2p_add_delay_callback(){
	return SendMessage(GetDlgItem(p2p_ui_connection_dlg, IDC_ADDDELAY), BM_GETCHECK, 0, 0)==BST_CHECKED;
	//return true;
}
void p2p_game_callback(char * game, int playernop, int maxplayersp){
	//GAME[150] = '\0'; //rs
	strcpy(GAME, game); //rs 
	playerno = playernop;
	numplayers = maxplayersp;
	KSSDFA.input = KSSDFA_START_GAME;
}

void p2p_end_game_callback(){
	KSSDFA.input = KSSDFA_END_GAME;
	KSSDFA.state = 0;
	SendMessage(GetDlgItem(p2p_ui_connection_dlg, IDC_READY), BM_SETCHECK, BST_UNCHECKED, 0);
}

void p2p_client_dropped_callback(char * nick, int playerno){
	if (infos.clientDroppedCallback) {
		infos.clientDroppedCallback(nick, playerno);
	}
}


void p2p_ui_chat_send(char * xxx){
	if (strcmp(xxx, ":pcs")==0) {
		p2p_ui_pcs();
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

void p2p_EndGame(){
	outpf("dropping game");
	p2p_drop_game();
}
bool p2p_SelectServerDlgStep(){
	if (COREINIT) {
		p2p_step();
		return true;
	}
	return false;
}



bool p2p_RecordingEnabled(){
	return SendMessage(GetDlgItem(p2p_ui_connection_dlg, CHK_REC), BM_GETCHECK, 0, 0)==BST_CHECKED;
}



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
void IniaialzeConnectionDialog(HWND hDlg){
	
	p2p_ui_connection_dlg = hDlg;
	
	if (p2p_core_initialize(HOST, PORT, APP, GAME, USERNAME)){
		
		ShowWindow(GetDlgItem(hDlg, IDC_ADDDELAY), HOST? SW_SHOW:SW_HIDE);
		
		if (HOST) {
			outpf("Hosting %s on port %i", GAME, p2p_core_get_port());
			outpf("WARNING: Hosting requires hosting ports to be forwarded and enabled in firewalls.");
		} else {

			char * host;
			host = IP;
			int port = 27886;
			while (*++host != ':' && *host != 0);
			if (*host == ':') {
				*host++ = 0x00;
				port = atoi(host);
				port = port==0?27886:port;
			}
			host = IP;
//			76.81.211.10:27886
			outpf("Connecting to %s:%i", host, port);

			if (!p2p_core_connect(host, port)){

				MessageBox(hDlg, "Error connecting to specified host/port", host, 0);
				//EndDialog(hDlg, 0);
				return;
			}


		}
		
		COREINIT = true;

	} else outpf("Error initializing sockets");
	


}


void outp(char * line){
	//kprintf(line);
	int i = strlen(line);
	CHARRANGE cr;
	GETTEXTLENGTHEX gtx;
	gtx.codepage = CP_ACP;
	gtx.flags = GTL_PRECISE;
	cr.cpMin = GetWindowTextLength(p2p_ui_con_richedit);//SendMessage(p2p_ui_con_richedit, EM_GETTEXTLENGTHEX, (WPARAM)&gtx, 0);
	cr.cpMax = cr.cpMin;//+i;
	SendMessage(p2p_ui_con_richedit, EM_EXSETSEL, 0, (LPARAM)&cr);
	SendMessage(p2p_ui_con_richedit, EM_REPLACESEL, FALSE, (LPARAM)line);
	SendMessage(p2p_ui_con_richedit, WM_VSCROLL, SB_BOTTOM, 0);
}



LRESULT CALLBACK ConnectionDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_INITDIALOG:
		{
			p2p_ui_con_richedit = GetDlgItem(hDlg, IDC_RICHEDIT2);
			p2p_ui_con_chatinp = GetDlgItem(hDlg, IDC_CHATI);
			p2p_ui_con_delay = GetDlgItem(hDlg, IDC_CHATI2);
			SetFocus(GetDlgItem(hDlg,IDC_CHATI));
			SendMessage(p2p_ui_con_richedit,EM_AUTOURLDETECT,true,0);
			SendMessage(p2p_ui_con_richedit,EM_SETEVENTMASK,0,ENM_LINK);

			LoadWindowPos(hDlg);

			IniaialzeConnectionDialog(hDlg);
		}
		break;
	case WM_SIZE:
		// Here we go again with the dynamic resizing code!
		if (wParam == SIZE_RESTORED || wParam == SIZE_MAXIMIZED) // what we care about
		{
			int iWidth, iHeight;
			iWidth = LOWORD(lParam);
			iHeight = HIWORD(lParam);
			int hTxtBox = 20; // absolute pixels

			// top half
			SetWindowPos(GetDlgItem(hDlg, IDC_RICHEDIT2),HWND_TOP,0,0,iWidth,(int)(0.75*iHeight),0);
			SetWindowPos(GetDlgItem(hDlg, IDC_CHATI),HWND_TOP,0,(int)(0.75*iHeight),iWidth-62,hTxtBox,0);
			SetWindowPos(GetDlgItem(hDlg, IDC_CHAT),HWND_TOP,iWidth-62,(int)(0.75*iHeight),0,0,SWP_NOSIZE);

			// bottom half
			SetWindowPos(GetDlgItem(hDlg, IDC_READY),HWND_TOP,0,(int)(0.75*iHeight)+hTxtBox+3,0,0,SWP_NOSIZE);
			SetWindowPos(GetDlgItem(hDlg, IDC_ADDDELAY),HWND_TOP,150,(int)(0.75*iHeight)+hTxtBox+3,0,0,SWP_NOSIZE);

			SetWindowPos(GetDlgItem(hDlg, IDC_CHATI2),HWND_TOP,0,(int)(0.75*iHeight)+hTxtBox+23,0,0,SWP_NOSIZE);
			SetWindowPos(GetDlgItem(hDlg, IDC_DELAY),HWND_TOP,105,(int)(0.75*iHeight)+hTxtBox+25,0,0,SWP_NOSIZE);
			SetWindowPos(GetDlgItem(hDlg, SA_PST),HWND_TOP,350,(int)(0.75*iHeight)+hTxtBox+25,0,0,SWP_NOSIZE);
			
			SetWindowPos(GetDlgItem(hDlg, CHK_REC),HWND_TOP,0,(int)(0.75*iHeight)+hTxtBox+50,0,0,SWP_NOSIZE);
			SetWindowPos(GetDlgItem(hDlg, IDC_RETR),HWND_TOP,105,(int)(0.75*iHeight)+hTxtBox+50,0,0,SWP_NOSIZE);
			SetWindowPos(GetDlgItem(hDlg, IDC_PING),HWND_TOP,150,(int)(0.75*iHeight)+hTxtBox+50,0,0,SWP_NOSIZE);
			SetWindowPos(GetDlgItem(hDlg, IDC_DROPGAME),HWND_TOP,200,(int)(0.75*iHeight)+hTxtBox+50,0,0,SWP_NOSIZE);

			// force redraw to eliminate artifacts
			InvalidateRect(hDlg,NULL,false);
		}
		break;
	case WM_CLOSE: case WM_QUIT: case WM_DESTROY:
		//kprintf(__FILE__ ":%i", __LINE__);//localhost:27888
		SaveWindowPos(hDlg); // this is being called more than once =( but oh well
		if (p2p_disconnect()){
			kprintf(__FILE__ ":%i", __LINE__);
			EndDialog(hDlg, 0);
			p2p_core_cleanup();
		}
		KSSDFA.state = 0;
		SetFocus(GetDlgItem(p2p_ui_ss_dlg,CMB_MODE));
		break;
	case WM_COMMAND:
		////kprintf(__FILE__ ":%i", __LINE__);//localhost:27888
		switch (LOWORD(wParam)) {
		case IDC_CHAT:
			{
				char xxx[251];
				GetWindowText(p2p_ui_con_chatinp, xxx, 251);
				p2p_ui_chat_send(xxx);
				SetWindowText(p2p_ui_con_chatinp, "");
			}
			break;
		case IDC_RETR:
			{
				p2p_ui_chat_send(":retr");
			}
			break;
		case IDC_PING:
			{
				p2p_ui_chat_send(":ping");
			}
			break;
		case IDC_DROPGAME:
			p2p_drop_game();
			break;
		case IDC_READY:
			{
				p2p_set_ready(SendMessage(GetDlgItem(hDlg, IDC_READY), BM_GETCHECK, 0, 0)==BST_CHECKED);
			}
			break;
		};
		break;
	case WM_NOTIFY:
		NMHDR* pNmhdr = (NMHDR*)lParam;
		ENLINK * tEN = (ENLINK *)pNmhdr;
		if(pNmhdr->code==EN_LINK && tEN->msg == WM_LBUTTONDOWN) // hyperlink stuff again
		{
			TEXTRANGE g;
			char tLink[1024];

			g.chrg = tEN->chrg;
			g.lpstrText = tLink;

			SendMessage(((LPNMHDR)lParam)->hwndFrom, EM_GETTEXTRANGE, 0, (LPARAM) &g);
			ShellExecute(((LPNMHDR)lParam)->hwndFrom, NULL, tLink, NULL, NULL, SW_SHOWNORMAL);
			tEN->msg = 0;
		}
		break;
	};	
	return 0;
}
///////////////////////////////////////////////////

void InitializeP2PSubsystem(HWND hDlg, bool host){

	HOST = host;

	
	if (!host && SendMessage(GetDlgItem(hDlg, IDC_CLIENTRANDOM), BM_GETCHECK, 0, 0)==BST_CHECKED){
		PORT = 0;
	} else {
		PORT = GetDlgItemInt(hDlg, IDC_PORT, 0, FALSE);
	}

	GetWindowText(GetDlgItem(hDlg, IDC_GAME), GAME, 127);
	GetWindowText(GetDlgItem(hDlg, IDC_IP), IP, 127);
	GetWindowText(GetDlgItem(hDlg, IDC_USRNAME), USERNAME, 31);


	//kprintf(__FILE__ ":%i", __LINE__);//localhost:27888

	if (HOST) {
		if (gamelist != 0) {
			char * xx = gamelist;
			int p;
			while ((p=strlen(xx))!= 0){
				if (strcmp(xx, GAME)==0){
					kprintf(__FILE__ ":%i", __LINE__);//localhost:27888
					ShowWindow(hDlg, SW_HIDE);
					DialogBox(hx, (LPCTSTR)CONNECTION_DIALOG, /*hDlg*/NULL, (DLGPROC)ConnectionDialogProc);
					ShowWindow(hDlg, SW_SHOW);
					return;
				}
				xx += p+ 1;
			}
		}
		MessageBox(hDlg, "Pick a valid game", 0,0);
		return;
	}

	DialogBox(hx, (LPCTSTR)CONNECTION_DIALOG, /*hDlg*/NULL, (DLGPROC)ConnectionDialogProc);

}


///////////////////////////////////////////////////////////////
#define P2PSelectionDialogProcSetModee(hDlg, ITEM, Mode) ShowWindow(GetDlgItem(hDlg, ITEM), Mode);

void P2PSelectionDialogProcSetMode(HWND hDlg, bool connector){

	if (connector){

		P2PSelectionDialogProcSetModee(hDlg, IDC_ULIST, SW_SHOW);
		P2PSelectionDialogProcSetModee(hDlg, IDC_CONNECT, SW_SHOW);
		P2PSelectionDialogProcSetModee(hDlg, IDC_ULIST, SW_SHOW);		
		P2PSelectionDialogProcSetModee(hDlg, IDC_ADD, SW_SHOW);
		P2PSelectionDialogProcSetModee(hDlg, IDC_EDIT, SW_SHOW);
		P2PSelectionDialogProcSetModee(hDlg, IDC_DELETE, SW_SHOW);
		P2PSelectionDialogProcSetModee(hDlg, IDC_IP, SW_SHOW);
		P2PSelectionDialogProcSetModee(hDlg, IDC_PIPL, SW_SHOW);
		P2PSelectionDialogProcSetModee(hDlg, IDC_STOREDL, SW_SHOW);		
		P2PSelectionDialogProcSetModee(hDlg, IDC_GAMEL, SW_HIDE);
		P2PSelectionDialogProcSetModee(hDlg, IDC_GAME, SW_HIDE);
		P2PSelectionDialogProcSetModee(hDlg, IDC_HOST, SW_HIDE);
		P2PSelectionDialogProcSetModee(hDlg, IDC_MYIP, SW_HIDE);

	} else {


		P2PSelectionDialogProcSetModee(hDlg, IDC_ULIST, SW_HIDE);
		P2PSelectionDialogProcSetModee(hDlg, IDC_CONNECT, SW_HIDE);
		P2PSelectionDialogProcSetModee(hDlg, IDC_ULIST, SW_HIDE);		
		P2PSelectionDialogProcSetModee(hDlg, IDC_ADD, SW_HIDE);
		P2PSelectionDialogProcSetModee(hDlg, IDC_EDIT, SW_HIDE);
		P2PSelectionDialogProcSetModee(hDlg, IDC_DELETE, SW_HIDE);
		P2PSelectionDialogProcSetModee(hDlg, IDC_IP, SW_HIDE);
		P2PSelectionDialogProcSetModee(hDlg, IDC_PIPL, SW_HIDE);
		P2PSelectionDialogProcSetModee(hDlg, IDC_STOREDL, SW_HIDE);
		P2PSelectionDialogProcSetModee(hDlg, IDC_GAMEL, SW_SHOW);
		P2PSelectionDialogProcSetModee(hDlg, IDC_GAME, SW_SHOW);
		P2PSelectionDialogProcSetModee(hDlg, IDC_HOST, SW_SHOW);
		P2PSelectionDialogProcSetModee(hDlg, IDC_MYIP, SW_SHOW);

	}
}
///////////////////////////////////////////////
typedef struct {
	char username[32];
	char hostname[128];
}P2PHBS;

odlist<P2PHBS, 32> P2PStoredUsers;


void P2PDisplayP2PStoredUsers(){
	p2p_ui_storedusers.DeleteAllRows();
	for (int x = 0; x< P2PStoredUsers.length; x++){
		p2p_ui_storedusers.AddRow(P2PStoredUsers[x].username, x);
		p2p_ui_storedusers.FillRow(P2PStoredUsers[x].hostname,1, x);
	}
}


P2PHBS P2PHBS_temp;


LRESULT CALLBACK P2PStoredUsersModifyDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (uMsg==WM_INITDIALOG){
		if (lParam){
			SetWindowText(GetDlgItem(hDlg, IDC_NAME), P2PHBS_temp.username);
			SetWindowText(GetDlgItem(hDlg, IDC_IP), P2PHBS_temp.hostname);
			SetWindowText(hDlg, "Edit");
		} else SetWindowText(hDlg, "Add");
	} else if (uMsg==WM_CLOSE){
		EndDialog(hDlg, 0);
	} else if (uMsg==WM_COMMAND){
		if (LOWORD(wParam)==IDOK){
			GetWindowText(GetDlgItem(hDlg, IDC_NAME), P2PHBS_temp.username,32);
			GetWindowText(GetDlgItem(hDlg, IDC_IP), P2PHBS_temp.hostname,128);
			EndDialog(hDlg, 1);
		} else if (LOWORD(wParam)==IDCANCEL)
			EndDialog(hDlg, 0);
	}
	return 0;
}

void P2PStoredUsersListEdit(){
	int i = p2p_ui_storedusers.SelectedRow();
	if (i>=0&&i<P2PStoredUsers.length) {
		P2PHBS_temp = P2PStoredUsers[i];
		if (DialogBoxParam(hx, (LPCTSTR)P2P_ITEM_EDIT, p2p_ui_ss_dlg, (DLGPROC)P2PStoredUsersModifyDlgProc, 1)){
			P2PStoredUsers.set(P2PHBS_temp, i);
		}
	}
	P2PDisplayP2PStoredUsers();
}

void P2PStoredUsersListAdd(){
	memset(&P2PHBS_temp, 0, sizeof(P2PHBS));
	if (DialogBoxParam(hx, (LPCTSTR)P2P_ITEM_EDIT, p2p_ui_ss_dlg, (DLGPROC)P2PStoredUsersModifyDlgProc, 0)){
		P2PStoredUsers.add(P2PHBS_temp);
	}
	P2PDisplayP2PStoredUsers();
}


void P2PStoredUsersListSelect(HWND hDlg){
	int i = p2p_ui_storedusers.SelectedRow();
	if (i>=0&&i<P2PStoredUsers.length)
		SetDlgItemText(hDlg, IDC_IP, P2PStoredUsers[i].hostname);
}

void P2PStoredUsersListDelete(){
	int i = p2p_ui_storedusers.SelectedRow();
	if (i>=0&&i<P2PStoredUsers.length)
		P2PStoredUsers.removei(i);
	P2PDisplayP2PStoredUsers();
}

void P2PLoadStoredUsersList(){
	P2PStoredUsers.clear();
	int count = nSettings::get_int("ULISTC", 0);
	for (int x=1;x<=count;x++){
		char idt[32];
		P2PHBS sx;
		wsprintf(idt, "ULUN%i", x);
		nSettings::get_str(idt,sx.username, "UserName");
		wsprintf(idt, "ULHN%i", x);
		nSettings::get_str(idt,sx.hostname, "127.0.0.1");
		P2PStoredUsers.add(sx);
	}
	P2PDisplayP2PStoredUsers();
}

void P2PSaveStoredUsersList(){
	nSettings::set_int("ULISTC",P2PStoredUsers.length);
	for (int x=0;x<P2PStoredUsers.length;x++){
		char idt[32];
		P2PHBS sx = P2PStoredUsers[x];
		wsprintf(idt, "ULUN%i", x+1);
		nSettings::set_str(idt,sx.username);
		wsprintf(idt, "ULHN%i", x+1);
		nSettings::set_str(idt,sx.hostname);
	}
}
/////////////////////////////////////////////////

void p2p_hosted_game_callback(char * game){
	if (gamelist != 0) {
		char * xx = gamelist;
		int p;
		while ((p=strlen(xx))!= 0){
			if (strcmp(game, xx)==0) {
				return;
			}
			xx += p+ 1;
		}
	}
	
	outpf("ERROR: Game not found on your local list");
	outpf("ERROR: Game not found on your local list");
	outpf("ERROR: Game not found on your local list");
	outpf("ERROR: Game not found on your local list");
}

/////////////////////////////////////////

UINT_PTR p2p_cdlg_timer;
LRESULT CALLBACK P2PSelectionDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
		case WM_INITDIALOG:
		{
			
			p2p_ui_ss_dlg = hDlg;

			SetWindowText(hDlg, "n02.p2p " P2P_VERSION);
			
			nSettings::Initialize();

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
			{
				DWORD xxx = 32;
				GetUserName(USERNAME, &xxx);
				char un[128];
				nSettings::get_str("IDC_USRNAME", un, USERNAME);
				strncpy(USERNAME, un, 34);
				SetWindowText(GetDlgItem(hDlg, IDC_USRNAME), USERNAME);
			}
			SendMessage(GetDlgItem(hDlg, IDC_CLIENTRANDOM), BM_SETCHECK, nSettings::get_int("IDC_CLIENTRANDOM", BST_CHECKED), 0);
			nTab tabb;
			tabb.handle = p2p_ui_modeseltab = GetDlgItem(hDlg, IDC_TAB1);
			tabb.AddTab("Host", 0);
			tabb.AddTab("Connect", 1);
			tabb.SelectTab(min(max(nSettings::get_int("IDC_TAB", 0),0),1));
			P2PSelectionDialogProcSetMode(hDlg, tabb.SelectedTab()!=0);

			p2p_ui_storedusers.handle = GetDlgItem(hDlg, IDC_ULIST);
			p2p_ui_storedusers.AddColumn("Name", 200);
			p2p_ui_storedusers.AddColumn("IP", 180);
			p2p_ui_storedusers.FullRowSelect();

			
			P2PLoadStoredUsersList();

			initialize_mode_cb(GetDlgItem(hDlg, CMB_MODE));

			p2p_cdlg_timer = SetTimer(hDlg, 0, 1000, 0);

			SetFocus(GetDlgItem(hDlg,CMB_MODE));
		}
		break;
	case WM_TIMER:
		{
			if (KSSDFA.state == 0 && p2p_is_connected()){
				p2p_ping();
			}
			break;
		}
	case WM_CLOSE: case WM_DESTROY: case WM_QUIT:
		nSettings::set_int("IDC_CLIENTRANDOM", SendMessage(GetDlgItem(hDlg, IDC_CLIENTRANDOM), BM_GETCHECK, 0, 0));
		GetWindowText(GetDlgItem(hDlg, IDC_USRNAME), USERNAME, 31);
		nSettings::set_str("IDC_USRNAME", USERNAME);
		nSettings::set_int("IDC_PORT", GetDlgItemInt(hDlg, IDC_PORT, 0, FALSE));
		GetWindowText(GetDlgItem(hDlg, IDC_GAME), GAME, 127);
		nSettings::set_str("IDC_GAME", GAME);
		GetWindowText(GetDlgItem(hDlg, IDC_IP), IP, 127);
		nSettings::set_str("IDC_IP", IP);

		nTab tabb;
		tabb.handle = p2p_ui_modeseltab;
		nSettings::set_int("IDC_TAB", tabb.SelectedTab());

		//char tempBuff[8];
		//GetWindowText(GetDlgItem(hDlg, CMB_MODE),tempBuff,8);
		//if(strncmp(tempBuff,"P2P",3) == 0 )
		P2PSaveStoredUsersList();


		EndDialog(hDlg, 0);


		nSettings::Terminate();
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_CONNECT:
			// save stuff
			nSettings::set_int("IDC_CLIENTRANDOM", SendMessage(GetDlgItem(hDlg, IDC_CLIENTRANDOM), BM_GETCHECK, 0, 0));
			GetWindowText(GetDlgItem(hDlg, IDC_USRNAME), USERNAME, 31);
			nSettings::set_str("IDC_USRNAME", USERNAME);
			nSettings::set_int("IDC_PORT", GetDlgItemInt(hDlg, IDC_PORT, 0, FALSE));
			GetWindowText(GetDlgItem(hDlg, IDC_GAME), GAME, 127);
			nSettings::set_str("IDC_GAME", GAME);
			GetWindowText(GetDlgItem(hDlg, IDC_IP), IP, 127);
			nSettings::set_str("IDC_IP", IP);
			nTab tabb;
			tabb.handle = p2p_ui_modeseltab;
			nSettings::set_int("IDC_TAB", tabb.SelectedTab());
			P2PSaveStoredUsersList();

			InitializeP2PSubsystem(hDlg, false);
			break;
		case IDC_HOST:
			// save stuff
			nSettings::set_int("IDC_CLIENTRANDOM", SendMessage(GetDlgItem(hDlg, IDC_CLIENTRANDOM), BM_GETCHECK, 0, 0));
			GetWindowText(GetDlgItem(hDlg, IDC_USRNAME), USERNAME, 31);
			nSettings::set_str("IDC_USRNAME", USERNAME);
			nSettings::set_int("IDC_PORT", GetDlgItemInt(hDlg, IDC_PORT, 0, FALSE));
			GetWindowText(GetDlgItem(hDlg, IDC_GAME), GAME, 127);
			nSettings::set_str("IDC_GAME", GAME);
			GetWindowText(GetDlgItem(hDlg, IDC_IP), IP, 127);
			nSettings::set_str("IDC_IP", IP);
			nTab tabb2;
			tabb2.handle = p2p_ui_modeseltab;
			nSettings::set_int("IDC_TAB", tabb2.SelectedTab());
			P2PSaveStoredUsersList();

			InitializeP2PSubsystem(hDlg, true);
			break;
		case IDC_MYIP:
			SOCKET fetchip;
			sockaddr_in fetchipinfo;
			hostent* target;
			unsigned long tempaddr;
			char get_message[10000], portnumber[16];

			fetchip = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			fetchipinfo.sin_family = AF_INET;
			fetchipinfo.sin_port = htons((u_short)80);

			target = gethostbyname("whatismyip.com");
			if(!target)
			{
				MessageBox(NULL,"Failed to connect to www.whatismyip.com :(","Failure!",NULL);
				closesocket(fetchip);
				break;
			}
			fetchipinfo.sin_addr.s_addr = *((unsigned long*)target->h_addr);
			connect(fetchip, (sockaddr *)&fetchipinfo, sizeof(fetchipinfo));
			// Now we are connected to whatismyip.com.  time to send the http request.
			strcpy(get_message,"GET /automation/n09230945.asp HTTP/1.0\r\nHost: www.whatismyip.com\r\n\r\n");
			send(fetchip,get_message,strlen(get_message)+1,0);
			ZeroMemory(get_message,sizeof(get_message));
			recv(fetchip,get_message,sizeof(get_message),0);
			closesocket(fetchip);

			strtok(get_message,"\n");

			char* theip, *next;

			while(next = strtok(NULL,"\n"))
			{
				theip = next;
			}

			GetWindowText(GetDlgItem(hDlg, IDC_PORT),portnumber,sizeof(portnumber));
			strcat(theip,":");
			strcat(theip,portnumber);

			if(OpenClipboard(hDlg))
			{
				EmptyClipboard();

				HGLOBAL hClipboardData;
				hClipboardData = GlobalAlloc(GMEM_DDESHARE, strlen(theip)+1);
				char* data = (char*)GlobalLock(hClipboardData);
				strcpy(data,theip);
				GlobalUnlock(hClipboardData);
				SetClipboardData(CF_TEXT,hClipboardData);
				CloseClipboard();
			}
			break;
		case IDC_ADD:
			P2PStoredUsersListAdd();
			break;
		case IDC_EDIT:
			P2PStoredUsersListEdit();
			break;
		case IDC_DELETE:
			P2PStoredUsersListDelete();
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
		NMHDR * nParam = (NMHDR*)lParam;
		if (nParam->hwndFrom == p2p_ui_modeseltab && nParam->code == TCN_SELCHANGE){
			nTab tabb;
			tabb.handle = p2p_ui_modeseltab;
			P2PSelectionDialogProcSetMode(hDlg, tabb.SelectedTab()!=0);			
		}
		if (nParam->hwndFrom == p2p_ui_storedusers.handle){
			if (nParam->code == NM_CLICK) {
				P2PStoredUsersListSelect(hDlg);
			}
		}
		break;
	};
	return 0;
}

void p2p_GUI(){
	INITCOMMONCONTROLSEX icx;
	icx.dwSize = sizeof(icx);
	icx.dwICC = ICC_LISTVIEW_CLASSES | ICC_TAB_CLASSES;
	InitCommonControlsEx(&icx);

	HMODULE p2p_riched_hm = LoadLibrary("riched32.dll");

	DialogBox(hx, (LPCTSTR)MAIN_DIALOG, 0, (DLGPROC)P2PSelectionDialogProc);

	FreeLibrary(p2p_riched_hm);

}

