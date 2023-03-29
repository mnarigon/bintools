//
// -- bin2hex.c
//
#include <ctype.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>

#include "intel_format.h"

// -- default number of output bytes per record
static const int k_bytes_per_record = 32;

// -- print usage message
static void
usage() {
    fprintf(stderr,
            "usage: bin2hex [options] [file]\n"
            "  convert binary file to Intel hexadecimal object file format\n"
            "  reads from file (or stdin if file not given on command line)\n"
            "  writes to stdout (or file specified by the -o option)\n"
            "options:\n"
            "  -a|--address address: starting address (default 0)\n"
            "  -o|--output file: output\n");
    exit(EXIT_FAILURE);
}

// -- program options
static char *short_options = "a:o:";
static struct option long_options[] = {
    {"address", required_argument, 0, 'a'},
    {"output",  required_argument, 0, 'o'},
    {0, 0, 0, 0}
};

static FILE *in_fp;
static FILE *out_fp;

// -- record data
static byte_type data[k_bytes_per_record];
static address_type offset;

// -- default start address
static address_type start_address;

//
// -- scan for an address
static address_type
strtoaddr(const char *str) {
    const char *ptr = str;
    unsigned int radix = 10;
    // -- advance past hexadecimal digits
    while (isxdigit(*ptr)) {
        ++ptr;
    }
    // -- test for radix tag
    if (toupper(*ptr) == 'O' || toupper(*ptr) == 'Q') {
        radix = 8;
    } else if (toupper(*ptr) == 'H') {
        radix = 16;
    } else {
        --ptr;
        if (toupper(*ptr) == 'B') {
            radix = 2;
        } else if (toupper(*ptr) == 'D') {
            radix = 10;
        } else {
            ++ptr;
        }
    }
    
    unsigned int value = 0;
    while (str < ptr) {
        int ch = toupper(*str++);
        unsigned int digit = (ch >= 'A') ? (ch - 'A' + 10) : (ch - '0');
        value = value * radix + digit;
        if (digit >= radix || value > 0xFFFF) {
            fprintf(stderr, "invalid address\n");
            exit(EXIT_FAILURE);
        }
    }
    return (address_type) value;
}

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
            case 'a':
                // -- start address
                start_address = strtoaddr(optarg);
                break;
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
    // -- generate extended linear address record
    write_ela_record(out_fp, 0x0000);
    // -- check output file status
    if (ferror(out_fp)) {
        perror("write");
        exit(EXIT_FAILURE);
    }
    
    //
    // -- process input file to retrieve binary data
    int read_count;
    offset = start_address;
    while ((read_count = (int) fread(data, 1, k_bytes_per_record, in_fp)) > 0) {
        if (ferror(in_fp)) {
            perror("read");
            exit(EXIT_FAILURE);
        }

        // -- generate data record
        write_data_record(out_fp, offset, data, read_count);
        // -- check output file status
        if (ferror(out_fp)) {
            perror("write");
            exit(EXIT_FAILURE);
        }
        
        // -- increment offset
        offset += read_count;
    }
    if (ferror(in_fp)) {
        perror("read");
        exit(EXIT_FAILURE);
    }

    //
    // -- mark end of file
    write_eof_record(out_fp, 0x0000);
    // -- check output file status
    if (ferror(out_fp)) {
        perror("write");
        exit(EXIT_FAILURE);
    }

    //
    // -- close and return
    fclose(in_fp);
    fclose(out_fp);
    return EXIT_SUCCESS;
}
