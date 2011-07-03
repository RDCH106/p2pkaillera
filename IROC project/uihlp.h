// Updated by Ownasaurus in June 2010

#pragma once

#include "richedit.h"
#include "commctrl.h"

#include <windows.h>
#include <stdio.h>

extern HWND kaillera_sdlg_partchat;
extern bool workaround[2];
extern int timestamp;

class nTab{
public:
	HWND handle;

	inline void AddTab(char * name, int index){
		TCITEM tie;
		tie.mask = TCIF_TEXT;
		tie.pszText = name;
		tie.lParam = index;
		TabCtrl_InsertItem(handle, index, &tie);
	}

	inline void DeleteAllTabs(){
		TabCtrl_DeleteAllItems(handle);
	}

	inline void SelectTab(int index){
		TabCtrl_SetCurSel(handle,index);
	}
	inline int SelectedTab(){
		return TabCtrl_GetCurSel(handle);
	}
};



class nLVw {
protected:
	int cols;
	int format;
	int mask;
	int recz;

public:
    HWND handle;

	void initialize(){
		cols=0;
		recz = 0;
	}

	int AddRow (char * content){
		LVITEM lvI;
		lvI.mask        = LVIF_TEXT | LVIF_PARAM;
		lvI.iItem       = recz++;
		lvI.iSubItem    = 0;
		lvI.pszText		= content;
		lvI.lParam		= lvI.iItem;

		return ListView_InsertItem(handle, &lvI);
	}


	int AddRow(char*content, int rowno){
		LVITEM lvI;
		lvI.mask        = LVIF_TEXT | LVIF_PARAM;
		lvI.iItem       = rowno;
		lvI.iSubItem    = 0;
		lvI.pszText		= content;
		lvI.lParam		= rowno;

		recz++;

		ListView_InsertItem(handle, &lvI);
		return rowno;
	}

	int Find(int rowno){
		LVFINDINFO fi;
		fi.flags = LVFI_PARAM;
		fi.lParam = rowno;
		return ListView_FindItem(handle, -1, &fi);
	}

	int RowNo(int index){
		LVITEMA lvI;
		lvI.mask        = LVIF_PARAM;
		lvI.iItem       = index;
		lvI.iSubItem    = 0;
		SendMessage(handle, LVM_GETITEM, 0, (LPARAM)(&lvI));
		return lvI.lParam;
	}

	int RowsCount(){
		return ListView_GetItemCount(handle);
	}


	void DeleteRow(int rowno){
		recz--;
		ListView_DeleteItem(handle, rowno);
	}

	void FillRow (char * content, int coli, int rowi){
		/*
		LVITEMA lvI;
		lvI.mask        = LVIF_TEXT;
		lvI.iItem       = rowi;
		lvI.iSubItem    = coli;
		lvI.pszText		= content;
		SendMessage(handle, LVM_SETITEM, 0, (LPARAM)(&lvI));*/
		ListView_SetItemText(handle, rowi, coli, content);
	}

	void CheckRow (char * content, int llen, int coli, int rowi){
		LVITEMA lvI;
		lvI.mask        = LVIF_TEXT;
		lvI.iItem       = rowi;
		lvI.iSubItem    = coli;
		lvI.pszText		= content;
		lvI.cchTextMax	= llen;
		SendMessage(handle, LVM_GETITEM, 0, (LPARAM)(&lvI));
	}

	void AddColumn(char * name, int colw){
		LVCOLUMNA lvc;
		lvc.mask		= LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		lvc.iSubItem	= cols++;
		lvc.pszText		= name;
		lvc.cx			= colw;
		lvc.fmt			= LVCFMT_LEFT;

		ListView_InsertColumn(handle, (cols-1), &lvc);
	}

	void DeleteAllRows(){
		ListView_DeleteAllItems(handle);
		recz=0;
	}

	int SelectedRow(){
		return ListView_GetSelectionMark(handle);
	}
	void SelectRow(int i){
		ListView_EnsureVisible(handle, i, FALSE);
	}


	void FullRowSelect(void){
		LRESULT l = SendMessage(handle, LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0);
        SendMessage(handle, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, l|LVS_EX_FULLROWSELECT);
	}

	void FullRowUnSelect(void){
		LRESULT l = SendMessage(handle, LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0);
		l = (l & ~LVS_EX_FULLROWSELECT);
        SendMessage(handle, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, l);
	}

};

// Owna's rigged version of this function
inline void re_append(HWND hwnd, char * line, COLORREF color = 0){
	char msg[2097]; //Req: changed to help avoid spam crashes

	// Add timestamps if desired
	// TODO: create option for 24H time if wanted 
	if(timestamp)
	{
		SYSTEMTIME st;
		GetLocalTime(&st);
		if(st.wHour >= 12) //PM
			//Req: changing both of these lines to sprintf_s to avoid spam crashes, in accordance with msg[]
			sprintf_s(msg, 2097,"(%02d:%02d:%02dPM) %s",st.wHour == 12?st.wHour:st.wHour-12,st.wMinute,st.wSecond,line);
		else sprintf_s(msg, 2097,"(%02d:%02d:%02dAM) %s",st.wHour,st.wMinute,st.wSecond,line);
	}

	int i;
	if(timestamp)
		i = strlen(msg);
	else i = strlen(line);

	CHARRANGE cr, prev;
	GETTEXTLENGTHEX gtx;

	gtx.codepage = CP_ACP;
	gtx.flags = GTL_PRECISE;
	cr.cpMin = GetWindowTextLength(hwnd);
	cr.cpMax = cr.cpMin +i;
	SendMessage(hwnd, EM_EXGETSEL, 0, (LPARAM)&prev); // Keep the selection!
	SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM)&cr);
	CHARFORMATA crf;
	crf.dwMask = CFM_COLOR;
	crf.cbSize = sizeof(crf);
	crf.crTextColor = color;
	
	SendMessage(hwnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&crf);
	if(timestamp)
		SendMessage(hwnd, EM_REPLACESEL, FALSE, (LPARAM)msg);
	else SendMessage(hwnd, EM_REPLACESEL, FALSE, (LPARAM)line);
	SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM)&cr);
	SendMessage(hwnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&crf);
	SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM)&prev); // keep the selection
	
	int max, min;
	GetScrollRange(hwnd, SB_VERT, &min, &max);

	// Owna: The following contains my horribly rigged way of handing the scrolling,
	// which is still bugged in specific situations.  I hope someone fixes this!
	if(max == 0) // indicates no scrollbar (in this program only cause of tweaks)
	{
		if(hwnd == kaillera_sdlg_partchat)
			workaround[0] = true;
		else
		{
			workaround[1] = true;
		}
	}
	else
	{
		if(hwnd == kaillera_sdlg_partchat)
		{
			if(workaround[0])
			{
				SendMessage(hwnd, WM_VSCROLL, SB_BOTTOM, 0);
				workaround[0] = false;
			}
		}
		else
		{
			if(workaround[1])
			{
				SendMessage(hwnd, WM_VSCROLL, SB_BOTTOM, 0);
				workaround[1] = false;
			}
		}
	}
}