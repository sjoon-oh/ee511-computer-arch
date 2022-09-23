rm krpassembler
rm output_log
rm vectors.bin
gcc krpassembler.c -o krpassembler
./krpassembler ./test_codes/test1_asm
