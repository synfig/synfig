/*
For general Scribus (>=1.3.2) copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Scribus 1.3.2
for which a new license (GPL+exception) is in place.
*/
/***************************************************************************
						main.cpp  -  description
							-------------------
	begin                : Fre Apr  6 21:47:55 CEST 2001
	copyright            : (C) 2001 by Franz Schmid
	email                : Franz.Schmid@altmuehlnet.de
	copyright            : (C) 2004 by Alessandro Rimoldi
	email                : http://ideale.ch/contact
	copyright            : (C) 2005 by Craig Bradney
	email                : cbradney@zip.com.au
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <iostream>
#include <signal.h>

#ifndef _WIN32
#error "This file compiles on win32 platform only"
#endif

#include <stdio.h>
#include <fcntl.h>
#include <io.h>

#include <QApplication>
#include <QMessageBox>
#include <exception>
using namespace std;

#define BASE_QM "scribus"
#define MAX_LINES 500

#include "scribusapp.h"
#include "scribuscore.h"
#include "scribus.h"
#include "scimagecachemanager.h"

#include "scconfig.h"

#include <windows.h>
#include <wincon.h>

int mainApp(ScribusQApp& app);

// Windows exception handling
static LONG exceptionFilter(DWORD exceptionCode);
static QString exceptionDescription(DWORD exceptionCode);
static void defaultCrashHandler(DWORD exceptionCode);

// Console IO redirection handling
void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);
bool consoleOptionEnabled(int argc, char* argv[]);
void redirectIOToConsole(void);

// Python environment configuration function
void setPythonEnvironment(const QString& appPath);

// Console option arguments declared in scribusapp.cpp
extern const char ARG_CONSOLE[];
extern const char ARG_CONSOLE_SHORT[];

ScribusCore SCRIBUS_API *ScCore;
ScribusMainWindow SCRIBUS_API *ScMW;
ScribusQApp SCRIBUS_API *ScQApp;
bool emergencyActivated;

int main(int argc, char *argv[])
{
	int result;
	emergencyActivated = false;
#if !defined(_CONSOLE)
	if (consoleOptionEnabled(argc, argv))
	{
		redirectIOToConsole();
		qInstallMessageHandler(messageHandler);
	}
#endif
	ScribusQApp app(argc, argv);
	setPythonEnvironment(app.applicationDirPath());
	result =  mainApp(app);
	return result;
}

/*!
\fn int mainApp(ScribusQApp& app)
\author Franz Schmid
\author Alessandro Rimoldi
\author Craig Bradney
\author Jean Ghali
\date Mon Feb  9 14:07:46 CET 2004
\brief Launches the Gui
\param app a Scribus QApplication object
\retval int Error code from the execution of Scribus
*/
int mainApp(ScribusQApp& app)
{
	int appRetVal = 0;
#ifndef _DEBUG
	__try
	{
#endif
		app.parseCommandLine();
		if (app.useGUI)
		{
			appRetVal = app.init();
			if (appRetVal != EXIT_FAILURE)
				appRetVal = app.exec();
		}
#ifndef _DEBUG
	}
	__except( exceptionFilter(GetExceptionCode()) )
	{
		defaultCrashHandler( GetExceptionCode() );
	}
#endif
	return appRetVal;
}

/*!
\fn void setPythonEnvironment(const QString& appPath)
\author Jean Ghali
\date Sat Jul 03 23:00:00 CET 2009
\brief set the Python envirionment for Scribus
\param appPath application Path
\retval None
*/
void setPythonEnvironment(const QString& appPath)
{
	QString pythonHome = appPath + "/python";
	if (!QDir(pythonHome).exists()) return; //assume a custom python

	QString tmp = "PYTHONHOME=" + QDir::toNativeSeparators(pythonHome);
	_wputenv((const wchar_t*) tmp.utf16());

	QString nativePath = QDir::toNativeSeparators(appPath);
    tmp = "PYTHONPATH=";
    tmp += nativePath;
    tmp += "\\python;";
    tmp += nativePath;
    tmp += "\\python\\lib;";
    tmp += nativePath;
    tmp += "\\python\\dlls;";
	tmp += nativePath;
    tmp += "\\python\\tcl";
    _wputenv((const wchar_t*) tmp.utf16());

	wchar_t* oldenv = _wgetenv(L"PATH");
    tmp = "PATH=";
    tmp += nativePath;
    tmp += ";";
    tmp += nativePath;
    tmp += "\\python";
    if(oldenv != NULL) {
        tmp += ";";
		tmp +=  QString::fromUtf16((const ushort*) oldenv);
    }
    _wputenv((const wchar_t*) tmp.utf16());
}

/*!
\fn int exceptionFilter(DWORD exceptionCode)
\author Jean Ghali
\date Sun Oct 30 14:30:30 CET 2005
\brief describe the behavior to be adopted against specific exception
\param exceptionCode the value returned by GetExceptionCode()
\retval LONG EXCEPTION_EXECUTE_HANDLER if handler is to be executed or EXCEPTION_CONTINUE_EXECUTION if not
*/
LONG exceptionFilter(DWORD exceptionCode)
{
	LONG result;
	switch( exceptionCode )
	{
	case EXCEPTION_ACCESS_VIOLATION:
	case EXCEPTION_DATATYPE_MISALIGNMENT:
	case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
	case EXCEPTION_FLT_DENORMAL_OPERAND:
	case EXCEPTION_FLT_DIVIDE_BY_ZERO:
	case EXCEPTION_FLT_INEXACT_RESULT:
	case EXCEPTION_FLT_INVALID_OPERATION:
	case EXCEPTION_FLT_OVERFLOW:
	case EXCEPTION_FLT_STACK_CHECK:
	case EXCEPTION_FLT_UNDERFLOW:
	case EXCEPTION_ILLEGAL_INSTRUCTION:
	case EXCEPTION_INT_DIVIDE_BY_ZERO:
	case EXCEPTION_INT_OVERFLOW:
	case EXCEPTION_INVALID_HANDLE:
	case EXCEPTION_NONCONTINUABLE_EXCEPTION:
	case EXCEPTION_STACK_OVERFLOW:
		result = EXCEPTION_EXECUTE_HANDLER;
		break;
	case EXCEPTION_BREAKPOINT:
	case EXCEPTION_SINGLE_STEP:
		result = EXCEPTION_CONTINUE_EXECUTION;
		break;
	default:
		result = EXCEPTION_EXECUTE_HANDLER;
		break;
	}
	return result;
}

/*!
\fn int exceptionDescription(DWORD exceptionCode)
\author Jean Ghali
\date Sun Oct 30 14:30:30 CET 2005
\brief return a string describing a specific exception
\param exceptionCode the value returned by GetExceptionCode()
\retval QString describing the specified exception
*/
static QString exceptionDescription(DWORD exceptionCode)
{
	QString description;
	if ( exceptionCode == EXCEPTION_ACCESS_VIOLATION )
		description = "EXCEPTION_ACCESS_VIOLATION";
	else if ( exceptionCode == EXCEPTION_DATATYPE_MISALIGNMENT )
		description = "EXCEPTION_DATATYPE_MISALIGNMENT";
	else if ( exceptionCode == EXCEPTION_ARRAY_BOUNDS_EXCEEDED )
		description = "EXCEPTION_ARRAY_BOUNDS_EXCEEDED";
	else if ( exceptionCode == EXCEPTION_FLT_DENORMAL_OPERAND )
		description = "EXCEPTION_FLT_DENORMAL_OPERAND";
	else if ( exceptionCode == EXCEPTION_FLT_DIVIDE_BY_ZERO )
		description = "EXCEPTION_FLT_DIVIDE_BY_ZERO";
	else if ( exceptionCode == EXCEPTION_FLT_INEXACT_RESULT )
		description = "EXCEPTION_FLT_INEXACT_RESULT";
	else if ( exceptionCode == EXCEPTION_FLT_INVALID_OPERATION )
		description = "EXCEPTION_FLT_INVALID_OPERATION";
	else if ( exceptionCode == EXCEPTION_FLT_OVERFLOW )
		description = "EXCEPTION_FLT_OVERFLOW";
	else if ( exceptionCode == EXCEPTION_FLT_STACK_CHECK )
		description = "EXCEPTION_FLT_STACK_CHECK";
	else if ( exceptionCode == EXCEPTION_FLT_UNDERFLOW )
		description = "EXCEPTION_FLT_UNDERFLOW";
	else if ( exceptionCode == EXCEPTION_GUARD_PAGE )
		description = "EXCEPTION_GUARD_PAGE";
	else if ( exceptionCode == EXCEPTION_ILLEGAL_INSTRUCTION )
		description = "EXCEPTION_ILLEGAL_INSTRUCTION";
	else if ( exceptionCode == EXCEPTION_IN_PAGE_ERROR )
		description = "EXCEPTION_IN_PAGE_ERROR";
	else if ( exceptionCode == EXCEPTION_INT_DIVIDE_BY_ZERO )
		description = "EXCEPTION_INT_DIVIDE_BY_ZERO";
	else if ( exceptionCode == EXCEPTION_INT_OVERFLOW )
		description = "EXCEPTION_INT_OVERFLOW";
	else if ( exceptionCode == EXCEPTION_INVALID_DISPOSITION )
		description = "EXCEPTION_INVALID_DISPOSITION";
	else if ( exceptionCode == EXCEPTION_INVALID_HANDLE )
		description = "EXCEPTION_INVALID_HANDLE";
	else if ( exceptionCode == EXCEPTION_NONCONTINUABLE_EXCEPTION )
		description = "EXCEPTION_NONCONTINUABLE_EXCEPTION";
	else if ( exceptionCode == EXCEPTION_PRIV_INSTRUCTION )
		description = "EXCEPTION_PRIV_INSTRUCTION";
	else if ( exceptionCode == EXCEPTION_STACK_OVERFLOW )
		description = "EXCEPTION_STACK_OVERFLOW";
	else
		description = "UNKNOWN EXCEPTION";
	return description;
}

void defaultCrashHandler(DWORD exceptionCode)
{
	static int crashRecursionCounter = 0;
	crashRecursionCounter++;
	if (crashRecursionCounter < 2)
	{
		emergencyActivated=true;
		crashRecursionCounter++;
		QString expDesc = exceptionDescription(exceptionCode);
		QString expHdr  = QObject::tr("Scribus Crash");
		QString expLine = "-------------";
		QString expMsg  = QObject::tr("Scribus crashes due to the following exception : %1").arg(expDesc);
		std::cout << expHdr.toStdString() << std::endl;
		std::cout << expLine.toStdString() << std::endl;
		std::cout << expMsg.toStdString() << std::endl;
		if (ScribusQApp::useGUI)
		{
			ScCore->closeSplash();
			QMessageBox::critical(ScMW, expHdr, expMsg, QObject::tr("&OK"));
			ScMW->emergencySave();
			ScMW->close();
		}
		ScImageCacheManager::instance().removeMasterLock();
	}
	ExitProcess(255);
}

void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
	QByteArray localMsg = msg.toLocal8Bit();
	switch (type)
	{
	case QtDebugMsg:
		cerr << "Debug: " << localMsg.constData()
			 << "(" << context.file << ":" << context.line << ", "
			 << context.function << ")"
			 << endl;
		break;
	case QtWarningMsg:
		cerr << "Warning: " << localMsg.constData()
			 << "(" << context.file << ":" << context.line << ", "
			 << context.function << ")"
			 << endl;
		break;
	case QtCriticalMsg:
		cerr << "Critical: " << localMsg.constData()
			 << "(" << context.file << ":" << context.line << ", "
			 << context.function << ")"
			 << endl;
		break;
	case QtFatalMsg:
		if (ScribusQApp::useGUI)
		{
			ScCore->closeSplash();
			QString expHdr = QObject::tr("Scribus Crash");
			QString expMsg = msg;
			QMessageBox::critical(ScMW, expHdr, expMsg, QObject::tr("&OK"));
			ScMW->emergencySave();
			ScMW->close();
		}
		ExitProcess(255);
	}
}

bool consoleOptionEnabled(int argc, char* argv[])
{
	bool value = false;
	for (int i = 0; i < argc; i++)
	{
		if (strcmp(argv[i], ARG_CONSOLE) == 0 ||
			strcmp(argv[i], ARG_CONSOLE_SHORT) == 0)
		{
			value = true;
			break;
		}
	}
	return value;
}

void redirectIOToConsole(void)
{
	int hConHandle;
	HANDLE lStdHandle;
	CONSOLE_SCREEN_BUFFER_INFO coninfo;
	FILE *fp;

	// allocate console
	if( GetStdHandle(STD_OUTPUT_HANDLE) != INVALID_HANDLE_VALUE )
		AllocConsole();
	// set the screen buffer to be big enough to let us scroll text
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &coninfo);
	coninfo.dwSize.Y = MAX_LINES;
	SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), coninfo.dwSize);
	//redirect unbuffered STDOUT to the console
	lStdHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	hConHandle = _open_osfhandle((intptr_t) lStdHandle, _O_TEXT);
	fp = _fdopen( hConHandle, "w" );
	*stdout = *fp;
	setvbuf( stdout, NULL, _IONBF, 0 );
	// redirect unbuffered STDIN to the console
	lStdHandle = GetStdHandle(STD_INPUT_HANDLE);
	hConHandle = _open_osfhandle((intptr_t) lStdHandle, _O_TEXT);
	fp = _fdopen( hConHandle, "r" );
	*stdin = *fp;
	setvbuf( stdin, NULL, _IONBF, 0 );
	// redirect unbuffered STDERR to the console
	lStdHandle = GetStdHandle(STD_ERROR_HANDLE);
	hConHandle = _open_osfhandle((intptr_t) lStdHandle, _O_TEXT);
	fp = _fdopen( hConHandle, "w" );
	*stderr = *fp;
	setvbuf( stderr, NULL, _IONBF, 0 );
	// make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog 
	// point to console as well
	ios::sync_with_stdio();
}
