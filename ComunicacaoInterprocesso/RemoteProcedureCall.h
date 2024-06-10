#ifndef REMOTEPROCEDURECALL_H
#define REMOTEPROCEDURECALL_H
#include <map>
#include <string>
#include <windows.h>
#define MAXMESSAGEBUFFER 1024

typedef void (*RemoteProcedure)(void*);

class RemoteProcedureCall
{
	typedef struct
	{
		char name[256];
		char parameters[MAXMESSAGEBUFFER];
		unsigned int size;
	} Message;

public:
	RemoteProcedureCall(std::string name, bool server);
	~RemoteProcedureCall();
	void RegisterProcedure(std::string name, RemoteProcedure functionPointer);
	void StartServer();
	void StartClient();
	void CallRemoteProcedure(std::string name, void* parameters, unsigned int size);
	void SetMessage(void *dados, unsigned int size);
	void GetMessage(void* dados, unsigned int size);

private :
	// Mapa de procedimentos que mapeia uma string para uma funcão
	std::map<std::string, RemoteProcedure> procedures;
	Message message;

	bool server;

	HANDLE mutex;
	HANDLE event;
	HANDLE thread;
	HANDLE mapFile;

	static DWORD WINAPI ClientThread(LPVOID lpParam);
	void ClientThreadLoop();
	RemoteProcedure GetProcedure(std::string name);
	void CallProcedure(std::string name, void* parameters);
	bool CopyDataToMapFile();
	bool CopyDataFromMapFile();

};

#endif // REMOTEPROCEDURECALL_H