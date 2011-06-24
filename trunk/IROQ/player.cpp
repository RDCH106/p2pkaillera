// Updated by Ownasaurus in June 2010

#include "player.h"
#include "kailleraclient.h"
#include "resource.h"
#include "uihlp.h"
#include <time.h>
bool player_playing = false;

class PlayBackBufferC {
public:
	char * buffer;
	char * ptr;
	char * end;
    
    void load_bytes(void * arg_0, unsigned int arg_4){
        if (ptr + 10 < end) {
            int p = min(arg_4, end - ptr);
            memcpy(arg_0, ptr, p);
            ptr += p;
        }
    }
    void load_str(char * arg_0, unsigned int arg_4){
		arg_4 = min(arg_4, strlen(ptr)+1);
        arg_4 = min(arg_4, end-ptr+1);
		load_bytes(arg_0, arg_4);
		arg_0[arg_4] = 0x00;
    }
	int load_int(){
        int x;
        load_bytes(&x,4);
        return x;
    }
	unsigned int load_unsigned_int(){
        int x;
        load_bytes(&x,4);
        return x;
    }
    unsigned char load_char(){
        unsigned char x;
        load_bytes(&x,1);
        return x;
    }
    unsigned short load_short(){
        unsigned short x;
        load_bytes(&x,2);
        return x;
    }
	
} PlayBackBuffer;

//==============================================


//..............................................

///////////////////////////////////////////////////////////////////////////////



nLVw RecordsListDlg_list;
HWND RecordsListDlg;
void player_play(char * fn){
	
	//char * fn = BrowseFile(0);
	if(fn== 0)
		return;
	
	OFSTRUCT of;
	HFILE in = OpenFile(fn, &of, OF_READ);
	
	if (in == HFILE_ERROR) 
		return;
	
	
	DWORD len = GetFileSize((HANDLE)in, 0);
	if (len < 256+16) {
		MessageBox(RecordsListDlg, "File too short", "Error", MB_OK | MB_ICONSTOP);
		return;
	}
	
	PlayBackBuffer.buffer = (char*)malloc(len+66);
	
	_lread(in, PlayBackBuffer.buffer, len);
	
	PlayBackBuffer.ptr = PlayBackBuffer.buffer + 4;
	PlayBackBuffer.end = PlayBackBuffer.buffer + len;
	
	_lclose(in);
	
	char APPC[128];
	
	PlayBackBuffer.load_str(APPC, 128);
	
	if (strcmp(APP, APPC)!= 0) {
		char wdr[2000];
		wsprintf(wdr, "Application name mismatch. \nExpected \"%s\" but recieved \"%s\".\nUse the correct emulator/version", APPC, APP);
		MessageBox(RecordsListDlg, wdr, "Error", MB_OK | MB_ICONSTOP);
		free (PlayBackBuffer.buffer);
		return;
	}
	
	PlayBackBuffer.ptr = PlayBackBuffer.buffer + 132;
	
	
	PlayBackBuffer.load_str(GAME,128);
	
	PlayBackBuffer.ptr = PlayBackBuffer.buffer + 264;
	
	playerno = PlayBackBuffer.load_int();
	numplayers = PlayBackBuffer.load_int();
	
	player_playing = true;
	
	KSSDFA.input = KSSDFA_START_GAME;
	
	while(player_playing)
		Sleep(2000);
	
	free (PlayBackBuffer.buffer);
	
}
void RecordsList_PlaySelected(){
	int s = RecordsListDlg_list.SelectedRow();
	if (s >=0 && s < RecordsListDlg_list.RowsCount()){
		char fn[1000];
		char filename[2000];
		RecordsListDlg_list.CheckRow(fn, 1000, 3, s);
		wsprintf(filename, ".\\records\\%s", fn);
		player_play(filename);
	}
}

void RecordsList_Populate();

void RecordsList_DeleteSelected(){
	int s = RecordsListDlg_list.SelectedRow();
	if (s >=0 && s < RecordsListDlg_list.RowsCount()){
		char fn[1000];
		char filename[2000];
		RecordsListDlg_list.CheckRow(fn, 1000, 3, s);
		wsprintf(filename, ".\\records\\%s", fn);
		DeleteFile(filename); // deletes the actual file
		RecordsList_Populate(); // repopulates the list (aka i'm too lazy to delete the row)
	}
}
void RecordsList_Populate_fn(char * fn, int i) {
	char filename[2000];
	wsprintf(filename, ".\\records\\%s", fn);
	
	CreateDirectory(".\\records", 0);

	OFSTRUCT of;
	HFILE in = OpenFile(filename, &of, OF_READ);
	if (in == HFILE_ERROR) {
		RecordsListDlg_list.AddRow(fn, i);
		RecordsListDlg_list.FillRow("Error", 1, i);
		return;
	}
	DWORD len = GetFileSize((HANDLE)in, 0);
	if (len < 300) {
		RecordsListDlg_list.AddRow(fn, i);
		RecordsListDlg_list.FillRow("File too short", 1, i);
		return;
	}
	PlayBackBuffer.buffer = filename;
	_lread(in, PlayBackBuffer.buffer, 300);
	PlayBackBuffer.ptr = PlayBackBuffer.buffer + 4;
	PlayBackBuffer.end = PlayBackBuffer.buffer + len;
	_lclose(in);
	char APPC[128];
	PlayBackBuffer.load_str(APPC, 128);
	PlayBackBuffer.ptr = PlayBackBuffer.buffer + 132;	
	PlayBackBuffer.load_str(GAME,128);
	PlayBackBuffer.ptr = PlayBackBuffer.buffer + 260;
	time_t timee = PlayBackBuffer.load_unsigned_int();
	playerno = PlayBackBuffer.load_int();
	numplayers = PlayBackBuffer.load_int();
	RecordsListDlg_list.AddRow(GAME,i);
	RecordsListDlg_list.FillRow(APPC,1,i);
	///**/time((time_t *)&timee);
	tm * ecx = localtime(&timee);
	if(!ecx) 
	{
		MessageBox(0,"Failed to read a replay file!","Failure!",0);
		return;
	}
	wsprintf(filename, "%02d:%02d on %02d/%02d/%02d",ecx->tm_hour,ecx->tm_min, ecx->tm_mon +1, ecx->tm_mday, ecx->tm_year +1900);
	RecordsListDlg_list.FillRow(filename,2,i);
	RecordsListDlg_list.FillRow(fn,3,i);
}

void RecordsList_Populate(){
	RecordsListDlg_list.DeleteAllRows();
	char * dir = ".\\records";
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	char DirSpec[5000];  // directory specification
	DWORD dwError;
	strncpy (DirSpec, dir, strlen(dir)+1);
	strncat (DirSpec, "\\*", 3);
	int i = 0;

	hFind = FindFirstFile(DirSpec, &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE) {
		RecordsListDlg_list.AddRow("Error refreshing list", i);
	} else {
		if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)==0){
			RecordsList_Populate_fn(FindFileData.cFileName,i++);
		}
		while (FindNextFile(hFind, &FindFileData) != 0) {
			if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)==0){
				RecordsList_Populate_fn(FindFileData.cFileName,i++);
			}
		}
		dwError = GetLastError();
		FindClose(hFind);
	}
}


LRESULT CALLBACK RecordsListDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_INITDIALOG:
		{
			RecordsListDlg = hDlg;
			RecordsListDlg_list.handle = GetDlgItem(hDlg, LV_GLIST);
			RecordsListDlg_list.AddColumn("Game", 200);
			RecordsListDlg_list.AddColumn("App", 140);
			RecordsListDlg_list.AddColumn("Time", 130);
			RecordsListDlg_list.AddColumn("fn", 10);
			RecordsListDlg_list.FullRowSelect();
			RecordsList_Populate();

			initialize_mode_cb(GetDlgItem(hDlg, CMB_MODE));

		}
		break;
	case WM_CLOSE:
		EndDialog(hDlg, 0);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDCREFRESH:
			{
				RecordsList_Populate();
			}
			break;
		case BTN_PLAY:
			RecordsList_PlaySelected();
			break;
		case BTN_DELETE:
			RecordsList_DeleteSelected();
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
		if(((LPNMHDR)lParam)->code==NM_DBLCLK && ((LPNMHDR)lParam)->hwndFrom==RecordsListDlg_list.handle){
			RecordsList_PlaySelected();
		}
		break;
	};
	return 0;
}

void player_GUI(){
	
	INITCOMMONCONTROLSEX icx;
	icx.dwSize = sizeof(icx);
	icx.dwICC = ICC_LISTVIEW_CLASSES | ICC_TAB_CLASSES;
	InitCommonControlsEx(&icx);
	
	HMODULE p2p_riched_hm = LoadLibrary("riched32.dll");
	
	DialogBox(hx, (LPCTSTR)RECORDER_PLAYBACK, 0, (DLGPROC)RecordsListDlgProc);
	
	FreeLibrary(p2p_riched_hm);
}

int player_MPV(void*values,int size){
	if (player_playing){
		if (PlayBackBuffer.ptr + 10 < PlayBackBuffer.end) {
			char b = PlayBackBuffer.load_char();
			if (b==0x12) {
				int l = PlayBackBuffer.load_short();
				if (l < 0) {
					player_playing = false;
					return -1;
				}
				if (l > 0)
					PlayBackBuffer.load_bytes((char*)values, l);//access error
				return l;
			}
			if (b==20) {
				char playernick[100];
				PlayBackBuffer.load_str(playernick, 100);
				int pn = PlayBackBuffer.load_int();
				infos.clientDroppedCallback(playernick, pn);
				return player_MPV(values, size);
			}
			if (b==8) {
				char nick[100];
				char msg[500];
				PlayBackBuffer.load_str(nick, 100);
				PlayBackBuffer.load_str(msg, 500);
				infos.chatReceivedCallback(nick, msg);
				return player_MPV(values, size);
			}
		} else player_playing = false;
	}
	return -1;
}
void player_EndGame(){
	player_playing =false;
	KSSDFA.input = KSSDFA_END_GAME;
	KSSDFA.state = 0;
}
bool player_SSDSTEP(){
	return false;
}
void player_ChatSend(char*){
	
}
bool player_RecordingEnabled(){
	return false;
}


