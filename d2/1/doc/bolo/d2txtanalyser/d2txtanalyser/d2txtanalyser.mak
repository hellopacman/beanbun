# Microsoft Developer Studio Generated NMAKE File, Based on d2txtanalyser.dsp
!IF "$(CFG)" == ""
CFG=d2txtanalyser - Win32 Debug
!MESSAGE No configuration specified. Defaulting to d2txtanalyser - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "d2txtanalyser - Win32 Release" && "$(CFG)" != "d2txtanalyser - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
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
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "d2txtanalyser - Win32 Release"

OUTDIR=.\d2txtanalyser___Win32_Release
INTDIR=.\d2txtanalyser___Win32_Release
# Begin Custom Macros
OutDir=.\d2txtanalyser___Win32_Release
# End Custom Macros

ALL : "$(OUTDIR)\d2txtanalyser.exe" "$(OUTDIR)\d2txtanalyser.bsc"


CLEAN :
	-@erase "$(INTDIR)\d2txtanalyser.obj"
	-@erase "$(INTDIR)\d2txtanalyser.sbr"
	-@erase "$(INTDIR)\Dcl_tbl.obj"
	-@erase "$(INTDIR)\Dcl_tbl.sbr"
	-@erase "$(INTDIR)\error.obj"
	-@erase "$(INTDIR)\error.sbr"
	-@erase "$(INTDIR)\Explode.obj"
	-@erase "$(INTDIR)\Explode.sbr"
	-@erase "$(INTDIR)\filmem.obj"
	-@erase "$(INTDIR)\filmem.sbr"
	-@erase "$(INTDIR)\iniread.obj"
	-@erase "$(INTDIR)\iniread.sbr"
	-@erase "$(INTDIR)\MpqView.obj"
	-@erase "$(INTDIR)\MpqView.sbr"
	-@erase "$(INTDIR)\test.obj"
	-@erase "$(INTDIR)\test.sbr"
	-@erase "$(INTDIR)\test_keys.obj"
	-@erase "$(INTDIR)\test_keys.sbr"
	-@erase "$(INTDIR)\txtread.obj"
	-@erase "$(INTDIR)\txtread.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\Wav_unp.obj"
	-@erase "$(INTDIR)\Wav_unp.sbr"
	-@erase "$(OUTDIR)\d2txtanalyser.bsc"
	-@erase "$(OUTDIR)\d2txtanalyser.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /ML /W4 /WX /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\d2txtanalyser.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\d2txtanalyser.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\Dcl_tbl.sbr" \
	"$(INTDIR)\Explode.sbr" \
	"$(INTDIR)\MpqView.sbr" \
	"$(INTDIR)\Wav_unp.sbr" \
	"$(INTDIR)\d2txtanalyser.sbr" \
	"$(INTDIR)\error.sbr" \
	"$(INTDIR)\filmem.sbr" \
	"$(INTDIR)\iniread.sbr" \
	"$(INTDIR)\test.sbr" \
	"$(INTDIR)\txtread.sbr" \
	"$(INTDIR)\test_keys.sbr"

"$(OUTDIR)\d2txtanalyser.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\d2txtanalyser.pdb" /machine:I386 /out:"$(OUTDIR)\d2txtanalyser.exe" 
LINK32_OBJS= \
	"$(INTDIR)\Dcl_tbl.obj" \
	"$(INTDIR)\Explode.obj" \
	"$(INTDIR)\MpqView.obj" \
	"$(INTDIR)\Wav_unp.obj" \
	"$(INTDIR)\d2txtanalyser.obj" \
	"$(INTDIR)\error.obj" \
	"$(INTDIR)\filmem.obj" \
	"$(INTDIR)\iniread.obj" \
	"$(INTDIR)\test.obj" \
	"$(INTDIR)\txtread.obj" \
	"$(INTDIR)\test_keys.obj"

"$(OUTDIR)\d2txtanalyser.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "d2txtanalyser - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\d2txtanalyser.exe"


CLEAN :
	-@erase "$(INTDIR)\d2txtanalyser.obj"
	-@erase "$(INTDIR)\Dcl_tbl.obj"
	-@erase "$(INTDIR)\error.obj"
	-@erase "$(INTDIR)\Explode.obj"
	-@erase "$(INTDIR)\filmem.obj"
	-@erase "$(INTDIR)\iniread.obj"
	-@erase "$(INTDIR)\MpqView.obj"
	-@erase "$(INTDIR)\test.obj"
	-@erase "$(INTDIR)\test_keys.obj"
	-@erase "$(INTDIR)\txtread.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\Wav_unp.obj"
	-@erase "$(OUTDIR)\d2txtanalyser.exe"
	-@erase "$(OUTDIR)\d2txtanalyser.ilk"
	-@erase "$(OUTDIR)\d2txtanalyser.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MLd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /Fp"$(INTDIR)\d2txtanalyser.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\d2txtanalyser.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\d2txtanalyser.pdb" /debug /machine:I386 /out:"$(OUTDIR)\d2txtanalyser.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\Dcl_tbl.obj" \
	"$(INTDIR)\Explode.obj" \
	"$(INTDIR)\MpqView.obj" \
	"$(INTDIR)\Wav_unp.obj" \
	"$(INTDIR)\d2txtanalyser.obj" \
	"$(INTDIR)\error.obj" \
	"$(INTDIR)\filmem.obj" \
	"$(INTDIR)\iniread.obj" \
	"$(INTDIR)\test.obj" \
	"$(INTDIR)\txtread.obj" \
	"$(INTDIR)\test_keys.obj"

"$(OUTDIR)\d2txtanalyser.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("d2txtanalyser.dep")
!INCLUDE "d2txtanalyser.dep"
!ELSE 
!MESSAGE Warning: cannot find "d2txtanalyser.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "d2txtanalyser - Win32 Release" || "$(CFG)" == "d2txtanalyser - Win32 Debug"
SOURCE=..\src\mpq\Dcl_tbl.c

!IF  "$(CFG)" == "d2txtanalyser - Win32 Release"

CPP_SWITCHES=/nologo /ML /W3 /WX /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\d2txtanalyser.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\Dcl_tbl.obj"	"$(INTDIR)\Dcl_tbl.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "d2txtanalyser - Win32 Debug"

CPP_SWITCHES=/nologo /MLd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /Fp"$(INTDIR)\d2txtanalyser.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\Dcl_tbl.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=..\src\mpq\Explode.c

!IF  "$(CFG)" == "d2txtanalyser - Win32 Release"

CPP_SWITCHES=/nologo /ML /W3 /WX /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\d2txtanalyser.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\Explode.obj"	"$(INTDIR)\Explode.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "d2txtanalyser - Win32 Debug"

CPP_SWITCHES=/nologo /MLd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /Fp"$(INTDIR)\d2txtanalyser.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\Explode.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=..\src\mpq\MpqView.c

!IF  "$(CFG)" == "d2txtanalyser - Win32 Release"

CPP_SWITCHES=/nologo /ML /W3 /WX /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\d2txtanalyser.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\MpqView.obj"	"$(INTDIR)\MpqView.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "d2txtanalyser - Win32 Debug"

CPP_SWITCHES=/nologo /MLd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /Fp"$(INTDIR)\d2txtanalyser.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\MpqView.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=..\src\mpq\Wav_unp.c

!IF  "$(CFG)" == "d2txtanalyser - Win32 Release"

CPP_SWITCHES=/nologo /ML /W3 /WX /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\d2txtanalyser.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\Wav_unp.obj"	"$(INTDIR)\Wav_unp.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "d2txtanalyser - Win32 Debug"

CPP_SWITCHES=/nologo /MLd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /Fp"$(INTDIR)\d2txtanalyser.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\Wav_unp.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=..\src\d2txtanalyser.c

!IF  "$(CFG)" == "d2txtanalyser - Win32 Release"


"$(INTDIR)\d2txtanalyser.obj"	"$(INTDIR)\d2txtanalyser.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "d2txtanalyser - Win32 Debug"


"$(INTDIR)\d2txtanalyser.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\src\error.c

!IF  "$(CFG)" == "d2txtanalyser - Win32 Release"


"$(INTDIR)\error.obj"	"$(INTDIR)\error.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "d2txtanalyser - Win32 Debug"


"$(INTDIR)\error.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\src\filmem.c

!IF  "$(CFG)" == "d2txtanalyser - Win32 Release"


"$(INTDIR)\filmem.obj"	"$(INTDIR)\filmem.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "d2txtanalyser - Win32 Debug"


"$(INTDIR)\filmem.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\src\iniread.c

!IF  "$(CFG)" == "d2txtanalyser - Win32 Release"


"$(INTDIR)\iniread.obj"	"$(INTDIR)\iniread.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "d2txtanalyser - Win32 Debug"


"$(INTDIR)\iniread.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\src\test.c

!IF  "$(CFG)" == "d2txtanalyser - Win32 Release"


"$(INTDIR)\test.obj"	"$(INTDIR)\test.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "d2txtanalyser - Win32 Debug"


"$(INTDIR)\test.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\src\test_keys.c

!IF  "$(CFG)" == "d2txtanalyser - Win32 Release"


"$(INTDIR)\test_keys.obj"	"$(INTDIR)\test_keys.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "d2txtanalyser - Win32 Debug"


"$(INTDIR)\test_keys.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\src\txtread.c

!IF  "$(CFG)" == "d2txtanalyser - Win32 Release"


"$(INTDIR)\txtread.obj"	"$(INTDIR)\txtread.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "d2txtanalyser - Win32 Debug"


"$(INTDIR)\txtread.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 


!ENDIF 

