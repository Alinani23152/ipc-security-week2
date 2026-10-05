#include <stdio.h>
#include <stdint.h>
#include <arpa/inet.h>

int main(void)
{
    uint16_t host16 = 0x1234;
    uint16_t network16 = htons(host16);

    uint32_t host32 = 0x12345678;
    uint32_t network32 = htonl(host32);

    printf("16-bit host value:    0x%04x\n", host16);
    printf("16-bit network value: 0x%04x\n", network16);

    printf("32-bit host value:    0x%08x\n", host32);
    printf("32-bit network value: 0x%08x\n", network32);

    printf("\nReverse conversion:\n");

    printf("ntohs(network16): 0x%04x\n", ntohs(network16));
    printf("ntohl(network32): 0x%08x\n", ntohl(network32));

    return 0;
}
