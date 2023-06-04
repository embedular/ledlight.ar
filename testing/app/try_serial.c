#define TERMINAL    "/dev/ttyUSB1"
#define DISPLAY_STRING

#include <errno.h>
#include <fcntl.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <stdint.h>


int set_interface_attribs(int fd, int speed)
{
    struct termios tty;

    if (tcgetattr(fd, &tty) < 0) {
        printf("Error from tcgetattr: %s\n", strerror(errno));
        return -1;
    }

    cfsetospeed(&tty, (speed_t)speed);
    cfsetispeed(&tty, (speed_t)speed);

    tty.c_cflag |= (CLOCAL | CREAD);    /* ignore modem controls */
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;         /* 8-bit characters */
    tty.c_cflag &= ~PARENB;     /* no parity bit */
    tty.c_cflag &= ~CSTOPB;     /* only need 1 stop bit */
    tty.c_cflag &= ~CRTSCTS;    /* no hardware flowcontrol */

    /* setup for non-canonical mode */
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    tty.c_oflag &= ~OPOST;

    /* fetch bytes as they become available */
    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = 0;

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        printf("Error from tcsetattr: %s\n", strerror(errno));
        return -1;
    }
    return 0;
}

void set_mincount(int fd, int mcount)
{
    struct termios tty;

    if (tcgetattr(fd, &tty) < 0) {
        printf("Error tcgetattr: %s\n", strerror(errno));
        return;
    }

    tty.c_cc[VMIN] = mcount ? 1 : 0;
    tty.c_cc[VTIME] = 5;        /* half second timer */

    if (tcsetattr(fd, TCSANOW, &tty) < 0)
        printf("Error tcsetattr: %s\n", strerror(errno));
}


int main()
{
    char *portname = TERMINAL;
    int fd;
    int wlen;
    char *xstr = "Hello!\n";
    int xlen = strlen(xstr);

    fd = open(portname, O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0) {
        printf("Error opening %s: %s\n", portname, strerror(errno));
        return -1;
    }
    /*baudrate 115200, 8 bits, no parity, 1 stop bit */
    set_interface_attribs(fd, B115200);
    //set_mincount(fd, 0);                /* set to pure timed read */

    struct timespec ts;
    ts.tv_sec  = 0;
    ts.tv_nsec = 0; 500 * 1000000;

    
    do 
    {
        unsigned char buf[256];
    
        buf[0] = 0x01;
        buf[1] = 0;
        buf[2] = 16;
        for (uint32_t i = 0; i < 16; ++i)
        {
            buf[3 + i] = rand() % 256;
        }
        
    /* simple noncanonical input */
            /* simple output */
        wlen = write(fd, buf, 3 + 16);
        if (wlen != 3 + 16) {
            printf("Error from write: %d, %d\n", wlen, errno);
        }
        //tcdrain(fd);    /* delay for output */

        fprintf (stderr, ".");

        #if 0
        int rdlen;

        rdlen = read(fd, buf, sizeof(buf) - 1);
        if (rdlen > 0) {
#ifdef DISPLAY_STRING
            buf[rdlen] = 0;
            //printf("Read %d: \"%s\"\n", rdlen, buf);
            printf("%s", buf);

#else /* display hex */
            unsigned char   *p;
            printf("Read %d:", rdlen);
            for (p = buf; rdlen-- > 0; p++)
                printf(" 0x%x", *p);
            printf("\n");
#endif
        } else if (rdlen < 0) {
            printf("Error from read: %d: %s\n", rdlen, strerror(errno));
        } else {  /* rdlen == 0 */
            printf("Timeout from read\n");
        }               
        /* repeat read to get full message */
#endif        
        // nanosleep (&ts, NULL);
        usleep (16 * 1000);
                
    } while (1);
}


#if 0
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
#define BUFLEN      512             // Max length of buffer
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
    ts.tv_nsec = 0; 500 * 1000000;


	while(1)
	{
        message[0] = 0x01;
        message[1] = 0;
        message[2] = 16;
        for (uint32_t i = 0; i < 16; ++i)
        {
            message[3 + i] = rand() % 256;
        }
		
		//send the message
		if (sendto(s, message, 3 + 16, 0, (struct sockaddr *)&si_other, slen)==-1)
		{
			die("sendto()");
		}
		
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

        // nanosleep (&ts, NULL);
        usleep (900 * 1000);
	}

	close(s);
	return 0;
}

#endif
