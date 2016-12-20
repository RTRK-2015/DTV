#ifndef PARSING_H
#define PARSING_H


// C includes
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>


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

    bool teletext;
};


struct sdt
{
    uint8_t st;
    char name[100];
};


struct pat parse_pat(const uint8_t *buffer);
struct pmt parse_pmt(const uint8_t *buffer);
struct tm  parse_tot(const uint8_t *buffer);
struct sdt parse_sdt(const uint8_t *buffer, uint16_t ch_num);


#endif
