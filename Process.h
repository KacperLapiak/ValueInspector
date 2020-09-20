#ifndef PROCESS_H
#define PROCESS_H

#include "Headers.h"

using namespace std;

class Process
{
public:
    int currentPid_ = -1;
    int procsNumber_;
    long startAddress_ = -1;
    SIZE_T readBytes_ = 0;
    unique_ptr<DWORD[]> procsPid_{ new DWORD[procsNumber_] };
    unique_ptr<string[]> procsName_{ new string[procsNumber_] };
    unique_ptr<HANDLE[]> hProcs_{ new HANDLE[procsNumber_] };
    vector<char> bytesBuffer_;

    Process() : procsNumber_(getProcsNumb())
    {
        getProcsPid();
        getProcsName();
    }

    int getProcsNumb()
    {
        // Get the list of process identifiers.
        DWORD procsIdTemp[1024], cbNeeded;
        EnumProcesses(procsIdTemp, sizeof(procsIdTemp), &cbNeeded);

        // Calculate how many process identifiers were returned.
        return cbNeeded / sizeof(DWORD);
    }
    void getProcsPid()
    {
        // Get the list of process identifiers.
        DWORD procsIdTemp[512], cbNeeded;
        EnumProcesses(procsIdTemp, sizeof(procsIdTemp), &cbNeeded);

        // Rewrite processes ID to extern table.
        for (int i = 0; i < procsNumber_; i++)
            procsPid_[i] = procsIdTemp[i];
    }
    void openProcs()
    {
        // assumption: there is not more than 200 running processes
        HANDLE hToken[200];
        TOKEN_PRIVILEGES newState[200];
        LUID luid[200];

        // Open all processes.
        for (int i = 0; i < procsNumber_; i++)
        {
            LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid[i]);

            newState[i].PrivilegeCount = 1;
            newState[i].Privileges[0].Luid = luid[i];
            newState[i].Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

            hProcs_[i] = OpenProcess(PROCESS_ALL_ACCESS, TRUE, procsPid_[i]);

            OpenProcessToken(hProcs_[i], TOKEN_ALL_ACCESS, &hToken[i]);
            AdjustTokenPrivileges(hToken[i], FALSE, &newState[i], sizeof(TOKEN_PRIVILEGES), (PTOKEN_PRIVILEGES)NULL, (PDWORD)NULL);
        }
    }
    void closeProcs()
    {
        // Close all processes.
        for (int i = 0; i < procsNumber_; i++)
            CloseHandle(hProcs_[i]);
    }
    void getProcsName()
    {
        openProcs();

        HMODULE hMod;
        DWORD cbNeeded;

        for (int i = 0; i < procsNumber_; i++)
        {
            TCHAR bufName[40]{};
            EnumProcessModules(hProcs_[i], &hMod, sizeof(hMod), &cbNeeded);
            GetModuleBaseName(hProcs_[i], hMod, bufName, sizeof(bufName) / sizeof(TCHAR));

            for (int a = 0; a < sizeof(bufName) / sizeof(TCHAR); a++)
                procsName_[i] += bufName[a];
        }
    }
    void readProcMem(int pid, int bytes)
    {
        char buf[2048]{};
        currentPid_ = pid;
        bytesBuffer_.clear();
        if (startAddress_ == -1) startAddress_ = 1;

        // create handle for process
        HANDLE hproc = OpenProcess(PROCESS_ALL_ACCESS, TRUE, pid);

        // find first byte ready to read
        while (ReadProcessMemory(hproc, (void*)startAddress_, buf, bytes, NULL) == 0) startAddress_++;

        // read memory from first byte ready to read
        ReadProcessMemory(hproc, (void*)startAddress_, buf, bytes, &readBytes_);
        startAddress_ = (startAddress_ + readBytes_);

        // save the bytes
        for (int i = 0; i < readBytes_; i++)
            bytesBuffer_.push_back(buf[i]);

        CloseHandle(hproc);
    }
    bool isAddrOpen(int pid, int address)
    {
        char buf[2];
        HANDLE hproc = OpenProcess(PROCESS_ALL_ACCESS, TRUE, pid);
        bool res = ReadProcessMemory(hproc, (void*)address, buf, 1, NULL);
        CloseHandle(hproc);
        return res;
    }
};

#endif