#pragma once

void InitDecompilerInfo(RpcInterfaceInfo_T* pRpcInterfaceInfo, RpcDecompilerInfo_T* pRpcDecompilerInfo)
{
	UINT	i;
	UINT	SymboleLength;
	HANDLE	hProcess = NULL;
	void* hPdb = NULL;
	WCHAR	SymboleName[RPC_MAX_LENGTH];

	if (pRpcDecompilerInfo == NULL) goto End;
	if (pRpcInterfaceInfo == NULL)	goto End;

	pRpcDecompilerInfo->pModuleBase = (UINT64)pRpcInterfaceInfo->pLocationBase;
	pRpcDecompilerInfo->pIfId = &pRpcInterfaceInfo->If;
	pRpcDecompilerInfo->Pid = pRpcInterfaceInfo->Pid;
	StringCbPrintfW(pRpcDecompilerInfo->InterfaceName, sizeof(pRpcDecompilerInfo->InterfaceName), L"%s", pRpcInterfaceInfo->Name);
	pRpcDecompilerInfo->pSyntaxId = &pRpcInterfaceInfo->TransfertSyntax;

	pRpcDecompilerInfo->MIDLVersion = pRpcInterfaceInfo->NdrInfo.MIDLVersion;
	pRpcDecompilerInfo->NDRFags = pRpcInterfaceInfo->NdrInfo.mFlags;
	pRpcDecompilerInfo->NDRVersion = pRpcInterfaceInfo->NdrInfo.Version;

	pRpcDecompilerInfo->NumberOfProcedures = pRpcInterfaceInfo->NumberOfProcedures;
	pRpcDecompilerInfo->ppProcAddressTable = pRpcInterfaceInfo->ppProcAddressTable;
	pRpcDecompilerInfo->pTypeFormatString = (RVA_T)((ULONG_PTR)pRpcInterfaceInfo->pTypeFormatString - (ULONG_PTR)pRpcInterfaceInfo->pLocationBase);

	pRpcDecompilerInfo->pFormatStringOffsetTable = pRpcInterfaceInfo->pFormatStringOffsetTable;
	pRpcDecompilerInfo->pProcFormatString = (RVA_T)((ULONG_PTR)pRpcInterfaceInfo->pProcFormatString - (ULONG_PTR)pRpcInterfaceInfo->pLocationBase);

	pRpcDecompilerInfo->apfnExprEval = (RVA_T)((ULONG_PTR)pRpcInterfaceInfo->apfnExprEval - (ULONG_PTR)pRpcInterfaceInfo->pLocationBase);
	pRpcDecompilerInfo->bIsInlined = FALSE;

	pRpcDecompilerInfo->pExprOffset = (RVA_T)((ULONG_PTR)pRpcInterfaceInfo->pExprOffset - (ULONG_PTR)pRpcInterfaceInfo->pLocationBase);
	pRpcDecompilerInfo->pExprFormatString = (RVA_T)((ULONG_PTR)pRpcInterfaceInfo->pExprFormatString - (ULONG_PTR)pRpcInterfaceInfo->pLocationBase);
	//
	// Cannot decompile if we cannot get the ppProcAddressTable value!!!
	//
	if (pRpcDecompilerInfo->ppProcAddressTable == NULL)
	{
		printf("*** No procedure: %u\n", pRpcDecompilerInfo->NumberOfProcedures);
		//	ExitProcess(0);
		//	goto End;
	}

	pRpcDecompilerInfo->ppProcNameTable = (WCHAR * *)OS_ALLOC(pRpcDecompilerInfo->NumberOfProcedures * sizeof(UCHAR*));
	if (pRpcDecompilerInfo->ppProcNameTable == NULL) goto End;

	hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pRpcInterfaceInfo->Pid);
	if (hProcess == NULL) goto End;
#ifdef _WIN64
	pRpcDecompilerInfo->bIs64Bits = !pRpcInterfaceInfo->bWow64Process;
#else
	pRpcDecompilerInfo->bIs64Bits = FALSE;
#endif
	//
	// Creates and initialiaze the pbFunctionInterpreted bool table
	// TODO : should be really initialized
	//
	pRpcDecompilerInfo->pbFunctionInterpreted = (BOOL*)OS_ALLOC(pRpcDecompilerInfo->NumberOfProcedures * sizeof(BOOL));
	if (pRpcDecompilerInfo->pbFunctionInterpreted == NULL) goto End;
	//
	// Creates and initialiaze the ppProcFormatInlined RVA_T table
	// TODO : should be really initialized
	//
	pRpcDecompilerInfo->ppProcFormatInlined = (RVA_T*)OS_ALLOC(pRpcDecompilerInfo->NumberOfProcedures * sizeof(RVA_T));
	if (pRpcDecompilerInfo->ppProcFormatInlined == NULL) goto End;
	//
	// Creates and initialiaze the ppDispatchProcAddressTable RVA_T table
	// TODO : should be really initialized
	//
	pRpcDecompilerInfo->ppDispatchProcAddressTable = (RVA_T*)OS_ALLOC(pRpcDecompilerInfo->NumberOfProcedures * sizeof(RVA_T));
	if (pRpcDecompilerInfo->ppDispatchProcAddressTable == NULL) goto End;

	/*hPdb = PdbInit(hProcess, pRpcInterfaceInfo->pLocationBase, pRpcInterfaceInfo->LocationSize);
	if (hPdb == NULL) goto End;
	for (i = 0; i < pRpcDecompilerInfo->NumberOfProcedures; i++)
	{
		SymboleName[0] = 0;
		if (PdbGetSymbolName(hPdb, (UCHAR*)pRpcInterfaceInfo->pLocationBase + pRpcDecompilerInfo->ppProcAddressTable[i], SymboleName, sizeof(SymboleName)))
		{
			SymboleLength = ((UINT)wcslen(SymboleName) + 1) * sizeof(WCHAR);
			pRpcDecompilerInfo->ppProcNameTable[i] = (WCHAR*)OS_ALLOC(SymboleLength);
			if (pRpcDecompilerInfo->ppProcNameTable[i] != NULL) {
				memcpy(pRpcDecompilerInfo->ppProcNameTable[i], SymboleName, SymboleLength);
			}
		}
	}
	PdbUninit(hPdb);*/
End:
	if (hProcess != NULL) CloseHandle(hProcess);
	return;
}

BOOL RpcGetInterfaceName(GUID* pUuid, UCHAR* pName, ULONG NameLength)
{
	HKEY        hKey = NULL;
	ULONG       DataLength;
	UCHAR       SubKeyName[MAX_PATH];
	RPC_CSTR    pUuidString = NULL;
	BOOL        bResult = FALSE;

	if (UuidToString(pUuid, &pUuidString) != RPC_S_OK) goto End;
	StringCbPrintf((STRSAFE_LPSTR)SubKeyName, sizeof(SubKeyName), "Interface\\{%s}", pUuidString);

	if (RegOpenKeyEx(HKEY_CLASSES_ROOT, (LPCSTR)SubKeyName, 0, KEY_READ, &hKey) != ERROR_SUCCESS) goto End;
	DataLength = NameLength;
	if (RegQueryValueEx(hKey, NULL, NULL, NULL, pName, &DataLength) != ERROR_SUCCESS) goto End;

	bResult = TRUE;
End:
	if (hKey != NULL) RegCloseKey(hKey);
	if (pUuidString != NULL) RpcStringFree(&pUuidString);
	return (bResult);
}


BOOL RpcGetProcessData(RpcModuleInfo_T* pRpcModuleInfo, RVA_T Rva, VOID* pBuffer, UINT BufferLength)
{
	BOOL    bResult = FALSE;
	HANDLE  hProcess = NULL;
	VOID* pAddress = NULL;

	if (pRpcModuleInfo == NULL) goto End;
	pAddress = (VOID*)(pRpcModuleInfo->pModuleBase + Rva);

	hProcess = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, pRpcModuleInfo->Pid);
	if (hProcess == NULL) goto End;
	bResult = ReadProcessMemory(hProcess, pAddress, pBuffer, BufferLength, NULL);
End:
	if (hProcess != NULL) CloseHandle(hProcess);
	return (bResult);
}