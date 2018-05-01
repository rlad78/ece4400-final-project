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

    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 100; j++)
        {
            vehicle_position[i][j] = 0;
        }
    }

    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 10; j++)
        {
            fread(&read_packet, sizeof(read_packet), 1, vehicles[i]);
            vehicle_position[i][read_packet.time] = read_packet.distance;
        }
    }



    for (i = 0; i < 100; i++)
    {
        for (j = 0; j < 4; j++)
        {
            for (k = j+1; k < 4; k++)
            {
                if ((vehicle_position[j][i] == vehicle_position[k][i]) && (vehicle_position[j][i] != 0 && vehicle_position[k][i] != 0))
                {
                    printf("Vehicle %i collided with vehicle %i at %i seconds", j, k, i);
                    break;
                }
            }
        }
    }

return 0;
}
