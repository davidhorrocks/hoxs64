// p64ref.cpp : Defines the entry point for the console application.
//
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include "p64.h"


int demo_tmain(int argc, _TCHAR* argv[])
{
    TP64MemoryStream P64MemoryStreamInstance;
    TP64Image P64Image;
    FILE *pFile;
    uint32_t lSize;
    uint8_t *buffer;
    P64MemoryStreamCreate(&P64MemoryStreamInstance);
    P64ImageCreate(&P64Image);
    pFile = fopen("input.p64", "rb");
    if (pFile)
    {
        fseek(pFile , 0 , SEEK_END);
        lSize = ftell(pFile);
        rewind(pFile);
        buffer = (uint8_t*) malloc(sizeof(uint8_t) * lSize);
        fread(buffer, 1, lSize, pFile);
        fclose(pFile);
        P64MemoryStreamWrite(&P64MemoryStreamInstance,buffer,lSize);
        P64MemoryStreamSeek(&P64MemoryStreamInstance,0);
        if (P64ImageReadFromStream(&P64Image,&P64MemoryStreamInstance, 0))
        {
            printf("Read ok!\n");
            P64MemoryStreamClear(&P64MemoryStreamInstance);
            if (P64ImageWriteToStream(&P64Image,&P64MemoryStreamInstance))
            {
                printf("Write ok!\n");
                pFile = fopen("output.p64", "wb");
                fwrite(P64MemoryStreamInstance.Data, 1, P64MemoryStreamInstance.Size, pFile);
                fclose(pFile);
            }
            else
            {
                printf("Write failed!\n");
            }
        }
        else
        {
            printf("Read failed!\n");
        }
    }
    P64MemoryStreamDestroy(&P64MemoryStreamInstance);
    P64ImageDestroy(&P64Image);
    printf("Hello world!\n");
    return 0;
}

