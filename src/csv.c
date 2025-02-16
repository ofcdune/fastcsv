#include "../header/csv.h"

void csv_set_filename(struct csv_file *restrict csv_file, const char *restrict filename) {
    csv_file->csv.filename = filename;
    csv_file->csv.filename_is_set = true;
}

void csv_set_index_filename(struct csv_file *restrict csv_file, const char *restrict filename) {
    csv_file->index.filename = filename;
    csv_file->index.filename_is_set = true;

    stat(filename, NULL);
    if (errno != 2) {
        csv_file->is_indexed = true;
    }
}

bool csv_is_indexed(struct csv_file *restrict csv_file) {
    return csv_file->is_indexed;
}

void csv_open_csv_file(struct csv_file *restrict csv_file) {
    if (!csv_file->csv.filename_is_set) {
        fputs(".csv filename not set\n", stderr);
        exit(1);
    }

    if (csv_file->csv.fp_is_set) {
        return;
    }

    if (-1 == (csv_file->csv.fp = open(csv_file->csv.filename, O_RDONLY))) {
        fprintf(stderr, "Failed to open file '%s'\n", csv_file->csv.filename);
        printf("Error: %s\n", strerror(errno));
        exit(1);
    }

     csv_file->csv.fp_is_set = true;
}

void csv_close_csv_file(struct csv_file *restrict csv_file) {
    if (!csv_file->csv.fp_is_set) {
        return;
    }

    close(csv_file->csv.fp);
    csv_file->csv.fp_is_set = false;
    csv_file->csv.fp = 0;
}

void csv_open_index_file(struct csv_file *restrict csv_file) {
    if (!csv_file->index.filename_is_set) {
        fputs("Index filename not set\n", stderr);
        exit(1);
    }

    if (csv_file->index.fp_is_set) {
        return;
    }

    if (-1 == (csv_file->index.fp = open(csv_file->index.filename, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR))) {
        fprintf(stderr, "Failed to open index file '%s'\n", csv_file->index.filename);
        printf("Error: %s\n", strerror(errno));
        exit(1);
    }

    csv_file->index.fp_is_set = true;
}

void csv_close_index_file(struct csv_file *restrict csv_file) {
    if (!csv_file->index.fp_is_set) {
        return;
    }

    close(csv_file->index.fp);
    csv_file->index.fp_is_set = false;
    csv_file->index.fp = 0;
}

void csv_index_file(struct csv_file *restrict csv_file, char newline) {

    csv_open_csv_file(csv_file);
    csv_open_index_file(csv_file);

    index_create(csv_file->csv.fp, csv_file->index.fp, newline);
    csv_close_index_file(csv_file);

    csv_file->is_indexed = true;
}

struct csv_row_partial *csv_get_line_partial(struct csv_file *restrict csv_file, ROW line_index) {

    csv_open_index_file(csv_file);
    ROWDIST line_size = 0;
    unsigned char *line_buffer = index_read_index(csv_file->csv.fp, csv_file->index.fp, line_index, &line_size);
    csv_close_index_file(csv_file);

    NEWSTRUCT(struct csv_row_partial, rw)
    rw->row_string = (char *) line_buffer;
    rw->length = line_size;

    return rw;
}

struct csv_row *csv_get_title_row(struct csv_file *restrict csv_file, unsigned char sep) {
    struct csv_row_partial *title_row_partial = csv_get_line_partial(csv_file, 0);

    unsigned int i, sepcount = 1;
    for (i = 0; i < title_row_partial->length; i++) {
        if (title_row_partial->row_string[i] == sep) {
            sepcount++;
        }
    }

    struct csv_row *title_row = csv_parse_row(sepcount, sep, title_row_partial->row_string, title_row_partial->length);
    free(title_row_partial);

    return title_row;
}

struct csv_row *csv_parse_row(unsigned int count, unsigned char sep, char *restrict row_string, unsigned int string_len) {
    NEWSTRUCT(struct csv_row, row)

    row->count = count;
    row->sep = sep;

    NEWSTRUCTMEM(char *, row, strings, count)

    NEW(char, buffer, string_len)
    unsigned buffer_index = 0, string_index = 0, i;

    char *start = row_string;
    char *end = row_string;

    size_t len;

    while (*end && string_index < count) {

        if (*end != sep) {
            end++;
            continue;
        }

        len = end - start;
        if (len == 0) {
            string_index++;
            continue;
        }

        row->strings[string_index] = &buffer[buffer_index];
        memcpy(row->strings[string_index], start, len);
        buffer_index += len + 1;
        string_index++;

        start = ++end;
    }

    len = end - start;
    if (len > 0 && string_index < count) {
        row->strings[string_index] = &buffer[buffer_index];
        memcpy(row->strings[string_index], start, len);
    }

    return row;
}

void print_str(const char *buffer, uint64_t size) {
    uint64_t i;
    for (i = 0; i < size; i++) {
        printf("%02x ", buffer[i]);
    }
    putchar('\n');
}

struct csv_row *csv_get_rows_bulk(struct csv_file *restrict csv_file, struct csv_row *restrict title_row, ROW from, ROW to) {

    // get the index buffer size inside the index file
    // load all the indices into the index buffer

    csv_open_index_file(csv_file);

    // offset added to skip the title row
    from++;
    if (to == 0) {
        to = index_get_index_count(csv_file->index.fp);
    }

    CHARDIST index_buffer_size = 0;
    CHAR *index_bulk = index_get_index_bulk(csv_file->index.fp, from, to, &index_buffer_size);

    // load the giant string (that includes substrings) into a single buffer
    ROWDIST lines_buffer_size = 0;
    char *lines_buffer = index_read_index_bulk(csv_file->csv.fp, csv_file->index.fp, from, to, &lines_buffer_size);

    csv_close_index_file(csv_file);

    // parse each row (split the string at the indices stored in the buffer)
    ROW i;
    CHARDIST line_length;

    NEWSTRUCT(struct csv_row, ptr)
    struct csv_row *cur = ptr;

    for (i = 0; i < index_buffer_size - 1; i++) {
        line_length = index_bulk[i+1] - index_bulk[i];

        cur->next = csv_parse_row(title_row->count, title_row->sep, lines_buffer, line_length);
        cur = cur->next;

        lines_buffer += line_length;
    }

    cur = ptr->next;

    free(ptr);

    return cur;
}

struct csv_row *csv_get_row(struct csv_file *restrict csv_file, struct csv_row *restrict title_row, ROW line_index) {
    // the offset is added to compensate for the title row
    line_index++;
    if (line_index > csv_rowcount(csv_file)) {
        return NULL;
    }

    struct csv_row_partial *partial = csv_get_line_partial(csv_file, line_index);
    struct csv_row *row = csv_parse_row(title_row->count, title_row->sep, partial->row_string, partial->length);
    free(partial);

    return row;
}

/* finds the string in the title row and returns its index */
int csv_string_to_index(struct csv_row *restrict title_row, char *string) {
    int i;
    for (i = 0; i < title_row->count; i++) {
        if (strcmp(title_row->strings[i], string) == 0) {
            return i;
        }
    }

    return -1;
}

void csv_print_rows(struct csv_row *rows, int rowindex) {

    struct csv_row *ptr = rows;
    while (ptr->next != NULL) {
        printf("%s\n", ptr->strings[rowindex]);
        ptr = ptr->next;
    }

    printf("%s\n", ptr->strings[rowindex]);
}

ROWDIST csv_rowcount(struct csv_file *restrict csv_file) {
    csv_open_index_file(csv_file);

    // subtract one to not include the title row
    return index_get_index_count(csv_file->index.fp) - 1;
}
