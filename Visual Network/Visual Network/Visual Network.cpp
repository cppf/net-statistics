#include "stdafx.h"
#include "Visual Network.h"
#include <stdio.h>
#include <winsock2.h>

#define MAX_LOADSTRING	100
#define CONNECT_TRIES	5
#define RECIEVE_TRIES	1
#define TIMES_TO_SEND	10



// Global Variables:
HWND hDLG;
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
TCHAR FreeSpace[512];
TCHAR StatusStr[4096] = {_T('\0')};
double inkTimerFreq;




// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
void inkProcess(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
void inkStatus(TCHAR* stat);
void Client(TCHAR* client_ip, unsigned short client_port, TCHAR* server_ip, unsigned short server_port);
void Server(TCHAR* client_ip, unsigned short client_port, TCHAR* server_ip, unsigned short server_port);
inline int inkStartNetwork(void);
inline int inkStopNetwork(void);
SOCKET inkCreateSocket(TCHAR* ip_address, unsigned short port);
inline int inkDestroySocket(SOCKET s);
SOCKET inkAccept(SOCKET s);
int inkConnect(SOCKET s, TCHAR* dest_ip, unsigned short dest_port);
inline int inkSend(SOCKET s, void* data, int datasize);
int inkRecieve(SOCKET s, void* data, int datasize);
inline char* inkUnicodeToAnsi(char* dest, int destsz, WCHAR* src);
inline WCHAR* inkAnsiToUnicode(WCHAR* dest, int destsz, char* src);
inline void inkStartTimer(void);
inline double inkTime(void);





int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_VISUALNETWORK, szWindowClass, MAX_LOADSTRING);
	DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), NULL, About);
	PostQuitMessage(0);
	ExitProcess(0);
	return (0);
}





// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	unsigned short wID = LOWORD(wParam);

	switch (message)
	{
	case WM_INITDIALOG:
		hDLG = hDlg;
		SetDlgItemText(hDlg, IDC_EDIT_DESTIP, _T("127.0.0.1"));
		SetDlgItemText(hDlg, IDC_EDIT_CLIENTPORT, _T("2000"));
		SetDlgItemText(hDlg, IDC_EDIT_SERVERPORT, _T("3000"));
		CheckDlgButton(hDlg, IDC_RADIO_CLIENT, BST_CHECKED);
		return (INT_PTR)TRUE;

	case WM_CLOSE:
		EndDialog(hDlg, wID);
		return((INT_PTR)TRUE);

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_COMMAND:
		switch(wID)
		{
		case IDC_OK:
			EndDialog(hDlg, wID);
			return (INT_PTR)TRUE;

		case IDC_SCAN:
			inkProcess(hDlg, message, wParam, lParam);
			inkStatus(_T("Done.\n\n"));
			break;
		}
	}
	return (INT_PTR)FALSE;
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
	type = (IsDlgButtonChecked(hDlg, IDC_RADIO_CLIENT) == BST_CHECKED);
	typex = (IsDlgButtonChecked(hDlg, IDC_RADIO_SERVER) == BST_CHECKED);
	inkStatus(FreeSpace);
	type = type | (typex<<1);
	client_port = GetDlgItemInt(hDlg, IDC_EDIT_CLIENTPORT, NULL, FALSE);
	server_port = GetDlgItemInt(hDlg, IDC_EDIT_SERVERPORT, NULL, FALSE);
	if(type == 2)
	{
		lstrcpy(server_ip, _T("127.0.0.1"));
		GetDlgItemText(hDlg, IDC_EDIT_DESTIP, client_ip, 256);
		Server(client_ip, client_port, server_ip, server_port);
	}
	else
	{
		lstrcpy(client_ip, _T("127.0.0.1"));
		GetDlgItemText(hDlg, IDC_EDIT_DESTIP, server_ip, 256);
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
	double cstart, cstop, sstart, sstop;

	inkStartTimer();
	if((s = inkCreateSocket(client_ip, client_port)) == INVALID_SOCKET) return;
	inkStatus(_T("Client Started.\n"));
	cstart = inkTime();
	if(inkConnect(s, server_ip, server_port) != 0) return;
	cstop = inkTime();
	swprintf_s(FreeSpace, 512, _T("Connection time = %f ms.\n"), (cstop - cstart) * 1000);
	inkStatus(FreeSpace);
	sstart = inkTime();
	for(i=0; i<TIMES_TO_SEND; i++)
	{
		if(inkSend(s, data_send, sizeof(data_send)) == SOCKET_ERROR) return;
		if(inkRecieve(s, data_recv, 256) == SOCKET_ERROR) return;
	}
	sstop = inkTime();
	swprintf_s(FreeSpace, _T("Round Trip time = %f ms\n"), (((sstop - sstart) * 1000) / TIMES_TO_SEND));
	inkStatus(FreeSpace);
	if(inkDestroySocket(s) != 0) return;
}


void Server(TCHAR* client_ip, unsigned short client_port, TCHAR* server_ip, unsigned short server_port)
{
	UNREFERENCED_PARAMETER(client_ip);
	int i;
	SOCKET s0, s;
	TCHAR data_send[256] = _T("I am the server.");
	TCHAR data_recv[256];

	if((s0 = inkCreateSocket(server_ip, server_port)) == INVALID_SOCKET) return;
	inkStatus(_T("Server Started.\n"));
	if((s = inkAccept(s0)) == INVALID_SOCKET) return;
	for(i=0; i<10; i++)
	{
		if(inkRecieve(s, data_recv, 256) == SOCKET_ERROR) return;
		if(inkSend(s, data_send, sizeof(data_send)) == SOCKET_ERROR) return;
	}
	inkDestroySocket(s);
}





inline int inkStartNetwork(void)
{
	int ret;
	WSADATA wsaData;

	// Start Windows Socket service
	if ((ret = WSAStartup(0x0101, &wsaData)) != 0) inkStatus(_T("Windows socket service failed to open.\n"));
	return(ret);
}


inline int inkStopNetwork(void)
{
	int ret;

	// Stop Socket Service
	if((ret = WSACleanup()) != 0) inkStatus(_T("Network Service could not be stopped.\n"));
	return(ret);
}


SOCKET inkCreateSocket(TCHAR* ip_address, unsigned short port)
{
	char ip_addr[64];
	SOCKET s;
	struct sockaddr_in service;

	// Create a datagram socket
	if((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) inkStatus(_T("Could not create a TCP socket.\n"));

	// Set service properties
	ZeroMemory(&service, sizeof(struct sockaddr_in));
	service.sin_family = AF_INET;
	service.sin_addr.s_addr = inet_addr(inkUnicodeToAnsi(ip_addr, 64, ip_address));
	service.sin_port = htons(port);

	// Bind local address to socket
	if (bind(s, (struct sockaddr *)&service, sizeof(struct sockaddr_in)) != 0)
	{
		closesocket(s);
		inkStatus(_T("Bind name to socket failed.\n"));
	}
	return(s);
}





SOCKET inkAccept(SOCKET s)
{
	struct sockaddr_in client;
	int addrlen;
	WCHAR clientip[64];
	SOCKET sc;

	if((sc = (SOCKET)listen(s, SOMAXCONN)) != 0) {inkStatus(_T("Failed to listen for connections.\n")); return(sc);}
	if((sc = accept(s, (sockaddr *)&client, &addrlen)) == INVALID_SOCKET) {inkStatus(_T("Failed to accept connection.\n")); return(sc);}
	inkAnsiToUnicode(clientip, 64, inet_ntoa(client.sin_addr));
	wsprintf(FreeSpace, _T("Client connected - IP(%s), port(%d).\n"), clientip, client.sin_port);
	inkStatus(FreeSpace);
	return(sc);
}




inline int inkDestroySocket(SOCKET s)
{
	int ret;

	// Close the socket
	if((ret = closesocket(s)) != 0) inkStatus(_T("Socket could not be closed.\n"));
	return(ret);
}


int inkConnect(SOCKET s, TCHAR* dest_ip, unsigned short dest_port)
{
	int ret;
	UINT i;
	struct sockaddr_in dest;
	char dest_ipv[64];

	// Set service properties
	ZeroMemory(&dest, sizeof(struct sockaddr_in));
	dest.sin_family = AF_INET;
	dest.sin_addr.s_addr = 	inet_addr(inkUnicodeToAnsi(dest_ipv, 64, dest_ip));;
	dest.sin_port = htons(dest_port);
	for(i=0; i<CONNECT_TRIES; i++)
	if((ret = connect(s, (sockaddr *)&dest, sizeof(dest))) == 0) break;
	if(ret != 0) inkStatus(_T("Connection failed.\n"));
	return(ret);
}


inline int inkSend(SOCKET s, void* data, int datasize)
{
	int ret;
	// Send data
	if((ret = send(s, (char*)data, datasize, 0)) == SOCKET_ERROR) inkStatus(_T("Sending of Packet Failed.\n"));
	return(ret);
}


int inkRecieve(SOCKET s, void* data, int datasize)
{
	int ret;
	UINT i;

	// Recieve data
	for(i=0; i<RECIEVE_TRIES; i++)
	if((ret = recv(s, (char*)data, datasize, 0)) != SOCKET_ERROR) break;
	if(ret == SOCKET_ERROR) inkStatus(_T("Recieving of Packet Failed.\n"));
	return(ret);
}



inline char* inkUnicodeToAnsi(char* dest, int destsz, WCHAR* src)
{
	BOOL usedefault;

	WideCharToMultiByte(CP_ACP, 0, src, -1, dest, destsz, 0, &usedefault);
	return(dest);
}



inline WCHAR* inkAnsiToUnicode(WCHAR* dest, int destsz, char* src)
{
	MultiByteToWideChar(CP_ACP, 0, src, -1, dest, destsz);
	return(dest);
}



inline void inkStartTimer(void)
{
	LARGE_INTEGER freq;

	QueryPerformanceFrequency(&freq);
	inkTimerFreq = (double)freq.QuadPart;
}



inline double inkTime(void)
{
	LARGE_INTEGER time;

	QueryPerformanceCounter(&time);
	return(time.QuadPart / inkTimerFreq);
}