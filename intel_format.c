//
// -- intel_format.c
//
#include "intel_format.h"

#include <assert.h>

// -- Intel format record types
static const byte_type k_data_record_type = 0;
static const byte_type k_eof_record_type = 1;
static const byte_type k_esa_record_type = 2;
static const byte_type k_ssa_record_type = 3;
static const byte_type k_ela_record_type = 4;
static const byte_type k_sla_record_type = 5;

// -- header length
static const int k_hdrlen = 4;

//
// -- helpers
//

static byte_type
high_byte(word_type value) {
    return (byte_type) (value / 256);
}

static byte_type
low_byte(word_type value) {
    return (byte_type) (value % 256);
}

// -- convert an ASCII hexadecimal char to its value
static byte_type
char_to_uint8(char ch) {
    if (ch >= '0' && ch <= '9') {
        return ch - '0';
    } else if (ch >= 'a' && ch <= 'f') {
        return ch - 'a' + 10;
    } else if (ch >= 'A' && ch <= 'F') {
        return ch - 'A' + 10;
    } else {
        return 0;
    }
}

// -- convert two ASCII hexadecimal characters to their value
static byte_type
chars_to_uint8(const char *record) {
    uint32_t result;
    result  = char_to_uint8(record[0]) << 4;
    result += char_to_uint8(record[1]);
    return (byte_type) result;
}

// -- convert four ASCII hexadecimal characters to their value
static word_type
chars_to_uint16(const char *record) {
    uint32_t result;
    result  = char_to_uint8(record[0]) << 12;
    result += char_to_uint8(record[1]) << 8;
    result += char_to_uint8(record[2]) << 4;
    result += char_to_uint8(record[3]);
    return (word_type) result;
}

// -- convert eight ASCII hexadecimal characters to their value
static uint32_t
chars_to_uint32(const char *record) {
    uint32_t result;
    result  = char_to_uint8(record[0]) << 28;
    result += char_to_uint8(record[1]) << 24;
    result += char_to_uint8(record[2]) << 20;
    result += char_to_uint8(record[3]) << 16;
    result += char_to_uint8(record[4]) << 12;
    result += char_to_uint8(record[5]) << 8;
    result += char_to_uint8(record[6]) << 4;
    result += char_to_uint8(record[7]);
    return result;
}

//
// -- generate the Intel hex format checksum
// -- checksum - initial checksum
// -- binbuf   - binary data
// -- binlen   - number of binary data bytes
static byte_type
gen_checksum(byte_type checksum,
             const byte_type *binbuf,
             int binlen) {
    assert(binbuf && "null buffer");
    for (int i = 0; i < binlen; ++i) {
        checksum += binbuf[i];
    }
    return checksum;
}

//
// -- public functions
//

//
// -- write an Intel hex format data record
// -- fp     - output file pointer
// -- offset - load offset
// -- binbuf - binary data
// -- binlen - number of binary data bytes
void
write_data_record(FILE *fp,
                  address_type offset,
                  const byte_type *binbuf,
                  int binlen) {
    assert(fp && "null file pointer");
    assert(binlen > 0 && binlen < 256 && "binlen out of range");

    byte_type hdrbuf[k_hdrlen];
    // -- set record length
    hdrbuf[0] = (byte_type) binlen;
    // -- set load offset
    hdrbuf[1] = high_byte(offset);
    hdrbuf[2] = low_byte(offset);
    // -- set the record type
    hdrbuf[3] = k_data_record_type;

    // -- generate the checksum
    byte_type checksum = gen_checksum(0, hdrbuf, k_hdrlen);
    checksum = -gen_checksum(checksum, binbuf, binlen);

    // -- and output
    fprintf(fp, ":");
    for (int i = 0; i < k_hdrlen; ++i) {
        fprintf(fp, "%02X", hdrbuf[i] & 0xFF);
    }
    for (int i = 0; i < binlen; ++i) {
        fprintf(fp, "%02X", binbuf[i] & 0xFF);
    }
    fprintf(fp, "%02X\n", checksum & 0xFF);
}

//
// -- write an Intel hex format extended linear address record
// -- fp     - output file pointer
// -- ulba   - upper linear base address
void
write_ela_record(FILE *fp, address_type ulba) {
    assert(fp && "null file pointer");
    static const int k_ulbalen = 2;

    byte_type hdrbuf[k_hdrlen + k_ulbalen];
    // -- set record length
    hdrbuf[0] = (byte_type) k_ulbalen;
    // -- set load offset
    hdrbuf[1] = high_byte(0);
    hdrbuf[2] = low_byte(0);
    // -- set the record type
    hdrbuf[3] = k_ela_record_type;
    // -- ulba
    hdrbuf[4] = high_byte(ulba);
    hdrbuf[5] = low_byte(ulba);

    // -- generate the checksum
    byte_type checksum = -gen_checksum(0, hdrbuf, k_hdrlen + k_ulbalen);

    // -- and output
    fprintf(fp, ":");
    for (int i = 0; i < k_hdrlen + k_ulbalen; ++i) {
        fprintf(fp, "%02X", hdrbuf[i] & 0xFF);
    }
    fprintf(fp, "%02X\n", checksum & 0xFF);
}

//
// -- write an Intel hex format end of file record
// -- fp     - output file pointer
// -- start  - start address
void
write_eof_record(FILE *fp, address_type start) {
    assert(fp && "null file pointer");
    byte_type hdrbuf[k_hdrlen];
    // -- set record length
    hdrbuf[0] = (byte_type) 0;
    // -- set load offset
    hdrbuf[1] = high_byte(start);
    hdrbuf[2] = low_byte(start);
    // -- set the record type
    hdrbuf[3] = k_eof_record_type;

    // -- generate the checksum
    byte_type checksum = -gen_checksum(0, hdrbuf, k_hdrlen);

    // -- and output
    fprintf(fp, ":");
    for (int i = 0; i < k_hdrlen; ++i) {
        fprintf(fp, "%02X", hdrbuf[i] & 0xFF);
    }
    fprintf(fp, "%02X\n", checksum & 0xFF);
}

//
// -- parse an Intel hex format record
// -- hexbuf    - pointer to ASCII data to be parsed
// -- hexsize   - size of the hex buffer
// -- binbuf    - pointer to parsed binary data
// -- binsize   - size of the binary buffer in bytes
// -- p_address - pointer for the
// --               returned load offset in a data record
// --               returned start address in an end of file record
// --               returned upper linear base address in an extended linear
// --                 address record
// -- p_binlen  - pointer for the returned number of binary data bytes
//
// -- return -3 if the buffer is not large enough to receive the binary data
// -- return -2 if an invalid record format
// -- return -1 if the record is empty or doesn't start with a record mark
// -- return  0 if a valid data record
// -- return  1 if a valid end of file record
// -- return  2 if a valid extended linear address record
// -- return  3 if a valid yet ignorable record
int
parse_record(const char *hexbuf, int hexsize,
             byte_type *binbuf, int binsize,
             address_type *p_address,
             int *p_binlen) {
    assert(hexbuf && "null hexbuf pointer");

    // -- 16-bit and 32-bit record type data
    uint16_t ulba = 0;
    uint16_t usba = 0;
    uint32_t eip = 0;
    uint32_t cs_ip = 0;

    // -- record is empty or doesn't start with a record mark
    if (hexsize < 1 || hexbuf[0] != ':') return -1;

    // -- record too small to be valid
    if (hexsize < 11) return -2;

    // -- common fields
    byte_type reclen = chars_to_uint8(&hexbuf[1]);
    address_type offset = chars_to_uint16(&hexbuf[3]);
    byte_type rectyp = chars_to_uint8(&hexbuf[7]);

    // -- validate record length
    // -- mark, header, data, checksum, newline
    int len = 1 + 2*4 + 2*reclen + 2 + 1;
    if (hexsize < len) return -2;

    // -- validate end of line
    if (hexbuf[len-1] != '\n' &&
        hexbuf[len-1] != '\r' ) return -2;

    // -- validate checksum
    byte_type checksum = 0;
    for (int i = 0; i < reclen + 5; ++i) {
        checksum += chars_to_uint8(&hexbuf[1+2*i]);
    }
    if (checksum != 0) return -2;

    // -- process information/data
    switch (rectyp) {
        case k_data_record_type:
            // -- data record
            assert(binbuf && "null binbuf pointer");
            assert(p_address && "null address pointer");
            assert(p_binlen && "null reclen pointer");

            // -- buffer isn't large enough
            if (binsize < reclen) return -3;

            *p_address = offset;
            for (int i = 0; i < reclen; ++i) {
                binbuf[i] = chars_to_uint8(&hexbuf[9+2*i]);
            }
            *p_binlen = reclen;
            return 0;

        case k_eof_record_type:
            // -- end of file record
            assert(p_address && "null address pointer");

            if (reclen != 0) return -2;
            *p_address = offset;
            return 1;

        case k_esa_record_type:
            // -- extended segment address record
            if (reclen != 2) return -1;
            if (offset != 0) return -1;
            usba = chars_to_uint16(&hexbuf[9]);
            return 3;

        case k_ssa_record_type:
            // -- start segment address record
            if (reclen != 4) return -1;
            if (offset != 0) return -1;
            cs_ip = chars_to_uint32(&hexbuf[9]);
            return 3;

        case k_ela_record_type:
            // -- extended linear address record
            assert(p_address && "null address pointer");

            if (reclen != 2) return -1;
            if (offset != 0) return -1;
            ulba = chars_to_uint16(&hexbuf[9]);
            *p_address = ulba;
            return 2;

        case k_sla_record_type:
            // -- start linear address record
            if (reclen != 4) return -1;
            if (offset != 0) return -1;
            eip = chars_to_uint32(&hexbuf[9]);
            return 3;

        default:
            // -- illegal record type
            return -2;
    }
}
