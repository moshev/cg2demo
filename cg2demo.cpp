// cg2demo.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "cg2demo.h"

// Global Variables:
HINSTANCE hInst;								// current instance

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	hInst = hInstance;

	return TRUE;
}
