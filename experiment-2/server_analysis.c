/*
 * server_analysis.c
 * ECE 4400 Final Project
 * Experiment 2:
 * Client program for vehicle
 * Based on source code and concepts from
 * "Beej's Guide to Network Programming Using Internet Sockets"
 * by Brian Hall
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <time.h>


struct DataPacket
{
    int type;
    char source[256];
    char dest[256];
    int distance;       //cars will be three units long
    int lane;           //only two lanes
    int time;
    int status;
};

const unsigned int SLEEP_TIME = 700000;

/* returns a string of n length with n-1 spaces and end-terminated
 * with a NULL char
 */
char * blankStr(unsigned int n){
    int i;
    char *string;
    string = (char*) malloc(sizeof(char)*n);
    for (i = 0; i < n-1; i++)
    {
        string[i] = ' ';
    }
    string[n-1] = '\0';
    return string;
}

/* prints to stdout a graphical representation of car positions
 */
void print_vehicle(int lane_print[4], int pos[4]){
    int i, startpoint = 0, endpoint = 75;
    char *lane1;
    char *lane2;
    lane1 = blankStr(83);
    lane2 = blankStr(83);

    system("clear");

    for (i = 0; i < 4; i++)
    {
        if (pos[i] >= endpoint)
        {
            endpoint = pos[i] + 5;
            startpoint = endpoint - 75;
        }

        switch(lane_print[i]){
            case 1 : 
                lane1[pos[i]] = i + '0';
                lane1[pos[i]-1] = '[';
                lane1[pos[i]+1] = ']';
                break;
            case 2 :
                lane2[pos[i]] = i + '0';
                lane2[pos[i]-1] = '[';
                lane2[pos[i]+1] = ']';
                break;
        }

    }

    for (i = 0; i < 16; i++)
    {
        fputs("|----",stdout);
    }
    putc('\n',stdout);
    puts(lane1);
    
    for (i = 0; i < 16; i++)
    {
        fputs("-----",stdout);
    }
    putc('\n',stdout);
    puts(lane2);

    for (i = 0; i < 16; i++)
    {
        fputs("|----",stdout);
    }
    putc('\n',stdout);

    while(startpoint <= endpoint){
        printf("%2d   ", startpoint);
        startpoint += 5;
    }
    putc('\n',stdout);

    // free(lane1);
    // free(lane2);
}

int main()
{
    struct DataPacket read_packet;
    int i, j, k;

    FILE *vehicles[4];

    //open all files for vehicles, replace "cari" with host name in geni
    vehicles[0] = fopen("vehicle-1.v2v-exp2.ch-geni-net.instageni.stanford.edu", "rb");
    vehicles[1] = fopen("vehicle-2.v2v-exp2.ch-geni-net.instageni.stanford.edu", "rb");
    vehicles[2] = fopen("vehicle-3.v2v-exp2.ch-geni-net.instageni.stanford.edu", "rb");
    vehicles[3] = fopen("vehicle-4.v2v-exp2.ch-geni-net.instageni.stanford.edu", "rb");

    int vehicle_position[4][100];
    int vehicle_lane[4][100];
    int lane_print[4];
    int temp_pos[4];

    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 100; j++)
        {
            vehicle_position[i][j] = 0;
        }
    }

    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 20; j++)
        {
            fread(&read_packet, sizeof(read_packet), 1, vehicles[i]);
            vehicle_position[i][read_packet.time] = read_packet.distance;
            vehicle_lane[i][read_packet.time] = read_packet.lane;
        }
    }


    for (i = 0; i < 20; i++)
    {
        lane_print[0] = 0;
        lane_print[1] = 0;
        lane_print[2] = 0;
        lane_print[3] = 0;
        for (j = 0; j < 4; j++)
        {
            //store the vehicle number in lane print
            lane_print[j] = vehicle_lane[j][i];
            for (k = j+1; k < 4; k++)
            {
                if ((vehicle_position[j][i] == vehicle_position[k][i]) && (vehicle_position[j][i] != 0 && vehicle_position[k][i] != 0) && (vehicle_lane[j][i] == vehicle_lane[k][i]))
                {
                    printf("Vehicle %i collided with vehicle %i at %i seconds", j, k, i);
                    exit(1);
                }
            }
        }
        temp_pos[0] = vehicle_position[0][i];
        temp_pos[1] = vehicle_position[1][i];
        temp_pos[2] = vehicle_position[2][i];
        temp_pos[3] = vehicle_position[3][i];
        print_vehicle(lane_print,temp_pos);
        usleep(SLEEP_TIME);
    }

return 0;
}
