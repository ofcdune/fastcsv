#ifndef WATER_CSV_H
#define WATER_CSV_H

#include "library.h"

#define ROW unsigned long long  // top level index
#define CHAR unsigned long long  // low level index

#define ROWDIST unsigned long long
#define CHARDIST unsigned long long

#define STRTOINDEX(string, title_row) \
    int string = csv_string_to_index(title_row, #string)

struct csv_file_partial {
    bool filename_is_set;
    bool fp_is_set;

    const char *filename;
    int fp;
};

struct csv_file {
    struct csv_file_partial csv;
    struct csv_file_partial index;
    bool is_indexed;
};

struct csv_row_partial {
    char *row_string;
    unsigned int length;
};

struct csv_row {
    char **strings;
    unsigned int count;
    unsigned char sep;
    struct csv_row *next;
};

void csv_set_filename(struct csv_file *restrict csv_file, const char *restrict filename);
void csv_set_index_filename(struct csv_file *restrict csv_file, const char *restrict new_fn);

void csv_open_csv_file(struct csv_file *restrict csv_file);
void csv_close_csv_file(struct csv_file *restrict csv_file);

bool csv_is_indexed(struct csv_file *restrict csv_file);

void csv_index_file(struct csv_file *restrict csv_file, char newline);
struct csv_row *csv_parse_row(unsigned int count, unsigned char sep, char *row_string, unsigned int string_len);

struct csv_row *csv_get_title_row(struct csv_file *restrict csv_file, unsigned char sep);
struct csv_row *csv_get_row(struct csv_file *csv_file, struct csv_row *title_row, ROW line_index);

struct csv_row *csv_get_rows_bulk(struct csv_file *csv_file, struct csv_row *title_row, ROW from, ROW to);
int csv_string_to_index(struct csv_row *restrict title_row, char *string);

void csv_print_rows(struct csv_row *rows, int rowindex);
ROWDIST csv_rowcount(struct csv_file *restrict csv_file);

#endif
