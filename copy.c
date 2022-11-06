#include <stdio.h>
#include <stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include <fcntl.h>

int main(int argc, char **argv)
{
    if (argc<3){
        printf("Error, you didn't provide enough input arguments\n");
        }
    else{
        char* source=argv[1];
        char* destination=argv[2];

        //odpremo izvorno datoteko
        int f_source= open(source, O_RDONLY);
        if (f_source>0){
                int f_destination=open(destination, O_WRONLY | O_CREAT,0755);
                if (f_destination>0){
                    int block_size=1048576; //1048576
                    int read_num;
                    int wrote_num;
                    char *buffer = (char *) calloc(block_size, sizeof(char));

                    //get size:
                    unsigned long size  = lseek(f_source, 0, SEEK_END);
                    lseek(f_source, 0, 0);
                    unsigned long now;
                    double percentage;

                    do {
                        //branje
                        read_num = read(f_source, buffer, block_size);
                        //printf("called read  returned that %d bytes  were read.\n", read_num);

                        //pisanje
                        wrote_num=write(f_destination, buffer, read_num);
                        //printf("called write returned that %d bytes  were written.\n", wrote_num);
                        if (wrote_num!=read_num){
                            printf("Error in copying, canceling operation.\n");
                            break;
                            }
                        //calculate percentage
                        now=lseek(f_destination, 0, SEEK_CUR);
                        percentage=(double)now/size*100;
                        printf("\rCopying %.2f%% ...",percentage);
                        fflush(stdout);
                        }
                    while (read_num==block_size);
                    printf("\rCopying %.2f%% ...",(double)100);
                    printf("\n");
                    close(f_destination);
                    }
                else{
                    printf("Error opening a destination file\n");
                    }
            close(f_source);
            }
        else{
            printf("Error opening a source file\n");
            }
        }
    return 0;
}
