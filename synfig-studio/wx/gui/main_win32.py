from ctypes import windll, create_string_buffer,Structure, byref
from ctypes.wintypes import DWORD,SHORT, WORD
import os
import msvcrt
import sys

STD_INPUT_HANDLE  = -10
STD_OUTPUT_HANDLE = -11
STD_ERROR_HANDLE  = -12
INVALID_HANDLE_VALUE = DWORD(-1).value

MAX_LINES = 500

def consoleOptionEnabled(argv):
	value = False
	if ("--console" in argv) or ("-c" in argv):
		value = True
	return value

def redirectIOToConsole():
	class CONSOLE_SCREEN_BUFFER_INFO(Structure):
		_fields_ = [("dwSize", COORD),
		("dwCursorPosition", COORD),
		("wAttributes", WORD),
        ("srWindow", SMALL_RECT),
        ("dwMaximumWindowSize", DWORD)]

	coninfo = CONSOLE_SCREEN_BUFFER_INFO()

	# allocate console
	if(windll.kernel32.GetStdHandle(STD_OUTPUT_HANDLE) != INVALID_HANDLE_VALUE):
		windll.kernel32.AllocConsole()

	# set the screen buffer to be big enough to let us scroll text
	windll.kernel32.GetConsoleScreenBufferInfo(windll.kernel32.GetStdHandle(STD_OUTPUT_HANDLE), byref(coninfo))
	coninfo.dwSize.Y = MAX_LINES
	windll.kernel32.SetConsoleScreenBufferSize(windll.kernel32.GetStdHandle(STD_OUTPUT_HANDLE), coninfo.dwSize)

	#redirect unbuffered STDOUT to the console
	lStdHandle = windll.kernel32.GetStdHandle(STD_OUTPUT_HANDLE)

	hConHandle = msvcrt.open_osfhandle(lStdHandle, os.O_TEXT)
	fp = os.fdopen( hConHandle, "w" )
	sys.stdout = fp
	setvbuf( stdout, NULL, _IONBF, 0 )
	# redirect unbuffered STDIN to the console
	lStdHandle = windll.kernel32.GetStdHandle(STD_INPUT_HANDLE)
	hConHandle = msvcrt.open_osfhandle(lStdHandle, os.O_TEXT)
	fp = os.fdopen( hConHandle, "r" )
	sys.stdin = fp
	setvbuf( stdin, NULL, _IONBF, 0 )
	
	#redirect unbuffered STDERR to the console
	lStdHandle = windll.kernel32.GetStdHandle(STD_ERROR_HANDLE)
	hConHandle = msvcrt.open_osfhandle(lStdHandle, os.O_TEXT)
	fp = os.fdopen( hConHandle, "w" )
	sys.stderr = fp
	setvbuf( stderr, NULL, _IONBF, 0 )
	# make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog 
	# point to console as well
	ios::sync_with_stdio()
