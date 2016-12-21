// Matching include
#include "parsing.h"
// C includes
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
// Local includes
#include "common.h"
#include "structures.h"


#define LOG_PARSING(fmt, ...) LOG("Parsing", fmt, ##__VA_ARGS__)

struct pat parse_pat(const uint8_t *buffer)
{
    struct pat_header pat_h = get_pat_header(buffer);

    LOG_PARSING("tid: %d, len: %d, tsi: %d\n",
        pat_h.hdr.tid, pat_h.hdr.b1u.b1s.len, pat_h.tsi);

    struct pat my_pat =
    { .tsi = pat_h.tsi
    , .pmt_len = 0
    };

    // Since the number of PMT entries is unknown until the whole PAT has
    // been parsed, the array will probably have to be expanded at some point.
    // The starting guess is 5 PMT entries.
    size_t res = 5;
    my_pat.pmts = (struct pmt_basic *)malloc(res * sizeof(struct pmt_basic));
    if (my_pat.pmts == NULL)
        FAIL_STD("%s\n", nameof(malloc));

    // current_ptr tracks the current body section. At the start, it points
    // to the *second* body section (the first contains network pid, which
    // is currently unimportant).
    // end_ptr is the pointer to the end of the last body section, which is
    // located 4 bytes before the end of the PAT
    const uint8_t
        *current_ptr = buffer + sizeof(pat_h) + sizeof(struct pat_body),
        *end_ptr = buffer + sizeof(struct table_header) + pat_h.hdr.b1u.b1s.len - 4;

    while (current_ptr < end_ptr)
    {
        struct pat_body pat_b = get_pat_body(current_ptr);

        // If the number of stored elements has grown to be equal to the number
        // of reserved slots, the array will have to be reallocated. It is
        // expanded by a factor of 2, which holds no special meaning.
        if (my_pat.pmt_len >= res)
        {
            res *= 2;
            my_pat.pmts = (struct pmt_basic *)realloc
            ( my_pat.pmts
            , res * sizeof(struct pmt_basic)
            );

            if (my_pat.pmts == NULL)
                FAIL_STD("%s\n", nameof(realloc));
        }

        // Every pmt entry is "initialized" here. The most important part is
        // the ch_num field, which will allow every PMT to find its place
        // in the pmts array.
        my_pat.pmts[my_pat.pmt_len].pid = pat_b.b1u.b1s.pid;
        my_pat.pmts[my_pat.pmt_len].ch_num = pat_b.ch_num;

        ++my_pat.pmt_len;

        current_ptr += sizeof(struct pat_body);
    }

    return my_pat;
}


struct pmt parse_pmt(const uint8_t *buffer)
{
    struct pmt_header pmt_h = get_pmt_header(buffer);

    struct pmt my_pmt = 
    { .video_pid = UINT16_C(0xFFFF)
    , .audio_pid = UINT16_C(0xFFFF)
    , .teletext = false
    };


    // current_ptr tracks the current body section. At the start, it points
    // to the first body section, which is just past the descriptors section,
    // the size of which is contained in the pilen field.
    // end_ptr is the pointer to the end of the last body section, which is
    // located 4 bytes before the end of the PMT
    const uint8_t
        *current_ptr = buffer + sizeof(pmt_h) + pmt_h.b2u.b2s.pilen,
        *end_ptr = buffer + sizeof(struct table_header) + pmt_h.hdr.b1u.b1s.len - 4;

    while (current_ptr < end_ptr)
    {
        struct pmt_body pmt_b = get_pmt_body(current_ptr);

        LOG_PARSING("type: %d, pid: %d\n", pmt_b.type, pmt_b.b1u.b1s.pid);
        
        if (pmt_b.type == 0x02 && my_pmt.video_pid == UINT16_C(0xFFFF))
        {
            my_pmt.video_pid = pmt_b.b1u.b1s.pid;
        }
        else if (pmt_b.type == 0x03 && my_pmt.audio_pid == UINT16_C(0xFFFF))
        {
            my_pmt.audio_pid = pmt_b.b1u.b1s.pid;
        }
        else if (pmt_b.type == 0x06 && !my_pmt.teletext)
        {
            if (current_ptr[sizeof(struct pmt_body)] == 0x56)
            {
                LOG_PARSING("FOUND TELETEXT!\n");
                my_pmt.teletext = true;
            }
        }

        // Advance the current_ptr by the size of the body section
        // and the size of the descriptors section that belongs to every
        // body section.
        current_ptr += sizeof(struct pmt_body) + pmt_b.b2u.b2s.esilen;
    }
    
    return my_pmt;
}


struct sdt parse_sdt(const uint8_t *buffer, uint16_t ch_num)
{
    struct sdt_header sdt_h = get_sdt_header(buffer);

    struct sdt my_sdt = { 0 };

    const uint8_t
        *current_ptr = buffer + sizeof(sdt_h),
        *end_ptr = buffer + sizeof(struct table_header) + sdt_h.hdr.b1u.b1s.len - 4;

    while (current_ptr < end_ptr)
    {
        struct sdt_body sdt_b = get_sdt_body(current_ptr);
        LOG_PARSING("sid: %d\n", sdt_b.sid);
        if (sdt_b.sid == ch_num)
        {
            LOG_PARSING("Found appropriate sdt\n");

            current_ptr += sizeof(sdt_b);
            struct sdt_descriptor1 sdt_d1 = get_sdt_descriptor1(current_ptr);
            LOG_PARSING("Service type: %d\n", sdt_d1.type);
            my_sdt.st = sdt_d1.type;

            current_ptr += sizeof(sdt_d1) + sdt_d1.spnlen;
            struct sdt_descriptor2 sdt_d2 = get_sdt_descriptor2(current_ptr);
            current_ptr += sizeof(sdt_d2);

            LOG_PARSING("snlen: %d\n", sdt_d2.snlen);
            strncpy(my_sdt.name, (const char*)current_ptr, sdt_d2.snlen);
            my_sdt.name[sdt_d2.snlen] = '\0';
            LOG_PARSING("Name: %s\n", my_sdt.name);

            current_ptr += sdt_d2.snlen;
            break;
        }
        else
        {
            current_ptr += sizeof(sdt_b) + sdt_b.b2u.b2s.dlen;
        }
    }

    return my_sdt;
}


struct tm parse_tot(const uint8_t *buffer)
{
    struct tot_header tot_h = get_tot_header(buffer);

    uint8_t bcd_hour = tot_h.time[2];
    uint8_t hour = (bcd_hour / 16) * 10 + bcd_hour % 16;

    uint8_t bcd_minute = tot_h.time[3];
    uint8_t minute = (bcd_minute / 16) * 10 + bcd_minute % 16;

    uint8_t bcd_second = tot_h.time[4];
    uint8_t second = (bcd_second / 16) * 10 + bcd_second % 16;

    uint16_t date = (tot_h.time[0] << 8) | tot_h.time[1];
    LOG_PARSING("encoded date: %x\n", date);

    struct tot_descriptor_header tot_d_h = get_tot_descriptor_header(buffer);
    if (tot_d_h.tag == 0x58)
    {
        struct tot_descriptor_body tot_d_b = get_tot_descriptor_body(buffer);
        date += tot_d_b.lto;
    }

    int yprime = (date - 15078.2) / 365.25;
    int mprime = (date - 14956.1 - (int)(yprime * 365.25)) / 30.6001;
    int d = date - 14956 - (int)(yprime * 365.25) - (int)(mprime * 30.6001);
    LOG_PARSING("y': %d, m': %d, d: %d\n", yprime, mprime, d);

    int k = (mprime == 14 || mprime == 15);
    int y = yprime + k;
    int m = mprime - 1 - k * 12;

    struct tm tm =
    { .tm_sec = second
    , .tm_min = minute
    , .tm_hour = hour
    , .tm_mday = d
    , .tm_mon = m - 1
    , .tm_year = y
    };

    char buf[100] = "";
    strftime(buf, 100, "Time: %Y-%m-%dT%H:%M:%S", &tm);
    puts(buf);

    return tm;
}

