#include <stdio.h>
#include <stdlib.h>

void bbStartLogger(char* FileName, size_t *BbAddr) {
    FILE *DumpFile = fopen(FileName, "a");
    fprintf(DumpFile, "exec %p\n", BbAddr);
    fclose(DumpFile);
}

void binOpLogger(char* FileName, size_t *BbAddr, size_t *InstrAddr, int Result) {
    FILE *DumpFile = fopen(FileName, "a");
    fprintf(DumpFile, "binop %p %p %d\n", BbAddr, InstrAddr, Result);
    fclose(DumpFile);
}
