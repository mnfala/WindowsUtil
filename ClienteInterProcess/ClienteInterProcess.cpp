
#include "D:\eds\Programacao\CPP\InterProcess\WindowsUtil\ComunicacaoInterprocesso\RemoteProcedureCall.h"
#include <stdio.h>

void teste(void* parameters)
{
	char* msg = (char*)parameters;
	printf(msg);
	char buffer[50];
	strncpy_s(buffer, "funcao chamada", sizeof(buffer) - 1);
	memcpy(parameters, buffer, sizeof(buffer));
}

int main()
{
	char buffer[256];
	strncpy_s(buffer, "teste", sizeof(buffer) - 1);
	RemoteProcedureCall rpcClient("teste", false);
	rpcClient.RegisterProcedure("teste", teste);
	rpcClient.StartClient();
	while(1)
	  Sleep(1000);
}

