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
    uint16_t ch_num;

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

    uint16_t ch_num;

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


struct sdt_header
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
    
    uint16_t oni;
    uint8_t res2;
} __attribute__((packed));


struct sdt_body
{
	uint16_t sid;
	
	struct
	{
		uint8_t epff : 1;
		uint8_t esf : 1;
		uint8_t res : 6;
	} b1s;
	
	union
	{
		struct
		{
			uint16_t dlen : 12;
			uint16_t fcm : 1;
			uint16_t rs : 3;
		} b2s;
		
		uint16_t bitfield2;
	} b2u;
} __attribute__((packed));


struct sdt_descriptor1
{
	uint8_t tag;
	uint8_t len;
	uint8_t type;
	uint8_t spnlen;
} __attribute__((packed));


struct sdt_descriptor2
{
	uint8_t snlen;
} __attribute__((packed));


struct tot_header
{
	struct table_header hdr;
	
	uint8_t time[5];
	
	union
	{
		struct
		{
			uint16_t dlen : 12;
			uint16_t res : 4;
		} b1s;
		
		uint16_t bitfield1;
	} b1u;
} __attribute__((packed));


struct tot_descriptor_header
{
    uint8_t tag;
    uint8_t len;
} __attribute__((packed));


struct tot_descriptor_body
{
    union
    {
        struct
        {
            uint32_t cc : 24;
            uint32_t regid : 6;
            uint32_t res : 1;
            uint32_t pol : 1;
        } b1s;

        uint32_t bitfield1;
    } b1u;

    uint16_t lto;
    uint8_t toc[5];
    uint16_t nto;
} __attribute__((packed));



// These functions retrieve the corresponding structures from the stream,
// performing the needed network-to-host conversions.
struct pat_header get_pat_header(const uint8_t *buffer);
struct pat_body get_pat_body(const uint8_t *buffer);

struct pmt_header get_pmt_header(const uint8_t *buffer);
struct pmt_body get_pmt_body(const uint8_t *buffer);

struct sdt_header get_sdt_header(const uint8_t *buffer);
struct sdt_body get_sdt_body(const uint8_t *buffer);
struct sdt_descriptor1 get_sdt_descriptor1(const uint8_t *buffer);
struct sdt_descriptor2 get_sdt_descriptor2(const uint8_t *buffer);

struct tot_header get_tot_header(const uint8_t *buffer);
struct tot_descriptor_header get_tot_descriptor_header(const uint8_t *buffer);
struct tot_descriptor_body get_tot_descriptor_body(const uint8_t *buffer);


#endif
