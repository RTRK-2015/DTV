#ifndef STRUCTURES_H
#define STRUCTURES_H


// C includes
#include <stdint.h>
// Local includes


struct table_header
{
    uint8_t tid;

    union
    {
        struct
        {
            uint16_t len : 12;
            uint16_t res : 2;
            uint16_t zero : 1;
            uint16_t ssi : 1;
        } b1s;

        uint16_t bitfield1;
    } b1u;
} __attribute__((packed));


struct pat_header
{
    struct table_header hdr;

    uint16_t tsi;

    struct
    {
        uint8_t cni : 1;
        uint8_t version : 5;
        uint8_t res : 2;
    } b1s;

    uint8_t sec;
    uint8_t lsn;
} __attribute__((packed));


struct pat_body
{
    uint16_t pr_num;

    union
    {
        struct
        {
            uint16_t pid : 13;
            uint16_t res : 3;
        } b1s;

        uint16_t bitfield1;
    } b1u;
} __attribute__((packed));



struct pmt_header
{
    struct table_header hdr;

    uint16_t pr_num;

    struct
    {
        uint8_t cni : 1;
        uint8_t version : 5;
        uint8_t res : 2;
    } b;

    uint8_t sec;
    uint8_t lsn;

    union
    {
        struct
        {
            uint16_t pcr_pid : 13;
            uint16_t res2 : 3;
        } b1s;

        uint16_t bitfield1;
    } b1u;

    union
    {
        struct
        {
            uint16_t pilen : 12;
            uint16_t res3 : 4;
        } b2s;

        uint16_t bitfield2;
    } b2u;
} __attribute__((packed));


struct pmt_body
{
    uint8_t type;

    union
    {
        struct
        {
	        uint16_t pid : 13;
            uint16_t res : 3;        
        } b1s;

        uint16_t bitfield1;
    } b1u;


    union
    {
        struct
        {
	        uint16_t esilen : 12;
            uint16_t res2 : 4;        
        } b2s;

        uint16_t bitfield2;
    } b2u;
} __attribute__((packed));


// These functions retrieve the corresponding structures from the stream,
// performing the needed network-to-host conversions.
struct pat_header get_pat_header(const uint8_t *buffer);
struct pat_body get_pat_body(const uint8_t *buffer);

struct pmt_header get_pmt_header(const uint8_t *buffer);
struct pmt_body get_pmt_body(const uint8_t *buffer);


#endif
