/*
This program has been ported to Turbo C++ for Windows by John C. Wright
from the original Macintosh program, COLDWAVE, by Rob Goldston.
	Rob Goldston						John C. Wright
	125 Clover Lane                     Princeton Plasma Physics Lab   		
	Princeton, New Jersey 08540         Princeton, NJ 08540                            	
	(609) 921-1462                      (609) 243-3789
    goldston@pppl.gov					jwright@pppl.gov

This program was compiled under Borland's Turbo C++ for Windows 3.1
Required files for compilation:
	coldwave.prg	Project File
	coldwave.rc     Resource file (specifies menus, etc.)
	cold_ids.h  	Resource ID numbers
    	colddisp.h	Model module (defines coldwave class, has all the calculations)
	coldwave.cpp    Controller/view module. main program create interface (this file)
    
Other files:
	coldwave.wri	Instructions for Coldwave in Write format
    	coldwave.exe	Windows 3.1 executable
*/


#include <owl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <complex.h>
#include <math.h>
#include <edit.h>
#include <radiobut.h>
#include <static.h>
#include "cold_ids.h"


struct TTransferStruct {
	BOOL wLIN,wLOG;
	BOOL kLIN,kLOG;
	char wce[8];
	char zion[8];
	char aion[8];
	char theta[8];
	char kmin[8];
	char kmax[8];
	char wmin[8];
	char wmax[8];
 	char reso[8];
};

#include "colddisp.h"

// Declare TApp, a TApplication descendant
class TApp : public TApplication
{
public:
  TApp(LPSTR AName, HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow)
    : TApplication(AName, hInstance, hPrevInstance, lpCmdLine, nCmdShow) {};
  virtual void InitMainWindow();
};

// Declare TheWindow, a TWindow descendant
_CLASSDEF(TheWindow)
class TheWindow : public TWindow
{
	ColdWave *Dsprel;                                       //instance of Coldwave class
	TStatic *kPos, *wPos, *kwPos;                           //mouse position readouts
public:
	int width, height;
    BOOL gotpara;
    TTransferStruct WaveTransfer;
	TheWindow(PTWindowsObject AParent, LPSTR ATitle);		//constructor
    ~TheWindow();                                           //destructor
	virtual BOOL CanClose();          	//Do you really want to quit?
	virtual void Paint(HDC PaintDC, PAINTSTRUCT _FAR& PaintInfo);
	virtual void GetWindowClass( WNDCLASS& WndClass );
	virtual void CMSeed(RTMessage Msg) = [CM_FIRST + CM_SEED]; //handle menu
	virtual void CMDone(RTMessage Msg) = [CM_FIRST + CM_DONE]; //handle menu
	virtual void SetupWindow();
	virtual void WMMouseMove(RTMessage Msg) = [WM_FIRST + WM_MOUSEMOVE];  //track mouse pos.
};

class WaveDlg : public TDialog {
public:
	WaveDlg(PTWindowsObject AParent, int ResourceId);
};

WaveDlg::WaveDlg(PTWindowsObject AParent, int ResourceId)
		: TDialog(AParent, ResourceId)
{
	new TRadioButton(this, IDB_WLIN, NULL);
	new TRadioButton(this, IDB_WLOG, NULL);
	new TRadioButton(this, IDB_KLIN, NULL);
	new TRadioButton(this, IDB_KLOG, NULL);
	new TEdit(this, ID_wce,
		sizeof(((TheWindow *)Parent)->WaveTransfer.wce));
	new TEdit(this, ID_zion,
		sizeof(((TheWindow *)Parent)->WaveTransfer.zion));
	new TEdit(this, ID_aion,
		sizeof(((TheWindow *)Parent)->WaveTransfer.aion));
	new TEdit(this, ID_theta,
		sizeof(((TheWindow *)Parent)->WaveTransfer.theta));
	new TEdit(this, ID_kmin,
		sizeof(((TheWindow *)Parent)->WaveTransfer.kmin));
	new TEdit(this, ID_kmax,
		sizeof(((TheWindow *)Parent)->WaveTransfer.kmax));
	new TEdit(this, ID_wmin,
		sizeof(((TheWindow *)Parent)->WaveTransfer.wmin));
	new TEdit(this, ID_wmax,
		sizeof(((TheWindow *)Parent)->WaveTransfer.wmax));
	new TEdit(this, ID_reso,
		sizeof(((TheWindow *)Parent)->WaveTransfer.reso));
	TransferBuffer = (void far*)&(((TheWindow *)Parent)->WaveTransfer);
}


// Constructor for a TheWindow
TheWindow::TheWindow(PTWindowsObject AParent, LPSTR ATitle)
		 : TWindow(AParent, ATitle)
{
    AssignMenu("COLD_MENU");
	Attr.Style = WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU;
	Attr.X = 100;
	Attr.Y = 50;
	Attr.W = WW;
	Attr.H = WH;
    gotpara = FALSE;
	memset(&WaveTransfer, 0x0, sizeof WaveTransfer);

	Dsprel = new ColdWave(500, 500);
	kPos = new TStatic(this, ID_KPOS,"kc/wpe = 00000",5,1,130,17,0);
	wPos = new TStatic(this, ID_WPOS,"w/wpe = 00000",145,1,130,17,0);
	kwPos = new TStatic(this, ID_KWPOS,"ck/w = 00000",285,1,130,17,0);
// Initialize dialog variables
	WaveTransfer.kLIN=TRUE;
	WaveTransfer.kLOG=FALSE;
	WaveTransfer.wLIN=TRUE;
	WaveTransfer.wLOG=FALSE;
	strcpy(WaveTransfer.wce,"1.414");
	strcpy(WaveTransfer.zion,"1");
	strcpy(WaveTransfer.aion,"1");
	strcpy(WaveTransfer.theta,"0.0");
	strcpy(WaveTransfer.wmin,"0.0");
	strcpy(WaveTransfer.wmax,"5.0");
	strcpy(WaveTransfer.kmin,"0.0");
	strcpy(WaveTransfer.kmax,"5.0");
	strcpy(WaveTransfer.reso,"1");
}


TheWindow::~TheWindow()
{
	delete Dsprel;
	delete kPos;
    delete wPos;
}

void TheWindow::SetupWindow()
{
	TWindow::SetupWindow();
	RECT client_rect;
	HMENU hMenu=GetMenu(HWindow);

	GetClientRect(HWindow, &client_rect);
	width = client_rect.right;
	height = client_rect.bottom;

	Dsprel->SetSize(width, height);

	AppendMenu(hMenu, MF_DISABLED, 777, "wce/wpe = 1.414e+00");
	AppendMenu(hMenu, MF_DISABLED, 778, "th = 0.000e+00");
	DrawMenuBar(HWindow);
}

void  TheWindow::GetWindowClass( WNDCLASS& WndClass)
{
	TWindow::GetWindowClass( WndClass );
	WndClass.hIcon  = LoadIcon( GetApplication()->hInstance, "COLD_ICON" );
	WndClass.hCursor = LoadCursor( GetApplication()->hInstance, "COLD_CURSOR");
}

void TheWindow::WMMouseMove(RTMessage Msg)
{
	WORD x,y;
  
    x = Msg.LP.Lo;     
	y = Msg.LP.Hi;

	double kpos, wpos;
	char kstr[18], wstr[18], kwstr[18];

	kpos = x;
	Dsprel->getcoord((int)x,(int)y,kpos,wpos);

	sprintf(kstr,"kc/wpe = %7.3e",kpos);
	sprintf(wstr,"w/wpe = %7.3e",wpos);
    sprintf(kwstr,"ck/w = %7.3e",kpos/wpos);

	kPos->SetText(kstr);
	wPos->SetText(wstr);
	kwPos->SetText(kwstr);        
}


void TheWindow::CMSeed(RTMessage)
{
	int i;
    double plotx,ploty1,ploty2;
	HMENU hMenu=GetMenu(HWindow);
	char s[40];

	if ( GetModule()->ExecDialog(new WaveDlg(this,ID_DIALOG))==IDOK ) {

	sprintf(s,"wce/wpe = %7.3e",atof(WaveTransfer.wce));
	ModifyMenu(hMenu, 777, MF_DISABLED,777, s);
	sprintf(s,"th = %7.3e", atof(WaveTransfer.theta));
	ModifyMenu(hMenu, 778, MF_DISABLED,778, s);
	DrawMenuBar(HWindow);

	Dsprel->GetBuffer(WaveTransfer);
	Dsprel->cwcalc(HWindow);
	gotpara = TRUE;
	InvalidateRect(HWindow, NULL, TRUE);  }
}

void TheWindow::CMDone(RTMessage)
{
   	CloseWindow();
}

BOOL TheWindow::CanClose()
{
	BOOL value;
	value = (BOOL) MessageBox(HWindow, "", "Sure you want to quit?"
		, MB_YESNO | MB_ICONQUESTION) == IDYES;
	return value;
}



// Responds to an incoming "paint" message by redrawing boxes.
void TheWindow::Paint(HDC PaintDC,PAINTSTRUCT&)
{
	Rectangle(PaintDC, 0, 0, 420, 20);
    if (gotpara) Dsprel->Plot(PaintDC);
}


// Construct the TApp's MainWindow of type TScrollWindow
void TApp::InitMainWindow()
{
  MainWindow = new TheWindow(NULL, "Cold Wave");
}

// Run the TApp
int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
  LPSTR lpCmdLine, int nCmdShow)
{
  TApp ColdWaveApp("Cold Wave", hInstance, hPrevInstance,
    lpCmdLine, nCmdShow);
  ColdWaveApp.Run();
  return ColdWaveApp.Status;
}
