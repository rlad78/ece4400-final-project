/*
 * light_parser.c
 * ECE 4400 Final Project
 * Experiment 1: Fleet Control Stoplight
 * Parser for recorded packets
 */
#include <stdio.h>
#include <stdlib.h>

struct DataPacket
{
    int type;
    char source[256];
    char dest[256];
    int distance;
    int time;
    int status;
    int padding[16];
};

int main()
{
    struct DataPacket read_packet;
    int i, j, k;

    FILE* vehicles[4];

    vehicles[0] = fopen("car1.v2v-exp1.ch-geni-net.instageni.rnoc.gatech.edu", "rb");
    vehicles[1] = fopen("car2.v2v-exp1.ch-geni-net.instageni.rnoc.gatech.edu", "rb");
    vehicles[2] = fopen("car3.v2v-exp1.ch-geni-net.instageni.rnoc.gatech.edu", "rb");
    vehicles[3] = fopen("car4.v2v-exp1.ch-geni-net.instageni.rnoc.gatech.edu", "rb");

    int vehicle_position[4][100];

    for (i = 0; i < 4; i++)
        for (j = 0; j < 10; j++)
        {
            fread(&read_packet, sizeof(read_packet), 1, vehicles[i]);
            vehicle_position[i][read_packet.time] = read_packet.distance;
        }

    for (i = 0; i < 100; i++)
        for (j = 0; j < 4; j++)
            for (k = 0; k < 4; k++)
                if (vehicle_position[j][i] == vehicle_position[k][i])
                    printf("Vehicle %i collided with vehicle %i at %i seconds", j, k, i);

    return 0;
}
