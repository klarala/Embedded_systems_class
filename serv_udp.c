/* Nepovezaven UDP streznik */ 
#include <stdio.h> 
#include <stdlib.h>
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h> 

#define BUFF_LEN    4096 
#define SERVPORTNO 50000 

int main( void ) 
{ 
  // priprava spremenljivk za delo s socketom
  int sockfd,n; 
  struct sockaddr_in servaddr,cliaddr; 
  socklen_t len; 

  // odpremo fifo datoteko za branje
  int fd;
  char* filename = "video_temp";
  fd = open(filename, O_RDONLY);

  // pripravimo pomnilnik, kamor bomo pisali prebrane podatke
  char* buff;
  buff = (char*) malloc(BUFF_LEN);

  // inicializiramo socket
  // AF_INET - uporabljamo IPv4 protokol
  // SOCK_DGRAM - uporabljamo UDP
  if( (sockfd=socket(AF_INET,SOCK_DGRAM,0)) == -1){
    perror("socket err");
    exit(1);
  }

  // inicializiramo servaddr, kjer sta shranjena IP in port
  bzero(&servaddr,sizeof(servaddr)); 
  servaddr.sin_family = AF_INET; 
  servaddr.sin_addr.s_addr=htonl(INADDR_ANY); 
  servaddr.sin_port=htons( SERVPORTNO ); 

  // servaddr vežemo na naš socket 
  if( bind(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr)) == -1){
    perror("bind err"); 
    exit(1);
  }

  // Da lahko pošljemo podatke našemu clientu, moramo poznati ip in port našega clienta
  // To lahko izvemo tako, da client pošlje en paket serverju
  // recvfrom blokira nadalnje izvajanje programa
  len = sizeof(cliaddr); 
  n = recvfrom(sockfd,buff,BUFF_LEN,0,(struct sockaddr *)&cliaddr,&len);

  printf("Zacetek zanke\n");
  while( 1 ){  
    // v neskončni zanki beremo podatke iz fifo dototeke, kjer raspivid shranjuje video
    while( (n = read( fd, buff, BUFF_LEN)) > 0 ){
    // Podatke pošljemo clientu
      if( sendto(sockfd,buff,n,0,(struct sockaddr *)&cliaddr,sizeof(cliaddr)) == -1)
        perror("write err"); 
    } 
  } 
return 0;
}