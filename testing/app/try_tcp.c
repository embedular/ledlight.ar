#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <netdb.h>
#include <netinet/in.h>

#include <string.h>

#include <time.h>
 


int main(int argc, char *argv[]) {
   int sockfd, portno, n;
   struct sockaddr_in serv_addr;
   struct hostent *server;
   
   char buffer[256];
   
   if (argc < 3) {
      fprintf(stderr,"usage %s hostname port\n", argv[0]);
      exit(0);
   }
	
   portno = atoi(argv[2]);
   
   /* Create a socket point */
   sockfd = socket(AF_INET, SOCK_STREAM, 0);
   
   if (sockfd < 0) {
      perror("ERROR opening socket");
      exit(1);
   }
	
   server = gethostbyname(argv[1]);
   
   if (server == NULL) {
      fprintf(stderr,"ERROR, no such host\n");
      exit(0);
   }
   
   bzero((char *) &serv_addr, sizeof(serv_addr));
   serv_addr.sin_family = AF_INET;
   bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
   serv_addr.sin_port = htons(portno);
   
   /* Now connect to the server */
   if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
      perror("ERROR connecting");
      exit(1);
   }
   
   srand((unsigned) clock());
 
   
   struct timespec ts;
   
   ts.tv_sec  = 0;
   ts.tv_nsec = 100 * 1000000;

   
   uint8_t l[16];
   for (uint32_t i = 0; i < 16; ++i)
   {
        l[i] = rand() % 0xFF;
   }
   
   
   while (1)
   {    
        buffer[0] = 0x02;
        buffer[1] = 0x00;
        buffer[2] = 0x10;
        
        for (uint32_t i = 0; i < 16; ++i)
        {
            buffer[3 + i] = l[i]; //rand () % 0xFF;
            l[i] += 10; 
        }
        
        buffer[3 + 15] = 0x00;
        
        n = write (sockfd, buffer, 3 + 16);
   
        if (n < 0)
        {
            perror("ERROR writing to socket");
            exit(1);
        }  
    
        //936  312 
       /* 
        // Pixels 0 to 63
        buffer[0] = 0x10;
        buffer[1] = 0;
        buffer[2] = 64;
        
        for (uint32_t i = 0; i < 64 * 3; ++i)
        {
            buffer[3 + i] = rand () % 0xA0;
        }
        
        n = write (sockfd, buffer, 3 + 64 * 3);
   
        if (n < 0)
        {
            perror("ERROR writing to socket");
            exit(1);
        }*/
#if 0      
        // Pixels 64 to 127
        buffer[0] = 0x10;
        buffer[1] = 4;
        buffer[2] = 64;
        
        for (uint32_t i = 0; i < 64 * 3; ++i)
        {
            buffer[3 + i] = rand () % 0xA0;
        }
        
        n = write (sockfd, buffer, 3 + 64 * 3);
   
        if (n < 0)
        {
            perror("ERROR writing to socket");
            exit(1);
        }
        
        // Pixels 128 to 191
        buffer[0] = 0x10;
        buffer[1] = 8;
        buffer[2] = 64;
        
        for (uint32_t i = 0; i < 64 * 3; ++i)
        {
            buffer[3 + i] = rand () % 0xA0;
        }
        
        n = write (sockfd, buffer, 3 + 64 * 3);
   
        if (n < 0)
        {
            perror("ERROR writing to socket");
            exit(1);
        }
        
        // Pixels 192 to 255
        buffer[0] = 0x10;
        buffer[1] = 12;
        buffer[2] = 64;
        
        for (uint32_t i = 0; i < 64 * 3; ++i)
        {
            buffer[3 + i] = rand () % 0xA0;
        }
        
        n = write (sockfd, buffer, 3 + 64 * 3);
   
        if (n < 0)
        {
            perror("ERROR writing to socket");
            exit(1);
        }
        
        // Pixels 256 to 311
        buffer[0] = 0x10;
        buffer[1] = 16;
        buffer[2] = 56;
        
        for (uint32_t i = 0; i < 56 * 3; ++i)
        {
            buffer[3 + i] = rand () % 0xA0;
        }
        
        n = write (sockfd, buffer, 3 + 56 * 3);
   
        if (n < 0)
        {
            perror("ERROR writing to socket");
            exit(1);
        }
#endif
        nanosleep (&ts, NULL);
   }
   
/*
   // Now read server response
   bzero(buffer,256);
   n = read(sockfd, buffer, 255);
   
   if (n < 0) {
      perror("ERROR reading from socket");
      exit(1);
   }
	
   printf("%s\n",buffer);
*/
   
   return 0;
}
