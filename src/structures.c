// Matching include
#include "structures.h"
// C includes
#include <stdint.h>
// Unix includes

uint16_t ntohs(uint16_t x);


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

    pat_b.pr_num = ntohs(pat_b.pr_num);
    pat_b.b1u.bitfield1 = ntohs(pat_b.b1u.bitfield1);

    return pat_b;
}



struct pmt_header get_pmt_header(const uint8_t *buffer)
{
    struct pmt_header pmt_h =
        *(const struct pmt_header *)buffer;

    fix_table_header(&pmt_h.hdr);
    pmt_h.pr_num = ntohs(pmt_h.pr_num);
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
