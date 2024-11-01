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
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    char cmd[100];
    OVERLAPPED osReader = {0};
    OVERLAPPED osWriter = {0};
    BOOL got_ok = FALSE;
    char line_buffer[MAX_RESPONSE] = {0};
    int line_pos = 0;

    // Create event handles for overlapped I/O
    osReader.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    osWriter.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    // Send command
    sprintf(cmd, "%s\r\n", command);
    get_timestamp(timestamp);
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
    printf("%s_S:] %s\n", timestamp, command);

    // Write with overlapped I/O
    WriteFile(hSerial, cmd, strlen(cmd), &bytes_written, &osWriter);
    WaitForSingleObject(osWriter.hEvent, 100);

    SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_INTENSITY);

    // Read response with smaller buffer and shorter timeouts
    char small_buffer[16];
    DWORD start_time = GetTickCount();

    while (GetTickCount() - start_time < 1000) // 1 second timeout total
    {
        memset(small_buffer, 0, sizeof(small_buffer));
        ReadFile(hSerial, small_buffer, sizeof(small_buffer) - 1, &bytes_read, &osReader);

        if (WaitForSingleObject(osReader.hEvent, 100) == WAIT_OBJECT_0)
        {
            DWORD bytes_transferred;
            if (GetOverlappedResult(hSerial, &osReader, &bytes_transferred, FALSE) && bytes_transferred > 0)
            {
                // Process each character
                for (DWORD i = 0; i < bytes_transferred; i++)
                {
                    char c = small_buffer[i];
                    if (c == '\r' || c == '\n')
                    {
                        if (line_pos > 0)
                        {
                            line_buffer[line_pos] = '\0';
                            get_timestamp(timestamp);
                            printf("%s_R:] %s\n", timestamp, line_buffer);

                            if (strcmp(line_buffer, "OK") == 0)
                            {
                                got_ok = TRUE;
                            }

                            line_pos = 0;
                        }
                    }
                    else
                    {
                        line_buffer[line_pos++] = c;
                    }
                }

                // Reset event for next read
                ResetEvent(osReader.hEvent);
            }
        }

        // Only break after we've gotten OK and processed any remaining data
        if (got_ok && WaitForSingleObject(osReader.hEvent, 50) == WAIT_TIMEOUT)
        {
            break;
        }
    }

    // Cleanup
    CloseHandle(osReader.hEvent);
    CloseHandle(osWriter.hEvent);
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
}

int main()
{

    HANDLE hSerial = CreateFile("\\\\.\\COM13",
                                GENERIC_READ | GENERIC_WRITE,
                                0,
                                0,
                                OPEN_EXISTING,
                                FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, // Added overlapped flag
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
    // Sleep(50);

    send_command(hSerial, "AT+CEREG?");
    // Sleep(50);

    send_command(hSerial, "AT+CGREG?");
    // Sleep(50);

    send_command(hSerial, "AT+COPS?");

    CloseHandle(hSerial);
    return 0;
}
