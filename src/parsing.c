// Matching include
#include "parsing.h"
// C includes
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
// Local includes
#include "common.h"
#include "structures.h"


struct pat parse_pat(const uint8_t *buffer)
{
    struct pat_header pat_h = get_pat_header(buffer);

    printf("tid: %d, len: %d, tsi: %d\n",
        pat_h.hdr.tid, pat_h.hdr.b1u.b1s.len, pat_h.tsi);

    struct pat my_pat =
    { .tsi = pat_h.tsi
    , .pmt_len = 0
    };

    // Since the number of PMT entries is unknown until the whole PAT has
    // been parsed, the array will probably have to be expanded at some point.
    // The starting guess is 5 PMT entries.
    size_t res = 5;
    my_pat.pmts = (struct pmt *)malloc(res * sizeof(struct pmt));
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
            my_pat.pmts =
                (struct pmt *)realloc(my_pat.pmts, res * sizeof(struct pmt));

            if (my_pat.pmts == NULL)
                FAIL_STD("%s\n", nameof(realloc));
        }

        // Every pmt entry is "initialized" here. The most important part is
        // the pr_num field, which will allow every PMT to find its place
        // in the pmts array.
        my_pat.pmts[my_pat.pmt_len].pid = pat_b.b1u.b1s.pid;
        my_pat.pmts[my_pat.pmt_len].pr_num = pat_b.pr_num;
        my_pat.pmts[my_pat.pmt_len].video_len = 0;
        my_pat.pmts[my_pat.pmt_len].video_pids = NULL;
        my_pat.pmts[my_pat.pmt_len].audio_len = 0;
        my_pat.pmts[my_pat.pmt_len].audio_pids = NULL;
        my_pat.pmts[my_pat.pmt_len].data_len = 0;
        my_pat.pmts[my_pat.pmt_len].data_pids = NULL;

        ++my_pat.pmt_len;

        current_ptr += sizeof(struct pat_body);
    }

    return my_pat;
}


void parse_pmt(const uint8_t *buffer, struct pat *my_pat)
{	
    struct pmt_header pmt_h = get_pmt_header(buffer);

    // Find the appropriate place for this PMT
    size_t idx = 0;
    struct pmt *my_pmt = my_pat->pmts;
    while (idx < my_pat->pmt_len && pmt_h.pr_num != my_pmt->pr_num)
    {
        ++idx;
        ++my_pmt;
    }
    if (idx == my_pat->pmt_len)
        FAIL("Cannot find a place for PMT %d\n", pmt_h.pr_num);

	if (my_pmt->video_pids != NULL || my_pmt->audio_pids != NULL || my_pmt->data_pids != NULL)
		return;

    // The number of streams in a PMT is unknown, so the corresponding arrays
    // will have to be expanded at some point. The starting guess is 5 of each
    // type of stream.
    size_t
        video_res = 5,
        audio_res = 5,
        data_res  = 5;
    my_pmt->video_pids = (uint16_t *)malloc(video_res * sizeof(uint16_t));
    if (my_pmt->video_pids == NULL)
        FAIL_STD("%s\n", nameof(malloc));
    my_pmt->audio_pids = (uint16_t *)malloc(audio_res * sizeof(uint16_t));
    if (my_pmt->audio_pids == NULL)
        FAIL_STD("%s\n", nameof(malloc));
    my_pmt->data_pids = (uint16_t *)malloc(data_res * sizeof(uint16_t));
    if (my_pmt->data_pids == NULL)
        FAIL_STD("%s\n", nameof(malloc));


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
    	printf("%d\n", current_ptr - buffer);
    	
        struct pmt_body pmt_b = get_pmt_body(current_ptr);

        // If the number of stored elements has grown to be equal to the number
        // of reserved slots, the array will have to be reallocated. It is
        // expanded by a factor of 2, which holds no special meaning.
        if (my_pmt->video_len >= video_res)
        {
            video_res *= 2;
            my_pmt->video_pids = (uint16_t *)realloc
            ( my_pmt->video_pids
            , video_res * sizeof(uint16_t)
            );

            if (my_pmt->video_pids == NULL)
                FAIL_STD("%s\n", nameof(realloc));
        }
        if (my_pmt->audio_len >= audio_res)
        {
            audio_res *= 2;
            my_pmt->audio_pids = (uint16_t *)realloc
            ( my_pmt->audio_pids
            , audio_res * sizeof(uint16_t)
            );

            if (my_pmt->audio_pids == NULL)
                FAIL_STD("%s\n", nameof(realloc));
        }
        if (my_pmt->data_len >= data_res)
        {
            data_res *= 2;
            my_pmt->data_pids = (uint16_t *)realloc
            ( my_pmt->data_pids
            , data_res * sizeof(uint16_t)
            );

            if (my_pmt->data_pids == NULL)
                FAIL_STD("%s\n", nameof(realloc));
        }


        // If the current body section contains a supported type of stream,
        // add it to the matching array.
        if (pmt_b.type == 0x02)
            my_pmt->video_pids[my_pmt->video_len++] = pmt_b.b1u.b1s.pid;
        else if (pmt_b.type == 0x03)
            my_pmt->audio_pids[my_pmt->audio_len++] = pmt_b.b1u.b1s.pid;
        else if (pmt_b.type == 0x06)
            my_pmt->data_pids[my_pmt->data_len++] = pmt_b.b1u.b1s.pid;

        // Finally, advance the current_ptr by the size of the body section
        // and the size of the descriptors section that belongs to every
        // body section.
        current_ptr += sizeof(struct pmt_body) + pmt_b.b2u.b2s.esilen;
    }
}
