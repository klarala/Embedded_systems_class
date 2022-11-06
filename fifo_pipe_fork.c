

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#define F_SETPIPE_SZ 1031
#define F_GETPIPE_SZ 1032


int beri_video0(int cev1[2]) {


    int ret = fcntl(cev1[1], F_SETPIPE_SZ, 4194304);
    if (ret < 0) {
        printf("Fifo size cannot be changed\n");
        return 0;
    }

    int video0 = open("/dev/video0", O_RDONLY);

    int bytes = 640 * 480 * 3;

    char* buffer = (char*) malloc(bytes * sizeof(char));

    int rd, wr;

    close(cev1[0]); // zapremo za branje
    int fd = cev1[1];

    int i = 0;

    while(1) {

        rd = read(video0, buffer, bytes);
        wr = write(fd, buffer, bytes); // zapišemo v prvo cev

        printf("prvi %d\n", i);

        lseek(video0, SEEK_SET, 0); 

        i++;

    }

    close(video0);
    close(fd);

    return 0;

}

int predelaj(int cev1[2], int cev2[2]) {

    int ret = fcntl(cev2[1], F_SETPIPE_SZ, 4194304);
    if (ret < 0) {
        printf("Fifo size cannot be changed\n");
        return 0;
    }

    close(cev1[1]); // prvo cev zapremo za pisanje
    int vhod = cev1[0];

    close(cev2[0]); // drugo cev zapremo za branje
    int izhod = cev2[1];

    int bytes_in = 640 * 480 * 3;
    int bytes_out = 1920 * 1080 * 2;

    char* buffer = (char*) malloc(bytes_in * sizeof(char));

    int rd, wr;

    char podatki[bytes_out];

    int i = 0;
    while(1) {

        rd = read(vhod, buffer, bytes_in); // beremo iz prve cevi

        for(int i = 0; i < 1920 * 1080; i++) {

            if ((i < 1920*300) || (i > 1920 * 779)) {

                podatki[2*i] = 0;
                podatki[2*i+1] = 0;
            }
            else if ((i % 1920 < 640) || (i % 1920 > 1279)){

                    podatki[2*i] = 0;
                    podatki[2*i+1] = 0;
            }
            else {

                int stolpec = i % 1920 - 640;
                int vrstica = i / 1920 - 300;
                int polozaj = (vrstica) * 640 + stolpec;

                int r_in = buffer[3*polozaj];
                int g_in = buffer[3*polozaj+1];
                int b_in = buffer[3*polozaj+2];

                int r_new = (float) (r_in / 8);
                int g_new = (float) (g_in / 4);
                int b_new = (float) (b_in / 8);

                u_int8_t byte2 = ((7 & g_new) << 5) + (31 & b_new);
                u_int8_t byte1 = ((31 & r_new) << 3) + ((56 & g_new) >> 3);

                podatki[2*i] = byte2;
                podatki[2*i+1] = byte1;
            }
        }

        wr = write(izhod, podatki, bytes_out); // zapišemo v drugo cev

        printf("drugi %d\n", i);
        i++;

    }

    close(vhod);
    close(izhod);

    return 0;
}


int prikazi(int cev2[2]) {


    close(cev2[1]); // zapremo drugo cev za pisanje
    int fi = cev2[0];

    int bytes = 1920 * 1080 * 2;

    char* buffer = (char*) malloc(bytes * sizeof(char));

    int rd, wr;

    struct timeval cas; // inicializacija spremenljivke za čas
    long start, end, razlika;
    float fps;


    int i = 0;
    while(1) {

        int fo = open("/dev/fb0", O_WRONLY | O_CREAT,0755);

        rd = read(fi, buffer, bytes); // beremo iz druge cevi
        wr = write(fo, buffer, bytes);

        printf("tretji %d\n", i);


        if (i == 0) {

            // začni meriti
            gettimeofday(&cas, NULL);
            start = cas.tv_usec + 1000000 * cas.tv_sec;

        }

        if (i == 200) {
            // končaj meriti
            gettimeofday(&cas, NULL);
            end = cas.tv_usec + 1000000 * cas.tv_sec;

            razlika = end - start;

            fps = (200000000.0 / razlika);

            printf("fps: %.3f\n", fps);
        }

        i++;

        close(fo);

    }

    close(fi);

    return 0;
}

int main(void) {

    
    int cev1[2], cev2[2];

    pid_t pid1, pid2; 
    
    if (pipe(cev1) == -1) { // naredimo prvo cev
	   printf("error making pipe vhod");
	   return 1;
	}

    if (pipe(cev2) == -1) { // naredimo drugo cev
	   printf("error making pipe izhod");
	   return 1;
	}
    
    pid1 = fork(); // naredimo nov proces
    if ( pid1 == -1 ) {
	   printf("error fork");
	   return 1;
    }

    if (pid1 == 0) {  // če otrok

	   beri_video0(cev1);
    }
    else { // če roditelj

        pid2 = fork(); // naredimo nov proces
        if ( pid2 == -1 ){
            printf("error fork");
            return 1;
        }

        if (pid2 == 0) { // če otrok
            predelaj(cev1, cev2);
        }
        else { // če roditelj

            prikazi(cev2); 
        }   
    }

    return 0;
}
