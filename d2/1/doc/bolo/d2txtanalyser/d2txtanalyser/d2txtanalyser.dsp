# Microsoft Developer Studio Project File - Name="d2txtanalyser" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=d2txtanalyser - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "d2txtanalyser.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "d2txtanalyser.mak" CFG="d2txtanalyser - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "d2txtanalyser - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "d2txtanalyser - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "d2txtanalyser - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "d2txtanalyser___Win32_Release"
# PROP BASE Intermediate_Dir "d2txtanalyser___Win32_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "d2txtanalyser___Win32_Release"
# PROP Intermediate_Dir "d2txtanalyser___Win32_Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W4 /WX /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /FR /YX /FD /c
# ADD BASE RSC /l 0x40c /d "NDEBUG"
# ADD RSC /l 0x40c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "d2txtanalyser - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD BASE RSC /l 0x40c /d "_DEBUG"
# ADD RSC /l 0x40c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "d2txtanalyser - Win32 Release"
# Name "d2txtanalyser - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "MPQ_C"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\mpq\Dcl_tbl.c

!IF  "$(CFG)" == "d2txtanalyser - Win32 Release"

# ADD CPP /W3

!ELSEIF  "$(CFG)" == "d2txtanalyser - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\mpq\Explode.c

!IF  "$(CFG)" == "d2txtanalyser - Win32 Release"

# ADD CPP /W3

!ELSEIF  "$(CFG)" == "d2txtanalyser - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\mpq\MpqView.c

!IF  "$(CFG)" == "d2txtanalyser - Win32 Release"

# ADD CPP /W3

!ELSEIF  "$(CFG)" == "d2txtanalyser - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\mpq\Wav_unp.c

!IF  "$(CFG)" == "d2txtanalyser - Win32 Release"

# ADD CPP /W3

!ELSEIF  "$(CFG)" == "d2txtanalyser - Win32 Debug"

!ENDIF 

# End Source File
# End Group
# Begin Source File

SOURCE=..\src\d2txtanalyser.c
# End Source File
# Begin Source File

SOURCE=..\src\error.c
# End Source File
# Begin Source File

SOURCE=..\src\filmem.c
# End Source File
# Begin Source File

SOURCE=..\src\iniread.c
# End Source File
# Begin Source File

SOURCE=..\src\test.c
# End Source File
# Begin Source File

SOURCE=..\src\test_keys.c
# End Source File
# Begin Source File

SOURCE=..\src\txtread.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "MPQ_H"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\mpq\Dclib.h
# End Source File
# Begin Source File

SOURCE=..\src\mpq\mpqtypes.h
# End Source File
# Begin Source File

SOURCE=..\src\mpq\MpqView.h
# End Source File
# Begin Source File

SOURCE=..\src\mpq\Wav_unp.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\src\error.h
# End Source File
# Begin Source File

SOURCE=..\src\filmem.h
# End Source File
# Begin Source File

SOURCE=..\src\globals.h
# End Source File
# Begin Source File

SOURCE=..\src\iniread.h
# End Source File
# Begin Source File

SOURCE=..\src\test.h
# End Source File
# Begin Source File

SOURCE=..\src\test_keys.h
# End Source File
# Begin Source File

SOURCE=..\src\txtread.h
# End Source File
# Begin Source File

SOURCE=..\src\types.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
