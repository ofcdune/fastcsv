#include "../header/indexing.h"
#include <limits.h>

#define CHUNKSIZE 1048576

long long index_get_index_offset(index_tl index) {
    return (index + 1) * 8;
}

void index_create(int fp_to_index, int index_fp, char index_char) {
    NEW(unsigned char, read_buffer, CHUNKSIZE)
    NEW(unsigned long long, write_buffer, CHUNKSIZE)

    char running = 1;

    unsigned long long
        write_offset = 0,
        read_chunk_count = 0,
        bytes_read,
        i,
        write_count = 0;

    write(index_fp, (unsigned char *) &write_count, 8);
    write(index_fp, (unsigned char *) &read_chunk_count, 8);

    while (running) {
        bytes_read = read(fp_to_index, read_buffer, CHUNKSIZE);
        if (bytes_read != CHUNKSIZE) {
            running = 0;
        }

        for (i = 0; i < bytes_read; i++) {
            if (read_buffer[i] != index_char) {
                continue;
            }

            write_buffer[write_offset++] = (read_chunk_count*CHUNKSIZE)+i+1;
            write_count++;

            if (write_offset == CHUNKSIZE) {
                write(index_fp, (unsigned char *) write_buffer, write_offset*8);
                write_offset = 0;
            }
        }

        read_chunk_count++;
    }

    if (write_offset != 0) {
        write(index_fp, (unsigned char *) write_buffer, write_offset*8);
    }

    lseek(index_fp, 0, SEEK_SET);
    write(index_fp, (unsigned char *) &write_count, 8);

    close(index_fp);
}

index_ll_dist index_get_index_count(int index_fp) {

    index_ll_dist count;
    pread(index_fp, (index_ll_dist *) &count, 8, 0);

    return count;
}

index_tl index_get_index(int index_fp, index_tl indx) {

    index_tl index;
    pread(index_fp, (index_tl *) &index, 8, index_get_index_offset(indx));
    return index;
}

/* Reads multiple indices from the index file, given the range of top level indices */
index_ll *index_get_index_bulk(int index_fp, index_tl from, index_tl to, index_tl_dist *restrict index_buffer_size) {
    *index_buffer_size = to - from + 1;

    NEW(index_ll, index, *index_buffer_size)
    pread(index_fp, (index_ll *) index, *index_buffer_size * 8, index_get_index_offset(from));
    return index;
}

/* reads the file content between one top level index and the next */
void *index_read_index(int fp_to_seek, int index_fp, index_tl indx, index_tl_dist *restrict buffer_size) {
    *buffer_size = index_get_size(index_fp, indx);   // cuts off the newline still included

    index_seek_in_tl_file(fp_to_seek, index_fp, indx);
    NEW(unsigned char, buffer, *buffer_size)
    read(fp_to_seek, buffer, *buffer_size);

    return buffer;
}

void *index_read_index_bulk(int fp_to_seek, int index_fp, index_tl from, index_tl to, index_tl_dist *restrict buffer_size) {
    *buffer_size = index_get_size_between(index_fp, from, to) - 1;

    index_seek_in_tl_file(fp_to_seek, index_fp, from);
    NEW(unsigned char, buffer, *buffer_size)

    CHARDIST total_bytes_read = 0, bytes_left = *buffer_size, to_read, br;

    if (*buffer_size <= INT_MAX) {
        read(fp_to_seek, buffer+total_bytes_read, *buffer_size);
        return buffer;
    }

    while (bytes_left > 0) {
        if (bytes_left > INT_MAX) {
            to_read = INT_MAX;
        } else {
            to_read = bytes_left;
        }

        br = read(fp_to_seek, buffer+total_bytes_read, to_read);

        total_bytes_read += br;
        bytes_left -= br;
    }

    return buffer;
}

/* Calculates the difference between one index and the next */
index_tl_dist index_get_size(int index_fp, index_tl indx) {
    off_t offset = index_get_index_offset(indx);

    index_ll index1, index2;

    pread(index_fp, (index_ll *) &index1, 8, offset);
    pread(index_fp, (index_ll *) &index2, 8, offset+8);

    return index2 - index1;
}

/* Calculates the difference between two top level indices with the given range */
index_tl_dist index_get_size_between(int index_fp, index_tl from, index_tl to) {
    index_ll index1, index2;
    pread(index_fp, (index_ll *) &index1, 8, index_get_index_offset(from));
    pread(index_fp, (index_ll *) &index2, 8, index_get_index_offset(to));
    return index2 - index1;
}

/* Seeks the given file pointer to the specified top level index using the index file */
void index_seek_in_tl_file(int fp_to_seek, int index_fp, index_tl indx) {
    lseek(fp_to_seek, index_get_index(index_fp, indx), SEEK_SET);
}
