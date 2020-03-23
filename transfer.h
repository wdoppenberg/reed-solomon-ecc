#ifndef TRANSFER_H
#define TRANSFER_H

#define HEADER_SIZE 5

// Commented out for now, might need to be used when transferring data serially
//#pragma pack(1) // https://stackoverflow.com/questions/3318410/pragma-pack-effect

struct packet {

    // Refer to ESA PSS-04-107 Issue 2 (April 1992) for details

    /*
     * Serialized (because of #pragma pack(1) structure containing the CLTU bytes / octets;
     * Descriptions per bit along with its bitmask (in binary and hex) are provided
     * for ease of access.
     *
     * Bitmasking: https://stackoverflow.com/questions/10493411/what-is-bit-masking
     */

     /*
     * ---HEADER---
     *
     * Contains data about packet, can be considered part of actual data, in other
     * words it is not the same as the start sequence.
     *
     * First two bytes:
     *  - Version number: 2 bits            | 00000000 00000011 | 0x03
     *  - Bypass flag: 1 bit                | 00000000 00000100 | 0x04
     *  - Control command flag : 1 bit      | 00000000 00001000 | 0x08
     *  - Reserved field A: 2 bits          | 00000000 00110000 | 0x18
     *  - Spacecraft ID: 10 bits            | 11111111 11000000 | 0xFFC0
     *
     * Third byte:
     *  - Virtual channel ID: 6 bits        | 00111111 | 0x3F
     *  - Reserved field B: 2 bits          | 11000000 | 0xC0
     *
     * Last two bytes:
     *  - Frame length: 8 bits              | 11111111 | 0xFF
     *  - Frame sequence number: 8 bits     | 11111111 | 0xFF
     */

    MSG_TYPE header[HEADER_SIZE];

    /*
     * ---DATA---
     *
     */

    MSG_TYPE data[kk];

    /*
     * ---ERROR CORRECTION FIELD---
     *
     */

    MSG_TYPE ECF[nn-kk];

};

typedef struct packet packet_t;

/*
 * Transfer struct, enables intuitive overview of data, along with the size
 * in data packets enclosed within.
 */

struct transfer {

    packet_t * packs;
    int size;

};

typedef struct transfer transfer_t;

void encode_rs(packet_t * packet);
void decode_rs(packet_t * packet);

/*
 * FILL DATA PACKETS
 */

void encode_transfer(transfer_t * transfer)
{
    // Error correction field
    for (int i = 0; i < transfer->size; i++)
        encode_rs(&(transfer->packs[i]));
}

transfer_t gen_transfer(const MSG_TYPE * data, const int size)
{
    register int i;

    transfer_t transfer;

    transfer.size   = size/kk + (size % kk != 0); // # of packets required to store message
    transfer.packs  = malloc(transfer.size * sizeof(packet_t));

    //printf("Generating %i packets...\n", transfer.size);

    int F_length;

    /*
     * Fill data packet(s)
     */

    for (i = 0; i < transfer.size; i++)
    {
        // Target information bytes - placeholder: these are the bitmasks.
        transfer.packs[i].header[0] |= 0x03u; // 00000011
        transfer.packs[i].header[0] |= 0x04u; // 00000100
        transfer.packs[i].header[0] |= 0x08u; // 00001000
        transfer.packs[i].header[0] |= 0x18u; // 00110000
        transfer.packs[i].header[0] |= 0xC0u; // 11000000
        transfer.packs[i].header[1] |= 0xFFu; // 11111111
        transfer.packs[i].header[2] |= 0x3Fu; // 00111111
        transfer.packs[i].header[2] |= 0xC0u; // 11000000

        F_length = ((size - (i+1)*kk) < 0) ? (size % kk) : kk; // Frame length
        //printf("Frame Length packet[%i] = %d\n", i, F_length);

        transfer.packs[i].header[3] = (MSG_TYPE) F_length;
        transfer.packs[i].header[4] = (MSG_TYPE) i; // Frame sequence number

        for (int j = 0; j < F_length; j++)
            transfer.packs[i].data[j] = data[j+i*kk];

        // Pad packet
        if (F_length < kk)
            for (int j = F_length; j < kk; j++)
                transfer.packs[i].data[j] = 0;

        // Generate ECF for all packets.
        encode_transfer(&transfer);
    }

    return transfer;
}

void decode_transfer(transfer_t * transfer)
{
    register int i;
    //printf("Decoding %i data packets...\n", transfer->size);

    for (i = 0; i < transfer->size; i++)
        decode_rs(&(transfer->packs[i]));

}

MSG_TYPE * unpack_transfer(transfer_t * transfer)
{
    register int i, j, N = 0;

    decode_transfer(transfer);

    // Determine message size
    for (i = 0; i<transfer->size; i++)
        N += transfer->packs[i].header[3];

    /*
     * It might not be as straightforward to unpack a transfer frame
     * after it has been received, since the size (in # of packets) needs
     * to be transmitted to the receiver alongside the message.
     */

    MSG_TYPE * msg = malloc(N * sizeof(MSG_TYPE));

    // Loop over data in all packets to form single message array
    for (i = 0; i<transfer->size; i++)
        for (j = 0; j<transfer->packs[i].header[3]; j++)
            msg[j + i*kk] = transfer->packs[i].data[j];


    return msg;
}

// Print transfer
void print_transfer(transfer_t * transfer)
{
    register int i,j;

    packet_t * packs = transfer->packs;

    printf("\n---TRANSFER---\n------------------------------------"
           "-----------------------------------\n");

    printf("Pack[ ]:");
    for (i = 0; i < transfer->size; i++) {printf("\t\t\t[%i]", i);}
    printf("\n");
    for (j = 0; j < HEADER_SIZE; j++) {
        printf("header[%i]\t\t\t", j);
        for (i = 0; i < transfer->size; i++)
            printf("%u\t\t\t", packs[i].header[j]);

        printf("\n");
    }
    for (j = 0; j < kk; j++) {
        printf("data[%i] \t\t\t", j);
        for (i = 0; i < transfer->size; i++)
            printf("%i\t\t\t", packs[i].data[j]);

        printf("\n");
    }
    for (j = 0; j < (nn-kk); j++) {
        printf("ECF[%i]   \t\t\t", j);
        for (i = 0; i < transfer->size; i++)
            printf("%i\t\t\t", packs[i].ECF[j]);

        printf("\n");
    }
}

#endif //TRANSFER_H
