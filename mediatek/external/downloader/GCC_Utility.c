#include "GCC_Utility.h"
#if defined(__GNUC__)


#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#if defined (__linux__)
#include <time.h>
#endif


#define _tcsrchr strrchr

DWORD GetCurrentProcessId()
{
    return (DWORD)getpid();;
}

DWORD GetCurrentThreadId()
{
    return 0;
}

void GetLocalTime(PSYSTEMTIME p_systime)
{
    struct timeval curTime;
    struct tm *p;

    gettimeofday(&curTime, NULL);
    p=localtime(&curTime.tv_sec);

    p_systime->wYear = 1900+p->tm_year;
    p_systime->wDayOfWeek = p->tm_wday;
    p_systime->wMonth = 1+p->tm_mon;
    p_systime->wDay = p->tm_mday;
    p_systime->wHour = p->tm_hour;
    p_systime->wMinute = p->tm_min;
    p_systime->wSecond = p->tm_sec;
    p_systime->wMilliseconds = curTime.tv_usec / 1000;

}

void OutputDebugString(const char* c)
{

}

FILE *_fsopen (const char *fname, const char *mode, int shflag)
{
    //without setting share mode flag
    FILE *f;
    f = fopen(fname, mode);
    return f;
}


bool WriteFile(HANDLE hFile, LPCVOID buffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, void* dummyforAPI)
{
    int written_len = 0;
    //usleep(3000);
    fd_set	writefd;

    struct timeval timeout;
    //Ruoyao: timeout from 500ms to 5s
    //timeout.tv_sec = 0;
    //timeout.tv_usec = 500000;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;

    bool is_written = false;
    while(!is_written)
    {
        FD_ZERO(&writefd);
        FD_SET(hFile, &writefd);
        if( select(hFile+1, NULL , &writefd, NULL, &timeout) > 0)
        {
            int ifd;
            for(ifd=0; ifd < hFile+1; ifd++)
            {
                if (FD_ISSET(ifd, &writefd))
                {
                    if(ifd == hFile)
                    {
                        written_len = write(hFile, buffer, nNumberOfBytesToWrite);
                        /*printf("%d written\n", written_len);
                        {
                            int i = 0;
                            while(i < written_len)
                            {
                                printf("%x ",((const char*)buffer)[i++]);
                            }
                            printf("\n");
                        }*/
                        is_written = true;
                    }
                }
            }
        }
        else
        {
            log_linux_errno("WriteFile.select()");
            break;
        }
    }

    if(written_len >= 0)
    {
        if(lpNumberOfBytesWritten != NULL)
        {
            *lpNumberOfBytesWritten = written_len;
        }
        return true;
    }
    else
    {
        log_linux_errno("WriteFile.write()");
        if(lpNumberOfBytesWritten != NULL)
        {
            *lpNumberOfBytesWritten = 0;
        }
    	return false;
    }
}

bool ReadFile(HANDLE hFile, LPVOID buffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, void* dummyforAPI)
{
    int read_len = 0;

    //usleep(3000);
    fd_set	readfd;
    struct timeval timeout;
	//Ruoyao: timeout from 500ms to 5s
    //timeout.tv_sec = 0;
    //timeout.tv_usec = 50000; //50ms
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;

    bool is_read = false;
    while(!is_read)
    {
        FD_ZERO(&readfd);
        FD_SET(hFile, &readfd);
        if( select(hFile+1, &readfd , NULL, NULL, &timeout) > 0)
        {
            int ifd;
            for(ifd=0; ifd < hFile+1; ifd++)
            {
                if (FD_ISSET(ifd, &readfd))
                {
                    if(ifd == hFile)
                    {
                        read_len = read(hFile, buffer, nNumberOfBytesToRead);
                        is_read = true;
                        /*{
                            int i = 0;
                            while(i < read_len)
                            {
                                printf("%x ",((const char*)buffer)[i++]);
                            }
                            printf("\n");
                        }*/
                    }
                }
            }
        }
        else
        {
            log_linux_errno("ReadFile.select()");
            break;
        }
    }

    if(read_len >= 0)
    {
        if(lpNumberOfBytesRead != NULL)
        {
            *lpNumberOfBytesRead = read_len;
        }
        return true;
    }
    else
    {
        log_linux_errno("ReadFile.read()");
        if(lpNumberOfBytesRead != NULL)
        {
            *lpNumberOfBytesRead = 0;
        }
        return false;
	}
}

bool ChangeBaudRate(HANDLE hCOM, DWORD  baudrate) {

    struct termios newtio;
    struct termios oldtio;
    speed_t baudrate_t = B9600;

    tcgetattr(hCOM, &oldtio);
    bzero(&newtio, sizeof(newtio));
    switch(baudrate)
    {
    case 1800:
        baudrate_t = B1800;
        break;
    case 2400:
        baudrate_t = B2400;
        break;
    case 4800:
        baudrate_t = B4800;
        break;
    case 9600:
        baudrate_t = B9600;
        break;
    case 19200:
        baudrate_t = B19200;
        break;
    case 38400:
        baudrate_t = B38400;
        break;
    case 57600:
        baudrate_t = B57600;
        break;
    case 115200:
        baudrate_t = B115200;
        break;
    default:
        break;
    }


    newtio.c_cflag = baudrate_t|CS8|CLOCAL|CREAD;
    //newtio.c_cflag |= baudrate;
    newtio.c_oflag = oldtio.c_oflag;
    newtio.c_iflag = oldtio.c_oflag;
    newtio.c_lflag = oldtio.c_oflag;
    //[TODO] set timeout by default
    newtio.c_cc[VTIME]=5;
    newtio.c_cc[VMIN]=0;

    tcflush(hCOM, TCIFLUSH);
    tcsetattr(hCOM, TCSANOW, &newtio);

	return true;
}

ULONG GetTickCount()
{
    ULONG currentTime;
#if defined(_MSC_VER)
    currentTime = ::GetTickCount();

#elif defined(__GNUC__)
    struct timeval current;
    gettimeofday(&current, NULL);
    currentTime = current.tv_sec * 1000 + current.tv_usec/1000;

#endif

    return currentTime;
}
ULONG GetTickCount2()
{
    ULONG currentTime;
#if defined(_MSC_VER)
    currentTime = ::GetTickCount();

#elif defined(__GNUC__)
    struct timeval current;
    gettimeofday(&current, NULL);
    currentTime = current.tv_sec * 1000000 + current.tv_usec;

#endif

    return currentTime;
}

char *_strupr( char *str )
{
    int str_size = strlen(str);
    int i = 0;
    while (i < str_size)
    {
        str[i] = (char)toupper(str[i]);
        i++;
    }
    return str;
}

#endif

