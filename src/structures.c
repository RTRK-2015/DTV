// Matching include
#include "structures.h"
// C includes
#include <stdint.h>
#include <arpa/inet.h>
// Unix includes


static void fix_table_header(struct table_header *hdr)
{
    hdr->b1u.bitfield1 = ntohs(hdr->b1u.bitfield1);
}


struct pat_header get_pat_header(const uint8_t *buffer)
{
    struct pat_header pat_h =
        *(const struct pat_header *)buffer;

    fix_table_header(&pat_h.hdr);
    pat_h.tsi = ntohs(pat_h.tsi);

    return pat_h;
}


struct pat_body get_pat_body(const uint8_t *buffer)
{
    struct pat_body pat_b =
        *(const struct pat_body *)buffer;

    pat_b.ch_num = ntohs(pat_b.ch_num);
    pat_b.b1u.bitfield1 = ntohs(pat_b.b1u.bitfield1);

    return pat_b;
}



struct pmt_header get_pmt_header(const uint8_t *buffer)
{
    struct pmt_header pmt_h =
        *(const struct pmt_header *)buffer;

    fix_table_header(&pmt_h.hdr);
    pmt_h.ch_num = ntohs(pmt_h.ch_num);
    pmt_h.b1u.bitfield1 = ntohs(pmt_h.b1u.bitfield1);
    pmt_h.b2u.bitfield2 = ntohs(pmt_h.b2u.bitfield2);

    return pmt_h;
}


struct pmt_body get_pmt_body(const uint8_t *buffer)
{
    struct pmt_body pmt_b =
        *(const struct pmt_body *)buffer;

    pmt_b.b1u.bitfield1 = ntohs(pmt_b.b1u.bitfield1);
    pmt_b.b2u.bitfield2 = ntohs(pmt_b.b2u.bitfield2);

    return pmt_b;
}


struct sdt_header get_sdt_header(const uint8_t *buffer)
{
	struct sdt_header sdt_h =
		*(const struct sdt_header *)buffer;
		
	fix_table_header(&sdt_h.hdr);
	sdt_h.tsi = ntohs(sdt_h.tsi);
	sdt_h.oni = ntohs(sdt_h.oni);
	
	return sdt_h;
}


struct sdt_body get_sdt_body(const uint8_t *buffer)
{
	struct sdt_body sdt_b =
		*(const struct sdt_body *)buffer;
		
	sdt_b.sid = ntohs(sdt_b.sid);
	sdt_b.b2u.bitfield2 = ntohs(sdt_b.b2u.bitfield2);
	
	return sdt_b;
}


struct sdt_descriptor1 get_sdt_descriptor1(const uint8_t *buffer)
{
	return *(const struct sdt_descriptor1 *)buffer;
}


struct sdt_descriptor2 get_sdt_descriptor2(const uint8_t *buffer)
{
    return *(const struct sdt_descriptor2 *)buffer;
}


struct tot_header get_tot_header(const uint8_t *buffer)
{
    struct tot_header tot_h =
        *(const struct tot_header *)buffer;

    fix_table_header(&tot_h.hdr);
    tot_h.b1u.bitfield1 = ntohs(tot_h.b1u.bitfield1);
	
    return tot_h;
}


struct tot_descriptor_header get_tot_descriptor_header(const uint8_t *buffer)
{
    return *(const struct tot_descriptor_header *)buffer;
}


struct tot_descriptor_body get_tot_descriptor_body(const uint8_t *buffer)
{
    struct tot_descriptor_body tot_d_b =
        *(const struct tot_descriptor_body *)buffer;

    tot_d_b.b1u.bitfield1 = ntohl(tot_d_b.b1u.bitfield1);

    return tot_d_b;
}

