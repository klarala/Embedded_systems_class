/* Nepovezaven UDP odjemalec */ 
#include <stdio.h> 
#include <stdlib.h>
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h> 


#define BUFF_LEN 4096 
#define SERVPORTNO 50000 

int main(int argc, char **argv) 
{ 

  // preverimo, če je uporabnik vpisal IP
  if (argc != 2){ 
    printf("usage: %s <IP address>\n", argv[0]); 
    exit(1); 
  } 

  // priprava spremenljivk za delo s socketom
  int sockfd,n; 
  struct sockaddr_in servaddr; 

  // pripravimo pomnilnik, kamor bomo shranjevali prejete podatke
  char* buff;
  buff = (char*) malloc(BUFF_LEN);

  // odpremo fifo datoteko
  int fd;
  char* filename = "video_temp";
  fd = open(filename, O_WRONLY);

  // inicializiramo socket
  if( (sockfd=socket(AF_INET,SOCK_DGRAM,0)) == -1){
    perror("socket err");
    exit(1);
  } 

  // inicializiramo servaddr
  bzero(&servaddr,sizeof(servaddr)); 
  servaddr.sin_family = AF_INET; 
  servaddr.sin_addr.s_addr=inet_addr(argv[1]); 
  servaddr.sin_port=htons(SERVPORTNO); 

  // serverju pošljemo en paket, da pridobi naš IP
  n = sendto(sockfd,buff,strlen(buff),0,(struct sockaddr *)&servaddr,sizeof(servaddr));
  if( n == -1 ){
    perror("sento err");
    exit(1);
  }

  while (1){
    // od serverja prejmemo paket in ga zapišemo v fifo datoteko
    n=recvfrom(sockfd,buff,BUFF_LEN,0,NULL,NULL);
    if( n == -1 ){
      perror("recvfrom err");
    }
    if( write(fd, buff, n) == -1)  perror("write err");
    }
  exit( 0 );
} 
