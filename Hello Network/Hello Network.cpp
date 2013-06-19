/* Link with library file wsock32.lib */
#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#define SIZE 500




// Prototypes
void Client(char* client_ip, unsigned short client_port, char* server_ip, unsigned short server_port);
void Server(char* client_ip, unsigned short client_port, char* server_ip, unsigned short server_port);
void inkStartNetwork(void);
void inkStopNetwork(void);
SOCKET inkCreateSocket(char* ip_address, unsigned short port);
void inkDestroySocket(SOCKET s);
void inkConnect(SOCKET s, char* dest_ip, unsigned short dest_port);
void inkSend(SOCKET s, void* data, int datasize);
int inkRecieve(SOCKET s, void* data, int datasize);
void inkErr(char* err);




// Code
int main(int argc, char **argv)
{
	char type[256];
	char c_ip[256], s_ip[256];
	char *client_ip = c_ip, *server_ip = s_ip;
	unsigned short client_port, server_port;

	inkStartNetwork();

	printf("Select computer type: (Client / Server):");
	scanf("%s", type);
	if(type[0] == 's' || type[0] == 'S')
	{
		server_ip = "127.0.0.1";
		printf("Enter server port:");
		scanf("%d", &server_port);
		printf("Enter client. IP address:");
		scanf("%s", client_ip);
		printf("Enter client port:");
		scanf("%d", &client_port);
		printf("\n\nWaiting . . .\n");
		Server(client_ip, client_port, server_ip, server_port);
	}
	else
	{
		client_ip = "127.0.0.1";
		printf("Enter client port:");
		scanf("%d", &client_port);
		printf("Enter server. IP address:");
		scanf("%s", server_ip);
		printf("Enter server port:");
		scanf("%d", &server_port);
		printf("\n\nWaiting . . .\n");
		Client(client_ip, client_port, server_ip, server_port);
	}
	inkStopNetwork();
	system("PAUSE");
	return(0);
}




void Client(char* client_ip, unsigned short client_port, char* server_ip, unsigned short server_port)
{
	int i;
	SOCKET s;
	char data_send[256] = "I am the client.";
	char data_recv[256];
	LARGE_INTEGER freqI, cstart, cstop, sstart, sstop;
	double freq;

	QueryPerformanceFrequency(&freqI);
	freq = (double)freqI.QuadPart;
	s = inkCreateSocket(client_ip, client_port);
	QueryPerformanceCounter(&cstart);
	inkConnect(s, server_ip, server_port);
	QueryPerformanceCounter(&cstop);
	printf("Connection time = %.12f s\n", ((double)(cstop.QuadPart-cstart.QuadPart))/freq);
	printf("Sending and recieving data 1000 times . . .\n\n");
	QueryPerformanceCounter(&sstart);
	for(i=0; i<1000; i++)
	{
		inkSend(s, data_send, sizeof(data_send));
		inkRecieve(s, data_recv, 256);
	}
	QueryPerformanceCounter(&sstop);
	printf("Round Trip time = %.12f s\n", ((double)(sstop.QuadPart-sstart.QuadPart))/(freq*1000));
	inkDestroySocket(s);
}


void Server(char* client_ip, unsigned short client_port, char* server_ip, unsigned short server_port)
{
	int i;
	SOCKET s;
	char data_send[256] = "I am the server.";
	char data_recv[256];

	s = inkCreateSocket(server_ip, server_port);
	inkConnect(s, client_ip, client_port);
	for(i=0; i<1000; i++)
	{
		inkRecieve(s, data_recv, 256);
		printf("Data recieved = %s\n", data_recv);
		inkSend(s, data_send, sizeof(data_send));
		printf("Data sent = %s\n", data_send);
	}
	inkDestroySocket(s);
}





void inkStartNetwork(void)
{
	WSADATA wsaData;

	// Start Windows Socket service
	if (WSAStartup(0x0101, &wsaData) != 0) inkErr("Windows socket service failed to open.\n");
	printf("Network started.\n");
}


void inkStopNetwork(void)
{
	// Stop Socket Service
	WSACleanup();
	printf("Network stopped.\n");
}


SOCKET inkCreateSocket(char* ip_address, unsigned short port)
{
	SOCKET s;
	struct sockaddr_in service;

	// Create a datagram socket
	if((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) inkErr("Could not create a TCP socket.");

	// Set service properties
	ZeroMemory(&service, sizeof(struct sockaddr_in));
	service.sin_family = AF_INET;
	service.sin_addr.s_addr = 	inet_addr(ip_address);
	service.sin_port = htons(port);

	// Bind local address to socket
	if (bind(s, (struct sockaddr *)&service, sizeof(struct sockaddr_in)) != 0)
	{
		closesocket(s);
		inkErr("Bind name to socket failed.");
	}
	printf("Socket with IP(%s) and port(%d) created.\n", ip_address, port);
	return(s);
}


void inkDestroySocket(SOCKET s)
{
	// Close the socket
	if((closesocket(s)) != 0) inkErr("Socket could not be closed.");
	printf("Socket destroyed.\n");
}


void inkConnect(SOCKET s, char* dest_ip, unsigned short dest_port)
{
	struct sockaddr_in dest;

	// Set service properties
	ZeroMemory(&dest, sizeof(struct sockaddr_in));
	dest.sin_family = AF_INET;
	dest.sin_addr.s_addr = 	inet_addr(dest_ip);
	dest.sin_port = htons(dest_port);
	if(connect(s, (sockaddr *)&dest, sizeof(dest)) != 0) inkErr("Connection failed.");
	else printf("Connection established.\n\n");
}


void inkSend(SOCKET s, void* data, int datasize)
{
	// Send data
	if((send(s, (char*)data, datasize, 0)) == SOCKET_ERROR) inkErr("Sending of Packet Failed.\n");
}


int inkRecieve(SOCKET s, void* data, int datasize)
{
	// Recieve data
	while((recv(s, (char*)data, datasize, 0)) == SOCKET_ERROR);
	return(0);
}


void inkErr(char* err)
{
	printf("%s\n\n", err);
	system("PAUSE");
	exit(0);
}



