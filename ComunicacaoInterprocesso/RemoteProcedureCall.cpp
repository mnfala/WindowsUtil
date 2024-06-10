#include "RemoteProcedureCall.h"

RemoteProcedureCall::RemoteProcedureCall(std::string name, bool server)
{
	WCHAR buffer[256];
	wsprintf(buffer, L"%s_mutex", name.c_str());
	mutex = CreateMutex(NULL, FALSE, buffer);

	wsprintf(buffer, L"%s_eventIniciado", name.c_str());
	eventIniciado = CreateEvent(NULL, FALSE, FALSE, buffer);
	wsprintf(buffer, L"%s_eventProcessado", name.c_str());
	//o evento processado é resetado manualmente pelo cliente
	eventProcessado = CreateEvent(NULL, TRUE, FALSE, buffer);

	thread = NULL;
	wsprintf(buffer, L"%s_mapfile", name.c_str());
	if (server) {
		mapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(Message), buffer);
	}
	else {
		mapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, buffer);
	}

	this->server = server;
}

RemoteProcedureCall::~RemoteProcedureCall()
{
	if (mutex)
		CloseHandle(mutex);
	mutex = NULL;
	if (eventIniciado)
		CloseHandle(eventIniciado);
	eventIniciado = NULL;
	if (eventProcessado)
		CloseHandle(eventProcessado);
	eventProcessado = NULL;
	if (mapFile)
		CloseHandle(mapFile);
	mapFile = NULL;
	if (thread != NULL) {
		Sleep(100);
		TerminateThread(thread, 0);
		CloseHandle(thread);
	}
}

void RemoteProcedureCall::RegisterProcedure(std::string name, RemoteProcedure functionPointer)
{
	procedures[name] = functionPointer;
}

RemoteProcedure RemoteProcedureCall::GetProcedure(std::string name)
{
	return procedures[name];
}

void RemoteProcedureCall::CallProcedure(std::string name, void* parameters)
{
	procedures[name](parameters);
}

void RemoteProcedureCall::WaitClient()
{
	ResetEvent(eventIniciado);
	WaitForSingleObject(eventIniciado, INFINITE);
}

void RemoteProcedureCall::StartClient()
{
	thread = CreateThread(NULL, 0, ClientThread, this, 0, NULL);
}



DWORD WINAPI RemoteProcedureCall::ClientThread(LPVOID lpParam)
{
	RemoteProcedureCall* rpc = (RemoteProcedureCall*)lpParam;
	rpc->ClientThreadLoop();
	return 0;
}

void RemoteProcedureCall::ClientThreadLoop()
{
	DWORD  error;

	//Primeiro sinaliza a thread para o cliente saber que o servidor está pronto
	error = SetEvent(eventIniciado);
	printf("Thread cliente sinalizou\n");
	if (!error) {
		printf("Erro na sinalizacao\n");
		return;
	}
	while (true)
	{
		printf("Aguardando cliente\n");
		error = WaitForSingleObject(eventIniciado, INFINITE);
		if (error) {
			printf("Erro na espera do evento \n");
			return;
		}
		CopyDataFromMapFile();
		printf("Inicio processamento mensagem %s \n", message.name);
		std::string name = message.name;
		void* parameters = message.parameters;
		CallProcedure(name, parameters);
		CopyDataToMapFile();
		printf("Fim processamento mensagem %s \n", message.name);
		error = ResetEvent(eventIniciado);
		if (!error) {
			printf("Erro no reset de eventIniciado \n");
			return;
		}
		error = SetEvent(eventProcessado);
		if (!error) {
			printf("Erro na sinalizacao de retorno de mensagem\n");
			return;
		}
	}
}

void RemoteProcedureCall::SetMessage(void* dados, unsigned int size)
{
	memcpy(message.parameters, dados, size);
	message.size = size;
}

void RemoteProcedureCall::GetMessage(void* dados, unsigned int size)
{
	memcpy(dados, message.parameters, size);
}

void RemoteProcedureCall::CallRemoteProcedure(std::string name, void* parameters, unsigned int size)
{
	WaitForSingleObject(mutex, INFINITE);
	strcpy_s(message.name, name.c_str());
	SetMessage(parameters, size);
	CopyDataToMapFile();
	ResetEvent(eventProcessado);
	SetEvent(eventIniciado);
	WaitForSingleObject(eventProcessado, INFINITE);
	CopyDataFromMapFile();
	ReleaseMutex(mutex);
}

bool RemoteProcedureCall::CopyDataToMapFile()
{
	LPVOID buffer = MapViewOfFile(mapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(Message));
	if (buffer == NULL)
		return false;
	memcpy(buffer, &message, sizeof(Message));
	UnmapViewOfFile(buffer);
	return true;
}

bool RemoteProcedureCall::CopyDataFromMapFile()
{
	LPVOID buffer = MapViewOfFile(mapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(Message));
	if (buffer == NULL)
		return false;
	memcpy(&message, buffer, sizeof(Message));
	UnmapViewOfFile(buffer);
	return true;
}

