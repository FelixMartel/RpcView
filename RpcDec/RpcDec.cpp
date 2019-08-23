#include <strsafe.h>
#include <iostream>
#include <string>

#include "RpcCore.h"
#include "RpcDecompiler.h"

// hide code ripped from rpcview
#include "ugly.h"

#pragma comment(lib, "rpcrt4.lib")

void consoleprint(void* ctxt, const char* stuff) {
	std::cout << stuff;
}

void ignore(const char* pFunction, ULONG Line, const char* pFormatString, ...) {
	return;
}

void dumpinterface(RpcInterfaceInfo_T* intinfo) {
	std::wcout << "Found: " << intinfo->Description << std::endl
		<< "From: " << intinfo->Location << std::endl
		<< "Base: " << intinfo->pLocationBase << std::endl
		<< "No" << "\t"<< "Name" << "\t" << "Address" << std::endl;
	for (auto i = 0u; i < intinfo->NumberOfProcedures; ++i) {
		std::wcout << i << "\t" << intinfo->Name << "\t" << (void*) intinfo->ppProcAddressTable[i] << std::endl;
	}
}

bool EnumProcessInterfacesCallback(RpcInterfaceInfo_T* intinfo, VOID* ctxt, BOOL* cont) {
	if (intinfo->IfType != IfType_RPC) { return true; }

	dumpinterface(intinfo);

	std::cout << "Decompiled idl" << std::endl;
	RpcDecompilerHelper_T* dechelper = (RpcDecompilerHelper_T*)ctxt;
	RpcDecompilerInfo_T RpcDecompilerInfo;
	InitDecompilerInfo(intinfo, &RpcDecompilerInfo);
	
	RpcViewHelper_T viewer = {NULL, malloc, free, RpcGetProcessData, consoleprint, ignore, RpcGetInterfaceName };
	VOID* decctxt = dechelper->RpcDecompilerInitFn(&viewer, &RpcDecompilerInfo);

	dechelper->RpcDecompilerPrintAllProceduresFn(decctxt);

	dechelper->RpcDecompilerUninitFn(decctxt);
	return true;
}

int main(int argc, char** argv) {
	if (argc < 2) {
		std::cout << "Usage: RpcDec [pid]" << std::endl;
		return 1;
	}

	int pid = std::stoi(argv[1]);

	HMODULE coredll = LoadLibrary("RpcCore4_64bits.dll");
	HMODULE decdll = LoadLibrary("RpcDecompiler.dll");
	if (!(coredll && decdll)) {
		std::cout << "Missing RpcDecompiler.dll and RpcCore4_64bits.dll" << std::endl;
		return 1;
	}

	RpcCore_T* rpccore = (RpcCore_T*) GetProcAddress(coredll, RPC_CORE_EXPORT_SYMBOL);
	RpcDecompilerHelper_T* dechelper = (RpcDecompilerHelper_T*)GetProcAddress(decdll, RPC_DECOMPILER_EXPORT_SYMBOL);

	VOID* corectxt = rpccore->RpcCoreInitFn(true);
	RpcProcessInfo_T* procinfo = rpccore->RpcCoreGetProcessInfoFn(corectxt, pid, 0, RPC_PROCESS_INFO_ALL);

	rpccore->RpcCoreEnumProcessInterfacesFn(corectxt, pid, (RpcCoreEnumProcessInterfacesCallbackFn_T)EnumProcessInterfacesCallback, (VOID*)dechelper, RPC_INTERFACE_INFO_ALL);

	rpccore->RpcCoreFreeProcessInfoFn(corectxt, procinfo);
	rpccore->RpcCoreUninitFn(corectxt);
	FreeLibrary(coredll);
	FreeLibrary(decdll);
}