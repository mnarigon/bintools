//
// -- hex2bin.c
//
#include <getopt.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "intel_format.h"

// -- maximum data
static const int k_max_data = 256;
// -- maximum memory
static const int k_max_memory = 65536;
// -- maximum warnings
static const int k_max_warnings = 10;

// -- print usage message
static void
usage() {
    fprintf(stderr,
            "usage: hex2bin [options] [file]\n"
            "  convert Intel hexadecimal object file to binary file format\n"
            "  reads from file (or stdin if file not given on command line)\n"
            "  writes to stdout (or file specified by the -o option)\n"
            "option:\n"
            "  -o|--output file: output\n");
}

// -- program options
static char *short_options = "o:";
static struct option long_options[] = {
    {"output",  required_argument, 0, 'o'},
    {0, 0, 0, 0}
};

static FILE *in_fp;
static FILE *out_fp;

// -- input line buffer
static char *line;
static size_t line_size;

// -- record data buffer
static byte_type data[k_max_data];
static address_type offset;
static int reclen;

// -- binary memory space
static byte_type memory[k_max_memory];
static address_type start_address = 0xffff;
static address_type end_address = 0x0000;

static int warning_count;

//
// -- main program
int
main(int argc, char *argv[]) {
    out_fp = stdout;
    // -- process command line arguments
    int option_index = 0;
    int ch;
    while ((ch = getopt_long(argc, argv,
                             short_options, long_options,
                             &option_index)) != -1) {
        switch (ch) {
            case 'o':
                // -- output
                out_fp = fopen(optarg, "w");
                if (!out_fp) {
                    perror("open");
                    exit(EXIT_FAILURE);
                }
                break;
            default:
                usage();
        }
    }
    argv += optind;
    
    // -- open input file
    char *path = argv[0];
    if (!path || strcmp(path, "-") == 0) {
        in_fp = stdin;
    } else {
        in_fp = fopen(path, "r");
        if (!in_fp) {
            perror("open");
            exit(EXIT_FAILURE);
        }
    }
    
    //
    // -- parse the input to retrieve binary data
    int read_count;
    int line_count = 0;
    while ((read_count = (int) getline(&line, &line_size, in_fp)) > 0) {
        ++line_count;

        // -- parse a record
        // -- return -3 if the buffer is not large enough to receive the binary data
        // -- return -2 if an invalid record format
        // -- return -1 if the record is empty or doesn't start with a record mark
        // -- return  0 if a valid data record
        // -- return  1 if a valid end of file record
        // -- return  2 if a valid extended linear address record
        // -- return  3 if a valid yet ignorable record
        int status = parse_record(line, read_count,
                                  data, k_max_data,
                                  &offset,
                                  &reclen);

        if (status == -3) {
            fprintf(stderr, "line %d: line too long\n", line_count);
            exit(EXIT_FAILURE);
        } else if (status == -2 || status == -1) {
            ++warning_count;
            if (warning_count < k_max_warnings) {
                fprintf(stderr, "line %d: invalid record format\n", line_count);
            } else if (warning_count == k_max_warnings) {
                fprintf(stderr, "line %d: too many warnings, will no longer report\n", line_count);
            }
        } else if (status == 0) {
            // -- data record
            // -- store data into memory buffer
            for (int i = 0; i < reclen; ++i) {
                memory[offset + i] = data[i];
            }
            // -- adjust memory bounds
            if (start_address > offset) start_address = offset;
            if (end_address < offset + reclen) end_address = offset + reclen;
        } else if (status == 1) {
            // -- eof record
            // -- exit loop
            break;
        }
        // -- otherwise skip line
    }
    // -- check for read errors
    if (ferror(in_fp)) {
        perror("read");
        exit(EXIT_FAILURE);
    }
    
    //
    // -- write the binary data
    int memory_size = end_address - start_address;
    int write_count = (int) fwrite(memory + start_address,
                                   1, memory_size, out_fp);
    if (write_count != memory_size) {
        perror("write");
        exit(EXIT_FAILURE);
    }

    // -- close and return
    fclose(in_fp);
    fclose(out_fp);
    return EXIT_SUCCESS;
}
