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
    struct
    { 
    	uint16_t pid;
    	uint16_t ch_num;
    } pmts;
};


struct pmt
{
    uint16_t pid;
    uint16_t ch_num;

    size_t video_len;
    uint16_t *video_pids;

    size_t audio_len;
    uint16_t *audio_pids;
};



struct pat parse_pat(const uint8_t *buffer);
struct pmt parse_pmt(const uint8_t *buffer);


#endif
