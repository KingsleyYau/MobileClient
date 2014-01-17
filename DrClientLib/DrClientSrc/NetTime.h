/*
 * File         : NetTime.h
 * Data         : 2010-11-02
 * Author       : Keqin Su
 * Copyright    : City Hotspot Co., Ltd.
 * Description  : ntp implement
 */
    
#include <stdlib.h>    
#include <time.h>
#include <stdio.h>
#include <sys/timeb.h>
#include <ctype.h>
#include "DrClientInclude.h"
#include "DrCOMSocket.h"

#if defined(_WIN32) || defined(_WIN32_WCE) || defined(_WIN32_WP8)
	#include <winsock2.h>
	#include <windows.h>
    #include <conio.h>
#else
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <sys/time.h>
	#include <unistd.h>
	#include <signal.h>
	#include <fcntl.h>
	#include <netdb.h>
	#include <errno.h>
#endif

#ifndef _INC_NETTIME_
#define _INC_NETTIME_

#define NTP_PORT      123

const char Days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};   
   
#define JAN_1970      0x83aa7e80      /* 2208988800 1970 - 1900 in seconds */    
   
/* How to multiply by 4294.967296 quickly (and not quite exactly)  
* without using floating point or greater than 32-bit integers.  
* If you want to fix the last 12 microseconds of error, add in  
* (2911*(x))>>28)  
*/   
#define NTPFRAC(x) (4294 * (x) + ((1981 * (x))>>11))    
   
/* The reverse of the above, needed if we want to set our microsecond  
 * clock (via settimeofday) based on the incoming time in NTP format.  
 * Basically exact.  
 */   
#define USEC(x) (((x) >> 12) - 759 * ((((x) >> 10) + 32768) >> 16))    

struct ntptime    
{   
    unsigned int coarse;   
    unsigned int fine;   
};

class NetTime
{
protected:
    void stdcalllocaltimes(time_t time,struct tm *tm_time){ 
        GMTIMEFUNC(&time, tm_time);
    }

    bool send_packet(udpSocket* pudpNet) {   
        unsigned int data[12];   
        /*struct timeval now;*/
        struct timeb now;
        int ret;
        #define LI 0    
        #define VN 3    
        #define MODE 3    
        #define STRATUM 0    
        #define POLL 4     
        #define PREC -6    
   
        if (sizeof(data) != 48){   
            return false;   
        }   
           
        memset((char*)data, 0, sizeof(data));   
        data[0] = htonl((LI << 30) | (VN << 27) | (MODE << 24) | (STRATUM << 16) | (POLL << 8) | (PREC & 0xff));   
        data[1] = htonl(1<<16);  /* Root Delay (seconds) */   
        data[2] = htonl(1<<16);  /* Root Dispersion (seconds) */   
        ftime(&now);   
        /*gettimeofday(&now, NULL);*/   
        data[10] = htonl(now.time + JAN_1970); /* Transmit Timestamp coarse */   
        data[11] = htonl(NTPFRAC(now.millitm));  /* Transmit Timestamp fine   */   
        ret = pudpNet->SendData((char*)data, 48);   
        return (ret > 0);
    }   
   
    void get_packet_timestamp(struct ntptime *udp_arrival_ntp){
        /*struct timeval udp_arrival;*/
        struct timeb udp_arrival;
        
        ftime(&udp_arrival);   
        /*gettimeofday(&udp_arrival, NULL);*/   
        udp_arrival_ntp->coarse = udp_arrival.time;
        udp_arrival_ntp->fine   = NTPFRAC(udp_arrival.millitm);
    }   
   
    void rfc1305print(unsigned int *data, struct ntptime *arrival, unsigned int* sec)   
    {   
    /* straight out of RFC-1305 Appendix A */   
        int li, vn, mode, stratum, poll, prec;   
        int delay, disp, refid;   
        struct ntptime reftime, orgtime, rectime, xmttime;   
        struct tm timevalue;   
        struct tm *ltm = &timevalue;   
           
           
    #define Data(i) ntohl(((unsigned int *)data)[i])    
        li      = Data(0) >> 30 & 0x03;   
        vn      = Data(0) >> 27 & 0x07;   
        mode    = Data(0) >> 24 & 0x07;   
        stratum = Data(0) >> 16 & 0xff;   
        poll    = Data(0) >>  8 & 0xff;   
        prec    = Data(0)       & 0xff;   
        if (prec & 0x80) prec|=0xffffff00;   
        delay   = Data(1);   
        disp    = Data(2);   
        refid   = Data(3);   
        reftime.coarse = Data(4);   
        reftime.fine   = Data(5);   
        orgtime.coarse = Data(6);   
        orgtime.fine   = Data(7);   
        rectime.coarse = Data(8);   
        rectime.fine   = Data(9);   
        xmttime.coarse = Data(10);   
        xmttime.fine   = Data(11);   
    #undef Data    
           
        *sec = xmttime.coarse - JAN_1970;  
        //stdcalllocaltimes(tv->tv_sec, ltm);   
    }

public:
    bool GetNetTime(const char* lpServer, unsigned uiPort, unsigned int& uiTime) {
        udpSocket udpNet;
        udpNet.Close();
		if (udpNet.Connect(lpServer, uiPort) > 0) {
            if (send_packet(&udpNet)) {
                unsigned int buf[12]; 
                struct ntptime arrival_ntp;   
                //struct timeval newtime;
                if (udpNet.RecvData((char *)buf, sizeof(buf)) == sizeof(buf)){
                    /* get local timestamp. */   
                    //get_packet_timestamp(&arrival_ntp);   
                    /* get server's time and print it. */   
                    rfc1305print(buf, &arrival_ntp, &uiTime);
                    return true;
                }
            }
        }
        return false;
    }
};
#endif