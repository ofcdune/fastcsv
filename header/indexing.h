#ifndef WATER_INDEXING_H
#define WATER_INDEXING_H

#include "library.h"

#if HAVE_BYTESWAP_H
#include <byteswap.h>
#define byteswap_ulong(x) bswap_32(x)
#else
#define bswap_16(value) \
        ((((value) & 0xff) << 8) | ((value) >> 8))
#define byteswap_ulong(value) \
        (((uint32_t)bswap_16((uint16_t)((value) & 0xffff)) << 16) | \
        (uint32_t)bswap_16((uint16_t)((value) >> 16)))
#endif

#define index_tl unsigned long long
#define index_ll unsigned long long

#define index_tl_dist unsigned long long
#define index_ll_dist unsigned long long

void index_create(int fp_to_index, int index_fp, char index_char);

index_ll_dist index_get_index_count(int index_fp);

void index_seek_in_index_file(int index_fp, index_ll index);
void index_seek_in_tl_file(int fp_to_seek, int index_fp, index_tl indx);

index_ll_dist *index_get_index_bulk(int index_fp, index_tl from, index_tl to, unsigned long long *index_buffer_size);

void *index_read_index(int fp_to_seek, int index_fp, index_tl indx, unsigned long long *restrict buffer_size);
void *index_read_index_bulk(int fp_to_seek, int index_fp, index_tl from, index_tl to, unsigned long long *restrict buffer_size);

unsigned long long index_get_size(int index_fp, index_tl indx);
unsigned long long index_get_size_between(int index_fp, index_tl from, index_tl to);

#endif