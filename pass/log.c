#include <stdio.h>
#include <stdlib.h>

void bbStartLogger(char* FileName, size_t *BbAddr) {
    FILE *DumpFile = fopen(FileName, "a");
    fprintf(DumpFile, "%p\n", BbAddr);
    fclose(DumpFile);
}

void binOpLogger(char* FileName, size_t *InstrAddr, int Result) {
    FILE *DumpFile = fopen(FileName, "a");
    fprintf(DumpFile, "%p %d\n", InstrAddr, Result);
    fclose(DumpFile);
}
