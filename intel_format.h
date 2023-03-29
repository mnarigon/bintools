//
// -- intel_format.h
//
#ifndef INTEL_FORMAT_H
#define INTEL_FORMAT_H

#include "types.h"

//
// -- write an Intel hex format data record
// -- fp     - output file pointer
// -- offset - load offset
// -- binbuf - binary data
// -- binlen - number of binary data bytes
void
write_data_record(FILE *fp,
                  address_type offset,
                  const byte_type *binbuf, int binlen);

//
// -- write an Intel hex format extended linear address record
// -- fp     - output file pointer
// -- ulba   - upper linear base address
void
write_ela_record(FILE *fp, address_type ulba);

//
// -- write an Intel hex format end of file record
// -- fp     - output file pointer
// -- start  - start address
void
write_eof_record(FILE *fp, address_type start);

//
// -- parse an Intel hex format record
// -- hexbuf    - pointer to ASCII data to be parsed
// -- hexsize   - size of the hex buffer
// -- binbuf    - pointer to parsed binary data
// -- binsize   - size of the binary buffer in bytes
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
             int *p_binlen);

#endif
