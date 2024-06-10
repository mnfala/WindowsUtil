#include "RemoteProcedureCall.h"

RemoteProcedureCall::RemoteProcedureCall(std::string name, bool server)
{
	WCHAR buffer[256];
	wsprintf(buffer, L"%s_mutex", name.c_str());
	mutex = CreateMutex(NULL, FALSE, buffer);

	wsprintf(buffer, L"%s_event", name.c_str());
	event = CreateEvent(NULL, FALSE, FALSE, buffer);

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
	if (event)
		CloseHandle(event);
	event = NULL;
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
	while (true)
	{
		DWORD  error = WaitForSingleObject(event, INFINITE);
		if (error)
			return;
		CopyDataFromMapFile();
		std::string name = message.name;
		void* parameters = message.parameters;
		CallProcedure(name, parameters);
		CopyDataToMapFile();
		error = SetEvent(event);
		if (!error)
			return;
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
	SetEvent(event);
	WaitForSingleObject(event, INFINITE);
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

