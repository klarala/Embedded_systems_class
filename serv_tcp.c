/* Povezaven - TCP streznik */
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

// KLARA JE ZAKON

int main( int argc, char **argv )
{
  // preverimo, če je uporabnik vpisal dovolj argumentov
  if (argc != 2) {
    printf("The correct use is: %s ip_naprave\n", argv[0]);
    exit( 1 );
  }

  // pripravimo spremenljivke za delo s socketom
  struct sockaddr_in cliAddr;
  socklen_t size;
  int       initServer( char * );
  int       sd, sn, n;

  // pripravimo pomnilnik, kamor bomo pisali prebrane podatke
  char* buff;
  buff = (char*) malloc(BUFF_LEN);

  // priprava na branje iz fifo datoteke
  int fd;
  char* filename = "video_temp";
  fd = open(filename, O_RDONLY);

  // inicializiramo server
  if( (sd = initServer( argv[1] )) < 0){
    printf("Napaka: init server\n"); exit( 1 );
  }

  // poslušamo, če se želi client povezati na socket
  listen( sd, 5 );
  alarm( 60 ); /* koncaj po eni minuti */

  while( 1 ){
    size = sizeof(cliAddr);
    memset( &cliAddr, 0, size );
    
    // sprejmemo povezavo na socket
    if( (sn = accept(sd, (struct sockaddr *)&cliAddr, &size)) < 0){
      perror("accept err"); exit( 2 ); 
    }
    /* zveza je vzpostavljena, ustvari strezni proces */
    if( fork() == 0 ){
      printf("odjemalec: %s:%d\n", inet_ntoa( cliAddr.sin_addr ), ntohs( cliAddr.sin_port ) );

      // preberemo podatke iz fifo datoteke in jih zapišemo v socket
      while( (n = read( fd, buff, sizeof( buff ))) > 0 ){
        if( write(sn, buff, n) == -1)
          perror("write err");
      }
      printf("odjemalec: %s:%d je prekinil povezavo\n", inet_ntoa( cliAddr.sin_addr ), ntohs( cliAddr.sin_port ));
      close( sn );
      exit( 0 );
    }
  }
}

int initServer( char *hostName )
{
  // priprava spremenljivk za delo s socketom
  struct sockaddr_in  servAddr; 
  struct hostent     *host;
  int    sd;

  // z gethostbyname inicializiramo hostent, ki ga rabimo, da pripravimo servAddr
  // v našem primeru smo uporabili namesto hostname ipv4 naslov serverja
  if( (host = gethostbyname( hostName )) == NULL) return( -1 );
  memset( &servAddr, 0, sizeof(servAddr) );
  memcpy( &servAddr.sin_addr, host->h_addr, host->h_length );
  servAddr.sin_family  = host->h_addrtype;
  servAddr.sin_port    = htons( PORT_NUMBER );

  printf("streznik: %s, ", host -> h_name);
  printf("%s:%d\n", inet_ntoa(servAddr.sin_addr), PORT_NUMBER );

  // inicializiramo socket
  // AF_INET - uporabimo IPv4 protokol
  // SOCK_STREAM - uporabljamo TCP
  if( (sd = socket(AF_INET, SOCK_STREAM,0)) < 0 ) return( -2 );

  // inicializiramo povezavo na socketu s serverjem
  if( bind(sd, (struct sockaddr *)&servAddr, sizeof( servAddr )) < 0) return( -3 );
  return( sd );
}