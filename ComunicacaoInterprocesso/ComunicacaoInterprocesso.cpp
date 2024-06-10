
#include "RemoteProcedureCall.h"
#include <stdio.h>

void teste(void* parameters)
{
	char *msg = (char*)parameters;
	printf(msg);
	char buffer[50];
	strncpy_s(buffer, "funcao chamada", sizeof(buffer) - 1);
	memcpy(parameters, buffer,sizeof(buffer));
}

void testeServer() {
	char buffer[256];
	strncpy_s(buffer, "teste", sizeof(buffer) - 1);
	RemoteProcedureCall rpcServer("teste", true);
	rpcServer.WaitClient();
	rpcServer.CallRemoteProcedure("teste", buffer, sizeof(buffer));
	rpcServer.GetMessage(buffer, sizeof(buffer));
}

void teste() {
	char buffer[256];
	strncpy_s(buffer, "teste", sizeof(buffer) - 1);
	RemoteProcedureCall rpcServer("teste", true);
	RemoteProcedureCall rpcClient("teste", false);
	rpcClient.RegisterProcedure("teste", teste);
	rpcClient.StartClient();
	Sleep(1000);
	rpcServer.CallRemoteProcedure("teste", buffer, sizeof(buffer));
	rpcServer.GetMessage(buffer, sizeof(buffer));
}

int main()
{

	testeServer();
}

