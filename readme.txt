bin2hex - convert binary file to Intel hexadecimal object file format
hex2bin - convert Intel hexadecimal object file to binary file format

usage: bin2hex [options] [file]
  convert binary file to Intel hexadecimal object file format
  reads from file (or stdin if file not given on command line)
  writes to stdout (or file specified by the -o option)
options:
  -a|--address address: starting address (default 0)
  -o|--output file: output

usage: hex2bin [options] [file]
  convert Intel hexadecimal object file to binary file format
  reads from file (or stdin if file not given on command line)
  writes to stdout (or file specified by the -o option)
option:
  -o|--output file: output
