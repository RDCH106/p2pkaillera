/******************************************************************************
***  n02 v0.3 winnt                                                         ***
**   Open Kaillera Client Core                                               **
***  For latest sources, visit http://sf.net/projects/okai                  ***
******************************************************************************/
// Updated by Ownasaurus in June 2010

#include "nSettings.h"

#include <windows.h>

char file[5000];
char * subm;

// Owna: I now only call this function with false.
// + Now vista doesn't have permission issues.
// - Now the settings are stored *locally* in n02.ini
void nSettings::Initialize(char * submo, bool global){
	file[0] = 0;
	if (!global) {
		GetCurrentDirectory(5000, file);
	}
	strcat(file, "\\n02.ini");
	subm = submo;
//	MessageBox(0, file, 0, 0);
}

void nSettings::Terminate(){

}
int nSettings::get_int(char * key, int def_){
	return GetPrivateProfileInt(subm, key, def_, file);
}
char* nSettings::get_str(char * key, char * buf, char * def_){
	GetPrivateProfileString(subm, key, def_, buf, 128, file);
	return buf;
}
void nSettings::set_int(char * key, int val){
	char bft [128];
	wsprintf(bft, "%i", val);
	WritePrivateProfileString(subm, key, bft, file);
}
void nSettings::set_str(char * key, char * val){
	WritePrivateProfileString(subm, key, val, file);
}
