/* Link with library file wsock32.lib */
#include "stdafx.h"
#include "Network GUI.h"
#include <winsock2.h>
#define MAX_LOADSTRING 100






// Global Variables:
HINSTANCE hInst;
HWND hDLG;
TCHAR szTitle[MAX_LOADSTRING];
TCHAR szWindowClass[MAX_LOADSTRING];
TCHAR FreeSpace[512];
TCHAR StatusStr[4096] = {_T('\0')};





// Prototypes
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
void inkProcess(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
void Client(TCHAR* client_ip, unsigned short client_port, TCHAR* server_ip, unsigned short server_port);
void Server(TCHAR* client_ip, unsigned short client_port, TCHAR* server_ip, unsigned short server_port);
void inkStartNetwork(void);
void inkStopNetwork(void);
SOCKET inkCreateSocket(TCHAR* ip_address, unsigned short port);
void inkDestroySocket(SOCKET s);
void inkConnect(SOCKET s, TCHAR* dest_ip, unsigned short dest_port);
void inkSend(SOCKET s, void* data, int datasize);
int inkRecieve(SOCKET s, void* data, int datasize);
void inkErr(TCHAR* err);






int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	MSG msg;
	HACCEL hAccelTable;
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_NETWORKGUI, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);
	if (!InitInstance (hInstance, nCmdShow))
	{return FALSE;}
	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_NETWORKGUI));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return((int)msg.wParam);
}


ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_NETWORKGUI));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_NETWORKGUI);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
	return(RegisterClassEx(&wcex));
}


BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}





// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	UINT cmd = LOWORD(wParam);
	hDLG = hDlg;

	switch (message)
	{
	case WM_INITDIALOG:
		SetDlgItemInt(hDlg, IDC_EDIT_SRCPORT, 2000, FALSE);
		SetDlgItemText(hDlg, IDC_EDIT_DESTIP, _T("127.0.0.1"));
		SetDlgItemInt(hDlg, IDC_EDIT_DESTPORT, 3000, FALSE);
		SetFocus(hDlg);
		return((INT_PTR)TRUE);

	case WM_COMMAND:
		switch(cmd)
		{
		case ID_CLOSE:
			EndDialog(hDlg, LOWORD(wParam));
			return((INT_PTR)TRUE);

		case ID_MONITOR:
			inkProcess(hDlg, message, wParam, lParam);
			return((INT_PTR)TRUE);

		}
		break;

	case WM_CLOSE:
		EndDialog(hDlg, LOWORD(wParam));
		return((INT_PTR)TRUE);
	}
	return((INT_PTR)FALSE);
}


void inkStatus(TCHAR* stat)
{
	lstrcat(StatusStr, stat);
	SetDlgItemText(hDLG, IDC_STATUS, StatusStr);
}



void inkProcess(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UINT type, typex;
	TCHAR client_ip[256], server_ip[256];
	unsigned short client_port, server_port;

	inkStartNetwork();
	type = GetDlgItemInt(hDlg, IDC_RADIO_CLIENT, NULL, FALSE);
	typex = GetDlgItemInt(hDlg, IDC_RADIO_SERVER, NULL, FALSE);
	type = type | (typex<<1);
	if(type == 2)
	{
		lstrcpy(server_ip, _T("127.0.0.1"));
		server_port = GetDlgItemInt(hDlg, IDC_EDIT_SRCPORT, NULL, FALSE);
		GetDlgItemText(hDlg, IDC_EDIT_DESTIP, client_ip, 256);
		client_port = GetDlgItemInt(hDlg, IDC_EDIT_DESTPORT, NULL, FALSE);
		inkStatus(_T("\n\nWaiting . . .\n"));
		Server(client_ip, client_port, server_ip, server_port);
	}
	else
	{
		lstrcpy(client_ip, _T("127.0.0.1"));
		client_port = GetDlgItemInt(hDlg, IDC_EDIT_SRCPORT, NULL, FALSE);
		GetDlgItemText(hDlg, IDC_EDIT_DESTIP, server_ip, 256);
		server_port = GetDlgItemInt(hDlg, IDC_EDIT_DESTPORT, NULL, FALSE);
		inkStatus(_T("\n\nWaiting . . .\n"));
		Client(client_ip, client_port, server_ip, server_port);
	}
	inkStopNetwork();
}




void Client(TCHAR* client_ip, unsigned short client_port, TCHAR* server_ip, unsigned short server_port)
{
	int i;
	SOCKET s;
	TCHAR data_send[256] = _T("I am the client.");
	TCHAR data_recv[256];
	LARGE_INTEGER freqI, cstart, cstop, sstart, sstop;
	double freq;

	QueryPerformanceFrequency(&freqI);
	freq = (double)freqI.QuadPart;
	s = inkCreateSocket(client_ip, client_port);
	QueryPerformanceCounter(&cstart);
	inkConnect(s, server_ip, server_port);
	QueryPerformanceCounter(&cstop);
	wsprintf(FreeSpace, _T("Connection time = %.12f s\n"), ((double)(cstop.QuadPart-cstart.QuadPart))/freq);
	inkStatus(FreeSpace);
	wsprintf(FreeSpace, _T("Sending and recieving data 1000 times . . .\n\n"));
	inkStatus(FreeSpace);
	QueryPerformanceCounter(&sstart);
	for(i=0; i<1000; i++)
	{
		inkSend(s, data_send, sizeof(data_send));
		inkRecieve(s, data_recv, 256);
	}
	QueryPerformanceCounter(&sstop);
	wsprintf(FreeSpace, _T("Round Trip time = %.12f s\n"), ((double)(sstop.QuadPart-sstart.QuadPart))/(freq*1000));
	inkStatus(FreeSpace);
	inkDestroySocket(s);
}


void Server(TCHAR* client_ip, unsigned short client_port, TCHAR* server_ip, unsigned short server_port)
{
	int i;
	SOCKET s;
	TCHAR data_send[256] = _T("I am the server.");
	TCHAR data_recv[256];

	s = inkCreateSocket(server_ip, server_port);
	inkConnect(s, client_ip, client_port);
	for(i=0; i<1000; i++)
	{
		inkRecieve(s, data_recv, 256);
		wsprintf(FreeSpace, _T("Data recieved = %s\n"), data_recv);
		inkStatus(FreeSpace);
		inkSend(s, data_send, sizeof(data_send));
		wsprintf(FreeSpace, _T("Data sent = %s\n"), data_send);
		inkStatus(FreeSpace);
	}
	inkDestroySocket(s);
}





void inkStartNetwork(void)
{
	WSADATA wsaData;

	// Start Windows Socket service
	if (WSAStartup(0x0101, &wsaData) != 0) inkErr(_T("Windows socket service failed to open.\n"));
	inkStatus(_T("Network started.\n"));
}


void inkStopNetwork(void)
{
	// Stop Socket Service
	WSACleanup();
	inkStatus(_T("Network stopped.\n"));
}


SOCKET inkCreateSocket(TCHAR* ip_address, unsigned short port)
{
	SOCKET s;
	struct sockaddr_in service;

	// Create a datagram socket
	if((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) inkErr(_T("Could not create a TCP socket."));

	// Set service properties
	ZeroMemory(&service, sizeof(struct sockaddr_in));
	service.sin_family = AF_INET;
	service.sin_addr.s_addr = 	inet_addr(ip_address);
	service.sin_port = htons(port);

	// Bind local address to socket
	if (bind(s, (struct sockaddr *)&service, sizeof(struct sockaddr_in)) != 0)
	{
		closesocket(s);
		inkErr(_T("Bind name to socket failed."));
	}
	wsprintf(FreeSpace, _T("Socket with IP(%s) and port(%d) created.\n"), ip_address, port);
	inkStatus(FreeSpace);
	return(s);
}


void inkDestroySocket(SOCKET s)
{
	// Close the socket
	if((closesocket(s)) != 0) inkErr(_T("Socket could not be closed."));
	inkStatus(_T("Socket destroyed.\n"));
}


void inkConnect(SOCKET s, TCHAR* dest_ip, unsigned short dest_port)
{
	struct sockaddr_in dest;

	// Set service properties
	ZeroMemory(&dest, sizeof(struct sockaddr_in));
	dest.sin_family = AF_INET;
	dest.sin_addr.s_addr = 	inet_addr(dest_ip);
	dest.sin_port = htons(dest_port);
	if(connect(s, (sockaddr *)&dest, sizeof(dest)) != 0) inkErr(_T("Connection failed."));
	else inkStatus(_T("Connection established.\n\n"));
}


void inkSend(SOCKET s, void* data, int datasize)
{
	// Send data
	if((send(s, (TCHAR*)data, datasize, 0)) == SOCKET_ERROR) inkErr(_T("Sending of Packet Failed.\n"));
}


int inkRecieve(SOCKET s, void* data, int datasize)
{
	// Recieve data
	while((recv(s, (TCHAR*)data, datasize, 0)) == SOCKET_ERROR);
	return(0);
}


void inkErr(TCHAR* err)
{
	inkStatus(err);
	inkStatus("\n\n");
}



