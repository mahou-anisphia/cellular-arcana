#include <windows.h>
#include <stdio.h>
#include <time.h>

#define MAX_RESPONSE 1024

void get_timestamp(char *timestamp)
{
    SYSTEMTIME st;
    GetLocalTime(&st);
    sprintf(timestamp, "[%04d-%02d-%02d %02d:%02d:%02d:%03d",
            st.wYear, st.wMonth, st.wDay,
            st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
}

void send_command(HANDLE hSerial, const char *command)
{
    char timestamp[32];
    char response[MAX_RESPONSE];
    DWORD bytes_written, bytes_read;
    char cmd[100];

    sprintf(cmd, "%s\r\n", command);
    get_timestamp(timestamp);
    printf("%s_S:] %s", timestamp, command);
    printf("\n");

    WriteFile(hSerial, cmd, strlen(cmd), &bytes_written, NULL);
    Sleep(100);

    memset(response, 0, MAX_RESPONSE);
    while (1)
    {
        if (!ReadFile(hSerial, response, MAX_RESPONSE - 1, &bytes_read, NULL) || bytes_read == 0)
        {
            break;
        }

        if (bytes_read > 0)
        {
            char *line = strtok(response, "\r\n");
            while (line)
            {
                if (strlen(line) > 0)
                {
                    get_timestamp(timestamp);
                    printf("%s_R:] %s\n", timestamp, line);
                }
                line = strtok(NULL, "\r\n");
            }

            if (strstr(response, "OK") || strstr(response, "ERROR"))
            {
                break;
            }
        }
    }
}

int main()
{

    HANDLE hSerial = CreateFile("\\\\.\\COM13",
                                GENERIC_READ | GENERIC_WRITE,
                                0,
                                0,
                                OPEN_EXISTING,
                                FILE_ATTRIBUTE_NORMAL,
                                0);

    if (hSerial == INVALID_HANDLE_VALUE)
    {
        printf("Error opening COM port. Check port number!\n");
        return 1;
    }

    DCB dcbSerialParams = {0};
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    GetCommState(hSerial, &dcbSerialParams);
    dcbSerialParams.BaudRate = CBR_115200;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;
    SetCommState(hSerial, &dcbSerialParams);

    COMMTIMEOUTS timeouts = {0};
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    SetCommTimeouts(hSerial, &timeouts);

    printf("Checking BG95 Network Status...\n\n");

    send_command(hSerial, "AT+CSQ");
    Sleep(50);

    send_command(hSerial, "AT+CEREG?");
    Sleep(50);

    send_command(hSerial, "AT+CGREG?");
    Sleep(50);

    send_command(hSerial, "AT+COPS?");

    CloseHandle(hSerial);
    return 0;
}
