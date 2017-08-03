#include <windows.h>
#include <stdio.h>
#include "DxLib.h"
#include <iostream>
#include <tchar.h>
#include <atlstr.h>
#include <string>
#include <iostream>
#include "DxLib.h"
#include <string>
#include <fstream>
#include <math.h>
#include "Header1.h"

// define constants
//   debug flag
#define NEW_IMPLI 1
// threshold 
#define DEGREE_THRESHOLD 40
#define THRESHOLD 3
// COM PORT 
#define COM_PORT "COM3"
// move mouse paramater
#define DEFAULT 32768
#define ADJUST_DEFAULT 8
// window handler
#define WIN_WIDTH 500
#define WIN_HEIGHT 300
HWND hwnd;
HINSTANCE hinst;
int wid_icon;
int hei_icon;
int StringColor;
unsigned int BoxColor;
int X_adjust_para = ADJUST_DEFAULT;
int Y_adjust_para = ADJUST_DEFAULT;
int X_MULTIPLE = 0.5 * X_adjust_para;
int X_UP_MULTIPLE = 2.5 * X_adjust_para;
int X_DOWN_MULTIPLE = 5 * X_adjust_para;
int Y_MULTIPLE = 0.5 * Y_adjust_para;
int  Y_UP_MULTIPLE = 2.5 * Y_adjust_para;
int  Y_DOWN_MULTIPLE = 5 * Y_adjust_para;
int LogoGraph;


#define PI 3.141592653589793

HANDLE hCom = INVALID_HANDLE_VALUE;
int nowMauseX = DEFAULT;
int nowMauseY = DEFAULT;
int checkXsum = 0;
int checkYsum = 0;
int noise_avg_count = 0;
CString message0001 = "";
std::string  before_window;
std::ofstream outputfile("./cood2.dat");

// prototype define
bool ConnectCOM();
void parseMessage(CString text);
void ClickMouse(int Botton);
void ReceivefromCom();
void OnButtonClose();
bool AdjustmentMode(int botton,int x,int y);
void MoveMouse(int difX, int difY);
bool MoveWindowMode(int Botton, int x, int y);
void DoGetForegroundWindow();
void DoGetForegroundWindow();
void InitMousePos();
void UpdatePara();
// [warning] if you use COM PORT in other program, this program cannnot connect to  COM 5.

bool ConnectCOM()
{
	hCom = CreateFile((COM_PORT), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (hCom == INVALID_HANDLE_VALUE)   return false;

	DCB dcb;
	memset(&dcb, 0, sizeof(DCB));
	dcb.DCBlength = sizeof(DCB);
	dcb.BaudRate = CBR_19200;
	dcb.ByteSize = 16;
	dcb.Parity = NOPARITY;
	dcb.StopBits = ONESTOPBIT;
	SetCommState(hCom, &dcb);
	return true;
}

void InitMousePos() {
	nowMauseX = DEFAULT;
	nowMauseY = DEFAULT;
	mouse_event(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE,
		nowMauseX, nowMauseY, 0, NULL);
}

void parseMessage(CString text) {
	static int x_pre = 0;
	static int y_pre = 0;
	static int nowXrotate = 0;
	static int nowYrotate = 0;
	static int threshold_X = 0;
	static int threshold_Y = 0;

	// split " , " text
	CString query[7];
	int count_comma = 0;

	int iStart = 0;
	int iPos = -1;

	CString restoken;
	iPos = text.Find(',', iStart);

	while (iPos > -1)
	{
		restoken = text.Mid(iStart, iPos - iStart);
		query[count_comma++] = restoken;
		//printf("Resulting token: %s\n", restoken);

		iStart = iPos + 1;
		iPos = text.Find(',', iStart);

		if (iPos == -1)
		{
			restoken = text.Mid(iStart);
			query[count_comma++] = restoken;
			//printf("Resulting token: %s\n", restoken);
		}
	}

	TCHAR *pEnd = NULL;

	int  x_gyr = _tcstol(query[1], &pEnd, 16);
	int  y_gyr = _tcstol(query[2], &pEnd, 16);
	int x_acl = _tcstol(query[3], &pEnd, 16);
	int y_acl = _tcstol(query[4], &pEnd, 16);
	int z_acl = _tcstol(query[5], &pEnd, 16);
	int  botton = _tcstol(query[6], &pEnd, 16);

	if (x_acl > 32767) {
		x_acl = x_acl - 65536;
	}
	if (y_acl > 32767) {
		y_acl = y_acl - 65536;
	}
	if (z_acl > 32767) {
		z_acl = z_acl - 65536;
	}
	if (x_gyr > 32767) {
		x_gyr = x_gyr - 65536;
	}
	if (y_gyr > 32767) {
		y_gyr = y_gyr - 65536;
	}

	// calculating degree using acceleration sensor 
	double pitch = atan2(sqrt(y_acl*y_acl + z_acl*z_acl), -x_acl) * 180 / PI - 90;
	double roll = atan2(z_acl, y_acl) * 180 / PI + 90;

	std::cout << pitch - threshold_X << "," << roll - threshold_Y << std::endl;
	//std::cout << threshold_X << "," << threshold_Y << std::endl;
	//std::cout << botton << std::endl;

	if (threshold_X == 0 && threshold_Y == 0) {
		threshold_X = roll;
		threshold_Y = pitch;
	}

	if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
		InitMousePos();
		threshold_X = roll;
		threshold_Y = pitch;
	}

	if (abs(roll - threshold_X) > DEGREE_THRESHOLD) roll =  (roll - threshold_X)* X_MULTIPLE;
	else  roll = roll - threshold_X;
	if (abs(pitch - threshold_Y) > DEGREE_THRESHOLD) pitch = (pitch - threshold_Y) * Y_MULTIPLE ;
	else pitch = pitch - threshold_Y;

	if ((botton ^ 65280) == 128) {
		InitMousePos();
		threshold_X = roll;
		threshold_Y = pitch;
	}
	if(!AdjustmentMode(botton,roll,pitch) && !MoveWindowMode(botton,roll,pitch)) MoveMouse(roll,pitch);

	ClickMouse(botton);

}

void ClickMouse(int Botton) {
	static bool click_flag = false;
	if (!click_flag && ((Botton & 256) == 0)) {
		mouse_event(MOUSEEVENTF_LEFTDOWN, nowMauseX, nowMauseY, 0, NULL);
		mouse_event(MOUSEEVENTF_LEFTUP, nowMauseX, nowMauseY, 0, NULL);
		click_flag = true;
	}
	else if (click_flag && ((Botton & 256) == 0)) {
		click_flag = false;
	}
	if (!click_flag && ((Botton & 512) == 0)) {
		mouse_event(MOUSEEVENTF_RIGHTDOWN, nowMauseX, nowMauseY, 0, NULL);
		click_flag = true;
	}
	else if (click_flag && ((Botton & 512) == 0)) {
		mouse_event(MOUSEEVENTF_RIGHTUP, nowMauseX, nowMauseY, 0, NULL);
		click_flag = false;
	}
}

bool MoveWindowMode(int Botton, int x, int y) {
	static bool click_flag = false;
	static bool movewin_mode = false;

	if (!click_flag && ((Botton ^ 65280) == 2048)) {
		click_flag = true;
	}
	else if (click_flag && !((Botton ^ 65280) == 2048)) {
		ClearDrawScreen();
		DrawFormatString(0, 0, StringColor, "start move window mode \n");
		DrawGraph(300, 200, LogoGraph, TRUE);
		ScreenFlip();
		if (!movewin_mode) movewin_mode = true;
		else {
			ClearDrawScreen();
			movewin_mode = false;
			ScreenFlip();
		}
		click_flag = false;
	}

	if (movewin_mode) {
		char buf[1000];
		HWND hWnd;
		RECT rect;
		static int width = GetSystemMetrics(SM_CXSCREEN);
		static int height = GetSystemMetrics(SM_CYSCREEN);
		hWnd = GetForegroundWindow();
		GetWindowRect(hWnd, &rect);
		ScreenFlip();
		if (IsZoomed(hWnd)) {
			if (y < -40) ShowWindow(hWnd, SW_RESTORE);
		}
		else {
			if( y > 60) ShowWindow(hWnd, SW_MAXIMIZE);
			if(x > 60) MoveWindow(hWnd,0,0,width/2,height,TRUE);
			else if(x < -60) MoveWindow(hWnd, width / 2, 0 , width / 2, height, TRUE);
		}
		return true;
	}

	return false;
}

bool AdjustmentMode(int Botton,int x,int y) {
	static bool click_flag = false;
	static bool adjust_mode = false;
	static int x_pos=200, y_pos=200;
	if (!click_flag && ((Botton ^ 65280) == 1024)) {
		click_flag = true;
	}
	else if (click_flag && !((Botton ^ 65280) == 1024)) {
		DrawFormatString(0, 0, StringColor, "start adjustment mode \n");

		if (!adjust_mode) adjust_mode = true;
		else {
			ClearDrawScreen();
			adjust_mode = false;
			ScreenFlip();
		}
		click_flag = false;
	}

	if (adjust_mode) {

		unsigned int Cr = GetColor(0, 0, 0);
		if (std::abs(x) > THRESHOLD && std::abs(y) > THRESHOLD) {
			if (std::abs(x) < std::abs(y)) {
				if (y > 0) y_pos += 5;
				else y_pos -= 5;
			}
			else {
				if (x > 0) x_pos += 5;
				else x_pos -= 5;
			}
			if (x_pos > 400) x_pos = 400;
			else if (x_pos < 0) x_pos = 0;
			if (y_pos > 400) y_pos = 400;
			else if (y_pos < 0) y_pos = 0;
		}
		X_adjust_para = x_pos % 16;
		Y_adjust_para = y_pos % 16;
		UpdatePara();
		ClearDrawScreen();
		DrawBox(10, 20, 620, 450, BoxColor, TRUE);
		DrawFormatString(305,35 , StringColor, "DOWN");
		DrawFormatString(305, 435, StringColor, "UP");
		DrawFormatString(115, 230, StringColor, "DOWN");
		DrawFormatString(515, 230, StringColor, "UP");
		DrawLine(115, 230, 515, 230, Cr);
		DrawLine(305, 35, 305, 435, Cr);
		DrawBox(x_pos+95, 220, x_pos+20+95, 240, Cr, TRUE);
		DrawBox(295, y_pos+10, 315, y_pos + 20+10, Cr, TRUE);
		ScreenFlip();
		return true;
	}
	return false;
}

void ReceivefromCom()
{
	if (hCom != INVALID_HANDLE_VALUE)
	{
		DWORD dwError;
		COMSTAT comStat;
		if (ClearCommError(hCom, &dwError, &comStat))
		{
			if (comStat.cbInQue)
			{
				char* buff = new char[comStat.cbInQue];
				DWORD NumberOfBytesRead = 0;
				if (ReadFile(hCom, buff, comStat.cbInQue, &NumberOfBytesRead, NULL))
				{
					CString strText(buff, NumberOfBytesRead);
					message0001 = message0001 + strText;
					if (strText.Find("\n") != -1) {
						parseMessage(message0001);
						message0001 = "";
					}
				}
				delete[] buff;
			}
		}
	}
}

void OnButtonClose()
{
	if (hCom != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hCom);
		hCom = INVALID_HANDLE_VALUE;
	}
}

void MoveMouse(int difX, int difY) {

	if (std::abs(difX) > THRESHOLD && std::abs(difY) > THRESHOLD) {
		if (difX < 0)  nowMauseX -= difX*X_DOWN_MULTIPLE;
		else nowMauseX -= difX*X_UP_MULTIPLE;
		if (difY < 0) nowMauseY -= difY*Y_DOWN_MULTIPLE;
		else nowMauseY -= difY*Y_UP_MULTIPLE ;
	}

	mouse_event(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE, nowMauseX, nowMauseY, 0, NULL);
}

void DoGetForegroundWindow()
{
	char buf[1000];
	HWND hWnd;

	/* フォアグラウンドウィンドウの取得 */
	hWnd = GetForegroundWindow();

	/* ウィンドウタイトルの表示 */
	GetWindowText(hWnd, buf, 1000);
	MessageBox(NULL, buf, "DoGetForegroundWindow", MB_OK);

}

void UpdatePara() {
	int X_MULTIPLE = 0.5 * X_adjust_para;
	int Y_MULTIPLE = 0.5 * Y_adjust_para;
	int X_UP_MULTIPLE = 2.5 * X_adjust_para;
	int X_DOWN_MULTIPLE = 5 * X_adjust_para;
	int  Y_UP_MULTIPLE = 2.5 * Y_adjust_para;
	int  Y_DOWN_MULTIPLE = 5 * Y_adjust_para;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	SetBackgroundColor(255, 255, 255);
	if (DxLib_Init() == -1)  return -1;
	SetDrawScreen(DX_SCREEN_BACK);
	ChangeWindowMode(TRUE);
	StringColor = GetColor(0, 0, 0);
	BoxColor = GetColor(220, 220, 220);
	SetMainWindowText("HSGW ~Hand Signed Gesture UI for Windows~");
	LogoGraph = LoadGraph("logo.png");
	DrawGraph(300, 200, LogoGraph, TRUE);

	DrawFormatString(0, 0, StringColor, "Connecting to COM PORT\n");
	if (ConnectCOM()) {
		DrawFormatString(0, 16, StringColor, "Connecting Success\n");
		ScreenFlip();
	}else {
		DrawFormatString(0, 16, StringColor, "Connecting failed\n");
		DrawFormatString(0, 32, StringColor, "Press left SHIFT to end program \n");
		ScreenFlip();
		while (1) {
			if (GetAsyncKeyState(VK_LSHIFT) & 0x8000) {
				outputfile.close();
				return 0;
			}
		}
	}
	DrawFormatString(0, 48, StringColor, "Start HSGW \n");

	outputfile << "# X Y " << std::endl;

	while (1) {
		if (GetAsyncKeyState(VK_RSHIFT) & 0x8000) {
			InitMousePos();
			outputfile.close();
			DxLib_End();
			return 0;
		}
		ReceivefromCom();
	}

	return 0;				// ソフトの終了 
}
