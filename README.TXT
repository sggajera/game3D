To create a new GX Toolkit project follow these steps:

1) Start Microsoft Visual Studio.
2) Create a new "Win32 Application" (empty) project. Uncheck "Create directory for solution".
3) Copy the Framework, Application and Objects folders (from Demo 1) to the new project directory.
4) In Visual Studio Solution Explorer, right click on the project name icon and create a new Filter named Framework.
5) Right click on the new Framework folder in Solution Explorer and add all the files in the Framework folder to the project.
6) Right click on the project name and add all the files in the Application folder to the project 
  (they should automatically be put into the Solution Explorer folders named Source Files and Header Files).
7) In Solution Explorer, right click on the main files icon and select properties.
8) Click the General tab and in the MFC drop-down menu select Use MFC in a Static Library (or Shared DLL).
9) Select Use "Multi-Byte Character Set" under Project Defaults | Character Set.
10) Click the C/C++ tab and in the Category drop-down menu select Code Generation.
11) Select Use run-time library: Multi-threaded Debug
12) Select Struct member alignment: 1 Byte.
13) Click the Build Events tab, select Post-Build Event, click on the white area to the right of Command Line and 
  type in a new command as follows:
    copy Debug\*.exe .
14) Hit OK.
15) Hit Ctrl-Shift-B to build the project.
16) Start making changes as needed in Application\main.cpp: Program_Run().

Directory Structure
You need to set the directory search path to be able to find the GX Toolkit include and library files.  
In Visual Studio select Tools | Options | Projects | VC++ Directories.  Set to the following:

_____EXE_____

c:\mycode\bin									// personal directory (optional)
c:\dev2012w7\bin
c:\program files (x86)\microsoft speech sdk 5.1\bin				// Speech Development only
$(DXSDK_DIR)\Utilities\Bin\x86
- other microsoft directories -


_____INC_____

c:\mycode\inc									// personal directory (optional)
c:\dev2012w7\inc
c:\program files (x86)\microsoft speech sdk 5.1\include				// Speech Development only
$(IncludePath)
$(DXSDK_DIR)\Include
- other microsoft directories -


_____LIB_____

c:\mycode\lib									// personal directory (optional)
c:\dev2012w7\lib
c:\program files (x86)\microsoft speech sdk 5.1\lib\i386			// Speech Development only
$(LibraryPath)
$(DXSDK_DIR)\lib\x86
- other microsoft directories -
