#include <Windows.h>

#define PROGRAM_NAME "ExecutionSelector"
#define MAILSLOT_FILE_NAME "\\\\.\\mailslot\\ExecutionSelector"
#define MAILSLOT_TIMEOUT 300

int main() {
    //用Mutex判断是否第一次执行
    HANDLE hSingleProMutex = CreateMutexA(NULL, TRUE, PROGRAM_NAME);
    HANDLE hMailslot;

    int mailData[1];
    DWORD mailDataLen;

    if (hSingleProMutex && GetLastError() == ERROR_ALREADY_EXISTS) {
        //第二个进程运行, 发mail给第一个进程
        hMailslot = CreateFileA(MAILSLOT_FILE_NAME, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                                FILE_ATTRIBUTE_NORMAL, NULL);
        if (hMailslot == INVALID_HANDLE_VALUE) {
            return 1;
        }
        mailData[0] = 1;  //随便写一个值用于判断
        if (!WriteFile(hMailslot, mailData, sizeof(mailData), &mailDataLen, NULL)) {
            CloseHandle(hMailslot);
            return 1;
        }
    } else {
        //第一个进程运行, 超时内等待第二个进程发mail
        hMailslot = CreateMailslotA(MAILSLOT_FILE_NAME, NULL, MAILSLOT_TIMEOUT, NULL);
        if (hMailslot == INVALID_HANDLE_VALUE) {
            return 1;
        }
        //超时, 没有运行第二次
        if (!ReadFile(hMailslot, mailData, sizeof(mailData), &mailDataLen, NULL)) {
            CloseHandle(hMailslot);
            ShellExecuteA(NULL, "open", "notepad", NULL, NULL, SW_SHOW);
            return 0;
        }
        //运行第二次
        if (mailDataLen > 0 && mailData[0] == 1) {  //与上面随便写的值相同
            ShellExecuteA(NULL, "open", "calc", NULL, NULL, SW_SHOW);
        }
    }

    CloseHandle(hMailslot);
    return 0;
}
