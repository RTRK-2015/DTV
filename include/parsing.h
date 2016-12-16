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
    struct pmt_basic
    { 
    	uint16_t pid;
    	uint16_t ch_num;
    } *pmts;
};


struct pmt
{
    uint16_t pid;
    uint16_t ch_num;

    uint16_t video_pid;
    uint16_t audio_pid;
};



struct pat parse_pat(const uint8_t *buffer);
struct pmt parse_pmt(const uint8_t *buffer);


#endif
