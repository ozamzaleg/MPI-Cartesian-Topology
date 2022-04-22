#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "functions.h"

int main(int argc, char *argv[])
{

    char *res;
    char *subString;
    char *string;
    char *strings;
    char *even;
    char *odd;
    int nprocs, rank;
    int left, right, up, down;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm comm;

    InfoFromFile infoFromFile;
    MPI_Datatype InfoFromFileMPIType;
    createInfoFromFileType(&InfoFromFileMPIType);

    if (rank == MASTER)
    {
        readFromFile(&infoFromFile, &subString, &strings);
        checkParameter(infoFromFile.K, nprocs);
    }

    MPI_Bcast(&infoFromFile, 1, InfoFromFileMPIType, MASTER, MPI_COMM_WORLD);

    if (rank != MASTER)
        subString = doMalloc(infoFromFile.N + 1);

    MPI_Bcast(subString, infoFromFile.sizeSubString, MPI_CHAR, MASTER, MPI_COMM_WORLD);

    string = doMalloc(infoFromFile.N * 2 + 1);

    MPI_Scatter(strings, infoFromFile.N * 2, MPI_CHAR, string, infoFromFile.N * 2, MPI_CHAR, MASTER, MPI_COMM_WORLD);

    res = doMalloc(infoFromFile.N * 2 + 1);
    odd = doMalloc(infoFromFile.N + 1);
    even = doMalloc(infoFromFile.N + 1);

    getEvenAndOddArrays(odd, even, infoFromFile.N, string);

    MPI_Barrier(MPI_COMM_WORLD);

    comm = createMat(infoFromFile.K, string, &up, &down, &right, &left);

    shuffleMatrix(strings, infoFromFile.maxItertions, odd, even, comm, string, infoFromFile.N, right, left, down, up, rank, subString, res, nprocs);

    if (rank == MASTER)
        free(strings);

    free(res);
    free(odd);
    free(even);
    free(string);
    free(subString);

    MPI_Finalize();
    return 0;
}
