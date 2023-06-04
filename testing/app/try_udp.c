/*
	Simple udp client
*/
#include <stdio.h>	//printf
#include <string.h> //memset
#include <stdlib.h> //exit(0);
#include <time.h>

#include <arpa/inet.h>
#include <sys/socket.h>

#include <unistd.h>

#define SERVER      "192.168.1.167" // led-living
#define BUFLEN      1024             // Max length of buffer
#define PORT        42424           // The port on which to send data


void die(char *s)
{
	perror(s);
	exit(1);
}


int main (void)
{
	struct sockaddr_in si_other;
	int s, i, slen=sizeof(si_other);
	char buf[BUFLEN];
	char message[BUFLEN];

    srand((unsigned) clock());
    
	if ( (s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
	{
		die("socket");
	}

	memset((char *) &si_other, 0, sizeof(si_other));
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(PORT);
	
	if (inet_aton(SERVER , &si_other.sin_addr) == 0) 
	{
		fprintf(stderr, "inet_aton() failed\n");
		exit(1);
	}
	
    struct timespec ts;

    ts.tv_sec  = 0;
    ts.tv_nsec = 15 * 1000000;


	while(1)
	{
        
        memset (message, 0xFC, sizeof(memset));
        
        /* 312 * 3 = 936 */
        message[0] = 0x10;
        message[1] = 0;
        message[2] = 0;
        message[3] = 3;
        message[4] = 168;
		
		//send the message
		if (sendto(s, message, 5 + 936, 0, (struct sockaddr *)&si_other, slen)==-1)
		{
			die("sendto()");
		}
		
		break;

#if 0
        message[0] = 0x01;
        message[1] = 0;
        message[2] = 16;
        for (uint32_t i = 0; i < 16; ++i)
        {
            message[3 + i] = 0xCC; //rand() % 256;
        }
		
		//send the message
		if (sendto(s, message, 3 + 16, 0, (struct sockaddr *)&si_other, slen)==-1)
		{
			die("sendto()");
		}
#endif
		
#if 0
		//receive a reply and print it
		//clear the buffer by filling null, it might have previously received data
		memset(buf,'\0', BUFLEN);
		//try to receive some data, this is a blocking call
		if (recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen) == -1)
		{
			die("recvfrom()");
		}
		
		puts(buf);
#endif

        nanosleep (&ts, NULL);
        //usleep (100 * 1000);
	}

	close(s);
	return 0;
}
