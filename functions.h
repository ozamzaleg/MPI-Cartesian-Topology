#ifndef __Functions_H__
#define __Functions_H__

#include <mpi.h>

#define FILE_NAME "data.txt"
#define PACK_SIZE 100

enum master
{
    MASTER
};
enum tags
{
    WORK,
    STOP
};
enum isExist
{
    NO,
    YES
};

typedef struct
{
    int K;
    int N;
    int maxItertions;
    int sizeSubString;
} InfoFromFile;

void checkParameter(int K, int nprocs);
void readFromFile(InfoFromFile *infoFromFile, char **subString, char **strings);
MPI_Comm createMat(int K, char *string, int *up, int *down, int *right, int *left);
void getEvenAndOddArrays(char *odd, char *even, int N, char *string);
void shuffleMatrix(char *strings, int maxIterations, char *odd, char *even, MPI_Comm cart_comm, char *string, int N, int right, int left, int down, int up, int rank, char *subString, char *res, int nprocs);
void *doMalloc(unsigned int nbytes);
void createInfoFromFileType(MPI_Datatype *dataType);
int checkIfExist(int *subArray, int nprocs);
void printExistResult(char *strings, char *subString, char *res, int nprocs, int N);
char *getOddEvenFromNeighbor(char *odd, char *even, char *string, int N, int right, int left, int down, int up, MPI_Comm cart_comm);
void informMasterIfExist(int *found, int rank, char *string, MPI_Comm cart_comm, int N, char **res, char *subString, int *TAG_WORK);
int checkTag(int TAG_WORK, char *res, int N, MPI_Comm cart_comm, MPI_Status status, char *strings, char *subString, int nprocs);

#endif
