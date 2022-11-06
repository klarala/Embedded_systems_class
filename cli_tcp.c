/* Povezaven - TCP odjemalec */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <fcntl.h>

#define PORT_NUMBER 55000
#define BUFF_LEN 1000000


int main( int argc, char **argv )
{ 

  // preverimo, če je uporabnik vpisal dovolj argumentov
  if (argc != 2) {
    printf("The correct use is: %s ip_streznika\n", argv[0]);
    exit( 1 );
  }

  // priprava spremenljivk za delo s socketom
  int    n, sd, initClient( char * );

  // pripravimo pomnilnik, kamor bomo shranjevali prejete podatke
  char* buff = (char*) malloc(BUFF_LEN);

  // odprimo datoteko, kamor bomo shranjevali prejete podatke
  int fd;
  char* filename = "video_temp";
  fd = open(filename, O_WRONLY);  

  // inicializiramo socket
  if( (sd = initClient( argv[1] )) < 0 ){
    printf("napaka init\n");  exit( 1 );
  }
  /*
  else{
    printf("tipkaj karkoli, ^D za konec\n");
    while( fgets(&buf[4], sizeof(buf), stdin ) != NULL ){
      memcpy(buf,"c-> ",4);
      printf("%s", buf); 
      if( write(sd, buf, strlen(buf)) == -1)
        perror("write err");
      if( (n = read(sd, buf, sizeof(buf))) == -1)
        perror("read err");
      buf[n] = 0;
      printf("%s", buf);
    }
    close( sd );
  }
  */

  // neskoncna zanka, kjer preberemo podatke in jih zapišemo v fifo datoteko
  while( (n = read( sd, buff, sizeof( buff ))) > 0 ){
    if( write(fd, buff, n) == -1)
      perror("write err");
  }
  close( sd );
  exit( 0 );
}
  

int initClient( char *hostName )
{
  // pripravimo spremenljivke za delo s socketom
  struct sockaddr_in  servAddr;
  struct hostent     *host;
  int    sd;
  
  // z gethostbyname inicializiramo hostent, ki ga rabimo, da pripravimo servAddr
  // v našem primeru smo uporabili namesto hostname ipv4 naslov serverja
  if( ( host = gethostbyname( hostName )) == NULL) return( -1 );
  memset( &servAddr, 0, sizeof(servAddr));
  memcpy( &servAddr.sin_addr, host->h_addr, host->h_length );
  servAddr.sin_family = host->h_addrtype;
  servAddr.sin_port   = htons( PORT_NUMBER );

  printf("streznik: %s, ", host -> h_name);
  printf("%s:%d\n", inet_ntoa( servAddr.sin_addr ), PORT_NUMBER);

  // inicializiramo socket
  // AF_INET - uporabimo IPv4 protokol
  // SOCK_STREAM - uporabljamo TCP
  if( (sd = socket(AF_INET,SOCK_STREAM,0)) < 0 ) return( -2 );

  // inicializiramo povezavo na socketu s serverjem
  if( connect(sd, (struct sockaddr *)&servAddr,sizeof(servAddr)) < 0) return( -3 );
  return( sd );
}