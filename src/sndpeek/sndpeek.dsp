# Microsoft Developer Studio Project File - Name="sndpeek" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=sndpeek - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "sndpeek.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "sndpeek.mak" CFG="sndpeek - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "sndpeek - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "sndpeek - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "sndpeek - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\marsyas\\" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "__WINDOWS_DS__" /D "_CONSOLE" /D "__LITTLE_ENDIAN__" /D "__PLATFORM_WIN32__" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib dsound.lib dxguid.lib glut32.lib opengl32.lib glu32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "sndpeek - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\marsyas\\" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "__WINDOWS_DS__" /D "_CONSOLE" /D "__LITTLE_ENDIAN__" /D "__PLATFORM_WIN32__" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib dsound.lib dxguid.lib glut32.lib opengl32.lib glu32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "sndpeek - Win32 Release"
# Name "sndpeek - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\marsyas\AutoCorrelation.cpp
# End Source File
# Begin Source File

SOURCE=..\marsyas\Centroid.cpp
# End Source File
# Begin Source File

SOURCE=.\chuck_fft.c
# End Source File
# Begin Source File

SOURCE=..\marsyas\Communicator.cpp
# End Source File
# Begin Source File

SOURCE=..\marsyas\DownSampler.cpp
# End Source File
# Begin Source File

SOURCE=..\marsyas\Flux.cpp
# End Source File
# Begin Source File

SOURCE=..\marsyas\fmatrix.cpp
# End Source File
# Begin Source File

SOURCE=..\marsyas\fvec.cpp
# End Source File
# Begin Source File

SOURCE=..\marsyas\Hamming.cpp
# End Source File
# Begin Source File

SOURCE=..\marsyas\LPC.cpp
# End Source File
# Begin Source File

SOURCE=..\marsyas\MagFFT.cpp
# End Source File
# Begin Source File

SOURCE=..\marsyas\MarSignal.cpp
# End Source File
# Begin Source File

SOURCE=..\marsyas\MFCC.cpp
# End Source File
# Begin Source File

SOURCE=..\marsyas\NormRMS.cpp
# End Source File
# Begin Source File

SOURCE=..\marsyas\RMS.cpp
# End Source File
# Begin Source File

SOURCE=..\marsyas\Rolloff.cpp
# End Source File
# Begin Source File

SOURCE=.\RtAudio.cpp
# End Source File
# Begin Source File

SOURCE=.\sndpeek.cpp
# End Source File
# Begin Source File

SOURCE=.\Stk.cpp
# End Source File
# Begin Source File

SOURCE=..\marsyas\System.cpp
# End Source File
# Begin Source File

SOURCE=.\Thread.cpp
# End Source File
# Begin Source File

SOURCE=.\util_sndfile.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\marsyas\AutoCorrelation.h
# End Source File
# Begin Source File

SOURCE=..\marsyas\Centroid.h
# End Source File
# Begin Source File

SOURCE=.\chuck_fft.h
# End Source File
# Begin Source File

SOURCE=..\marsyas\Communicator.h
# End Source File
# Begin Source File

SOURCE=..\marsyas\defs.h
# End Source File
# Begin Source File

SOURCE=..\marsyas\DownSampler.h
# End Source File
# Begin Source File

SOURCE=..\marsyas\Flux.h
# End Source File
# Begin Source File

SOURCE=..\marsyas\fmatrix.h
# End Source File
# Begin Source File

SOURCE=..\marsyas\fvec.h
# End Source File
# Begin Source File

SOURCE=..\marsyas\Hamming.h
# End Source File
# Begin Source File

SOURCE=..\marsyas\LPC.h
# End Source File
# Begin Source File

SOURCE=..\marsyas\MagFFT.h
# End Source File
# Begin Source File

SOURCE=..\marsyas\MarSignal.h
# End Source File
# Begin Source File

SOURCE=..\marsyas\MFCC.h
# End Source File
# Begin Source File

SOURCE=..\marsyas\NormRMS.h
# End Source File
# Begin Source File

SOURCE=..\marsyas\RMS.h
# End Source File
# Begin Source File

SOURCE=..\marsyas\Rolloff.h
# End Source File
# Begin Source File

SOURCE=.\RtAudio.h
# End Source File
# Begin Source File

SOURCE=.\Stk.h
# End Source File
# Begin Source File

SOURCE=..\marsyas\System.h
# End Source File
# Begin Source File

SOURCE=.\Thread.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
