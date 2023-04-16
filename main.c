#include <stdio.h>
#include <windows.h>
#include <psapi.h>

char* executable;
char* buff;
DWORD* addrs;
DWORD* ptchs;

int ___exit(int ret)
{
    free(executable);
    free(buff);
    free(addrs);
    free(ptchs);
    return ret;
}

DWORD_PTR get_base_addr(PROCESS_INFORMATION* pi)
{
    DWORD_PTR base_addr = 0;
    HMODULE* module_arr;
    LPBYTE module_arr_bytes;
    DWORD bytes_required;

    if (pi->hProcess)
        if (EnumProcessModules(pi->hProcess, NULL, 0, &bytes_required))
            if (bytes_required)
            {
                module_arr_bytes = (LPBYTE)LocalAlloc(LPTR, bytes_required);
                if (module_arr_bytes)
                {
                    module_arr = (HMODULE*)module_arr_bytes;

                    if (EnumProcessModules(pi->hProcess, module_arr, bytes_required, &bytes_required))
                        base_addr = (DWORD_PTR)module_arr[0];

                    LocalFree(module_arr_bytes);
                }
            }

    return base_addr;
}

int read_proc_mem(PROCESS_INFORMATION* pi, BYTE* addr, void* buff)
{
    if (ReadProcessMemory(pi->hProcess, addr, buff, sizeof(DWORD), 0))
        return 1;
    return 0;
}

int write_proc_mem(PROCESS_INFORMATION* pi, BYTE* addr, void* buff)
{
    if (WriteProcessMemory(pi->hProcess, addr, buff, sizeof(DWORD), 0))
        return 1;
    return 0;
}

int main()
{
    executable = malloc( sizeof(char)*0xFF*2 );
    FILE* f = fopen("patcher.conf", "r");
    if (!f)
        return ___exit(-1);
    if (fscanf(f, "%255[^\n]", executable) != 1)
        return ___exit(-1);
    for (int i = 0; i < 0xFF; i++)
    {
        if (!executable[i])
            break;
        if (executable[i] == '.')
            if (strcmp(&executable[i+1], "exe"))
                return ___exit(-1);
    }
    // GETTING ADDRESSES
    buff = malloc( sizeof(char)*0xFF );
    if (fscanf(f, "%s", buff) != 1)
        return ___exit(-1);
    if (strcmp(buff, "ADDRESSES"))
        return ___exit(-1);
    addrs = malloc( sizeof(DWORD)*0xFF );
    char addrs_cnt = 0;
    for (int i = 0; i < 0xFF; i++)
    {
        memset(buff, 0, sizeof(char)*0xFF);
        if (fscanf(f, "%s", buff) != 1)
            break;
        if (sscanf(buff, "0x%x", &addrs[i]) != 1)
            break;
        addrs_cnt++;
    }
    // GETTING PATCHES
    if (strcmp(buff, "PATCHES"))
        return ___exit(-1);
    ptchs = malloc( sizeof(DWORD)*0xFF*2*2 );
    char ptchs_cnt = 0;
    for (int i = 0, j = 0; i < 0xFF; i++)
    {
        memset(buff, 0, sizeof(char)*0xFF);
        if (fscanf(f, "%s", buff) != 1)
            break;
        if (!sscanf(buff, "%x,%x", &ptchs[i*2], &ptchs[i*2+1]))
            break;
        ptchs_cnt++;
    }
    if ((addrs_cnt-ptchs_cnt))
        return ___exit(-1);

    PROCESS_INFORMATION pi;
    memset(&pi, 0, sizeof( pi ));
    STARTUPINFO si;
    memset(&si, 0, sizeof( si ));
    if (!CreateProcess(executable, NULL, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi))
        return ___exit(-1);

    DWORD_PTR base_addr = 0;
    while (!(base_addr = get_base_addr(&pi)))
    {
        printf("waiting for process to be launched...\n");
    }

    while (get_base_addr(&pi))
    {
        for (int i = 0; i < addrs_cnt; i++)
        {
            DWORD* buff = malloc( sizeof(ptchs[i*2]) );
            
            if (!read_proc_mem(&pi, (BYTE*)addrs[i], buff))
                return ___exit(-1);
            if (*buff == ptchs[i*2])
                if (write_proc_mem(&pi, (BYTE*)addrs[i], &ptchs[i*2+1]))
                    printf("0x%x\n|  0x%x -> 0x%x\n", addrs[i], ptchs[i*2], ptchs[i*2+1]);
            else
                printf("value of the mem is =>0x%x\n", *buff);
        }
    }

    //TerminateProcess(pi.hProcess, 0);
    ___exit(0);
}