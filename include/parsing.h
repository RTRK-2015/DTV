#ifndef PARSING_H
#define PARSING_H


// C includes
#include <stdlib.h>
#include <stdint.h>


struct pmt;


struct pat
{
    uint16_t tsi;

    size_t pmt_len;
    struct pmt *pmts;
};


struct pmt
{
    uint16_t pid;
    uint16_t pr_num;

    size_t video_len;
    uint16_t *video_pids;

    size_t audio_len;
    uint16_t *audio_pids;

    size_t data_len;
    uint16_t *data_pids;
};


// The point of this function is to extract valuable information from the PAT
// and store it in a more compact and structured way.
struct pat parse_pat(const uint8_t *buffer);

// The point of this function is to extract valuable information from the PMT
// and store it in a mroe compact and structured way. Unlike the parse_pat,
// this function does not return the internal PMT table, but rather, embeds
// it into the internal PAT representation.
void parse_pmt(const uint8_t *buffer, struct pat *my_pat);


#endif
