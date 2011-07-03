# Microsoft Developer Studio Project File - Name="n02p" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=n02p - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "n02p.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "n02p.mak" CFG="n02p - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "n02p - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe
# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "N02P_EXPORTS" /YX /FD /c
# ADD CPP /nologo /Gz /MD /W3 /GX /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "N02P_EXPORTS" /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib comctl32.lib /nologo /dll /profile /machine:I386 /out:"../kailleraclient.dll"
# SUBTRACT LINK32 /map /debug
# Begin Target

# Name "n02p - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\kailleraclient.cpp
# End Source File
# Begin Source File

SOURCE=.\player.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\kailleraclient.h
# End Source File
# Begin Source File

SOURCE=.\player.h
# End Source File
# Begin Source File

SOURCE=.\uihlp.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\n02.rc
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# End Group
# Begin Group "common"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\common\dlist.h
# End Source File
# Begin Source File

SOURCE=.\common\k_framecache.cpp
# End Source File
# Begin Source File

SOURCE=.\common\k_framecache.h
# End Source File
# Begin Source File

SOURCE=.\common\k_socket.cpp
# End Source File
# Begin Source File

SOURCE=.\common\k_socket.h
# End Source File
# Begin Source File

SOURCE=.\common\nSettings.cpp
# End Source File
# Begin Source File

SOURCE=.\common\nSettings.h
# End Source File
# Begin Source File

SOURCE=.\common\nSTL.h
# End Source File
# Begin Source File

SOURCE=.\common\nThread.cpp
# End Source File
# Begin Source File

SOURCE=.\common\nThread.h
# End Source File
# Begin Source File

SOURCE=.\common\odlist.h
# End Source File
# Begin Source File

SOURCE=.\common\oslist.h
# End Source File
# Begin Source File

SOURCE=.\common\slist.h
# End Source File
# End Group
# Begin Group "p2p"

# PROP Default_Filter ""
# Begin Group "core"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\core\p2p_core.cpp
# End Source File
# Begin Source File

SOURCE=.\core\p2p_core.h
# End Source File
# Begin Source File

SOURCE=.\core\p2p_instruction.cpp
# End Source File
# Begin Source File

SOURCE=.\core\p2p_instruction.h
# End Source File
# Begin Source File

SOURCE=.\core\p2p_message.cpp
# End Source File
# Begin Source File

SOURCE=.\core\p2p_message.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\p2p_ui.cpp
# End Source File
# Begin Source File

SOURCE=.\p2p_ui.h
# End Source File
# End Group
# Begin Group "kaillera"

# PROP Default_Filter ""
# Begin Group "kcore"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\kcore\k_instruction.cpp
# End Source File
# Begin Source File

SOURCE=.\kcore\k_instruction.h
# End Source File
# Begin Source File

SOURCE=.\kcore\k_message.h
# End Source File
# Begin Source File

SOURCE=.\kcore\kaillera_core.cpp
# End Source File
# Begin Source File

SOURCE=.\kcore\kaillera_core.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\kaillera_ui.cpp
# End Source File
# Begin Source File

SOURCE=.\kaillera_ui.h
# End Source File
# Begin Source File

SOURCE=.\kaillera_ui_mslist.cpp
# End Source File
# End Group
# End Target
# End Project
