

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/time.h>
#include <time.h>
#include <sys/times.h>


int beri_video0(char* vhod_pointer, int vhod_trafficLight) {

    // inicializacija za delo s semaforjem
    struct sembuf trafficLight;

    int video0 = open("/dev/video0", O_RDONLY);

    printf("starting prvi\n");

    int bytes = 640 * 480 * 3;

    int rd, wr;

    int i = 0;

    while(1) {

        trafficLight.sem_num = 1;
        trafficLight.sem_op = -1;
        trafficLight.sem_flg = 0;
        // atomična operacija za delo s semaforjem
        semop(vhod_trafficLight, &trafficLight, 1);

        // preberemo in zapišemo v deljeni pomnilnik 
        rd = read(video0, vhod_pointer, bytes);

        trafficLight.sem_num = 0;
        trafficLight.sem_op =  1;
        trafficLight.sem_flg = 0;
        semop(vhod_trafficLight, &trafficLight, 1);


        printf("prvi %d\n", i);

        lseek(video0, SEEK_SET, 0); 

        i++;

    }

    close(video0);

    return 0;

}

int predelaj(char* vhod_pointer, int vhod_trafficLight, char* izhod_pointer, int izhod_trafficLight) {



    int bytes_in = 640 * 480 * 3;
    int bytes_out = 1920 * 1080 * 2;

    char* buffer_in = (char*) malloc(bytes_in * sizeof(char));
    char* buffer_out = (char*) malloc(bytes_out * sizeof(char));

    // inicializacija za delo s semaforjema
    struct sembuf trafficLight_in;
    struct sembuf trafficLight_out;

    printf("starting drugi\n");

    int rd, wr;

    int i = 0;
    while(1) {

        trafficLight_in.sem_num = 0;
        trafficLight_in.sem_op = -1;
        trafficLight_in.sem_flg = 0;
        semop(vhod_trafficLight, &trafficLight_in, 1);

        memcpy(buffer_in, vhod_pointer, bytes_in);

        //rd = read(vhod_pointer, buffer_in, bytes_in); // beremo iz prve cevi

        trafficLight_in.sem_num = 1;
        trafficLight_in.sem_op = 1;
        trafficLight_in.sem_flg = 0;
        semop(vhod_trafficLight, &trafficLight_in, 1);

        for(int i = 0; i < 1920 * 1080; i++) {

            if ((i < 1920*300) || (i > 1920 * 779)) {

                buffer_out[2*i] = 0;
                buffer_out[2*i+1] = 0;
            }
            else if ((i % 1920 < 640) || (i % 1920 > 1279)){

                    buffer_out[2*i] = 0;
                    buffer_out[2*i+1] = 0;
            }
            else {

                int stolpec = i % 1920 - 640;
                int vrstica = i / 1920 - 300;
                int polozaj = (vrstica) * 640 + stolpec;

                int r_in = buffer_in[3*polozaj];
                int g_in = buffer_in[3*polozaj+1];
                int b_in = buffer_in[3*polozaj+2];

                int r_new = (float) (r_in / 8);
                int g_new = (float) (g_in / 4);
                int b_new = (float) (b_in / 8);

                u_int8_t byte2 = ((7 & g_new) << 5) + (31 & b_new);
                u_int8_t byte1 = ((31 & r_new) << 3) + ((56 & g_new) >> 3);

                buffer_out[2*i] = byte2;
                buffer_out[2*i+1] = byte1;
            }
        }

        trafficLight_out.sem_num = 1;
        trafficLight_out.sem_op = -1;
        trafficLight_out.sem_flg = 0;
        semop(izhod_trafficLight, &trafficLight_out, 1);

        //wr = write(izhod, buffer_out, bytes_out); // zapišemo v drugo cev

        memcpy(izhod_pointer, buffer_out, bytes_out);

        trafficLight_out.sem_num = 0;
        trafficLight_out.sem_op =  1;
        trafficLight_out.sem_flg = 0;
        semop(izhod_trafficLight, &trafficLight_out, 1);

        printf("drugi %d\n", i);
        i++;

    }

    free(buffer_in);
    free(buffer_out);

    return 0;
}


int prikazi(char* izhod_pointer, int izhod_trafficLight) {


    int bytes = 1920 * 1080 * 2;

    char* buffer = (char*) malloc(bytes * sizeof(char));

    struct sembuf trafficLight; // inicializacija za delo s semaforjem

    int rd, wr;

    printf("starting tretji\n");

    int fo = open("/dev/fb0", O_WRONLY | O_CREAT,0755);

    int i = 0;
    while(1) {

        trafficLight.sem_num = 0;
        trafficLight.sem_op =  -1;
        trafficLight.sem_flg = 0;
        semop(izhod_trafficLight, &trafficLight, 1);

        //rd = read(izhod_pointer, fo, bytes);
        memcpy(buffer, izhod_pointer, bytes); 

        trafficLight.sem_num = 1;
        trafficLight.sem_op =  1;
        trafficLight.sem_flg = 0;
        semop(izhod_trafficLight, &trafficLight, 1);

        // sliko prikažemo
        wr = write(fo, buffer, bytes);
        lseek(fo, SEEK_SET, 0);

        printf("tretji %d\n", i);
        i++;
    }

    close(fo);
    free(buffer);

    return 0;
}

int main(void) {

    
    int vhod = shmget(IPC_PRIVATE, 640 * 480 * 3, IPC_CREAT | 0666); // inicializacija deljenega pomnilnika
    int izhod = shmget(IPC_PRIVATE, 1920 * 1080 * 2, IPC_CREAT | 0666);

    char* vhod_pointer = shmat(vhod, NULL, 0);
    char* izhod_pointer = shmat(izhod, NULL, 0);

    // inicializacija semaforja    
    int vhod_trafficLight = semget(IPC_PRIVATE, 2, IPC_CREAT | 0600);
    int izhod_trafficLight = semget(IPC_PRIVATE, 2, IPC_CREAT | 0600);

    // začetne vrednosti
    unsigned short vhod_initial[2], izhod_initial[2];
    vhod_initial[0] = 0;
    vhod_initial[1] = 1;
    semctl(vhod_trafficLight, 0, SETALL, vhod_initial);

    izhod_initial[0] = 0;
    izhod_initial[1] = 1;
    semctl(izhod_trafficLight, 0, SETALL, izhod_initial);

    pid_t pid1, pid2; 
    
    pid1 = fork(); // naredimo nov proces
    if ( pid1 == -1 ) {
	   printf("error fork");
	   return 1;
    }
    if (pid1 == 0) {  // če otrok

	   beri_video0(vhod_pointer, vhod_trafficLight);
    }
    else { // če roditelj

        pid2 = fork(); // naredimo nov proces
        if ( pid2 == -1 ){
            printf("error fork");
            return 1;
        }
        if (pid2 == 0) { // če otrok
            predelaj(vhod_pointer, vhod_trafficLight, izhod_pointer, izhod_trafficLight);
        }
        
        else { // če roditelj

            prikazi(izhod_pointer, izhod_trafficLight); 
        }   
    }

    return 0;
}