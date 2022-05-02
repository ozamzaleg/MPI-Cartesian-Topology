#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "functions.h"
// check if the number of process that we enter is equal to the input from the file
void checkParameter(int K, int nprocs)
{

    if (nprocs != K * K)
    {
        printf("Please run with %d processes!", K * K);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
}

// this function read data from file data.txt and inform the variables
void readFromFile(InfoFromFile *infoFromFile, char **subString, char **strings)
{
    FILE *fp;
    int amount;

    if ((fp = fopen(FILE_NAME, "r")) == 0)
    {
        printf("cannot open file %s for reading\n", FILE_NAME);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    fscanf(fp, "%d %d %d", &infoFromFile->K, &infoFromFile->N, &infoFromFile->maxItertions);

    infoFromFile->sizeSubString = infoFromFile->N + 1;
    *subString = doMalloc(infoFromFile->sizeSubString);

    fscanf(fp, "%s", *subString);

    amount = (infoFromFile->K) * (infoFromFile->K);
    *strings = doMalloc(amount * ((infoFromFile->N * 2) + 1));

    if (strings == NULL)
    {
        printf("Problem to allocate memory\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    for (int i = 0; i < amount; i++)
        fscanf(fp, "%s", *strings + (i * (infoFromFile->N) * 2));

    fclose(fp);
}

// this function we want to create matrix that all process is in cell in the matrix and after return left right and down
MPI_Comm createMat(int K, char *string, int *up, int *down, int *right, int *left)
{
    int dim[2], period[2], reorder, rank;
    MPI_Comm comm;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    dim[0] = K;
    dim[1] = K;
    period[0] = 1;
    period[1] = 1;
    reorder = 1;

    MPI_Cart_create(MPI_COMM_WORLD, 2, dim, period, reorder, &comm);

    MPI_Cart_shift(comm, 1, 1, left, right);
    MPI_Cart_shift(comm, 0, 1, up, down);

    return comm;
}

// this function update the odd and even arrays every shuffle
void getEvenAndOddArrays(char *odd, char *even, int N, char *string)
{
    int oddIndex = 0;
    int evenIndex = 0;
    int i = 0;
    while (string[i] != '\0')
    {
        if (i % 2 == 0)
            odd[oddIndex++] = string[i];
        else
            even[evenIndex++] = string[i];
        i++;
    }
    odd[oddIndex] = even[evenIndex] = '\0';
}

// this function is allocate memory and return void array
void *doMalloc(unsigned int nbytes)
{
    void *p = malloc(nbytes);

    if (p == NULL)
    {
        fprintf(stderr, "malloc failed\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    return p;
}

// this function create new datatype (struct)
void createInfoFromFileType(MPI_Datatype *dataType)
{
    MPI_Type_contiguous(4, MPI_INT, dataType);
    MPI_Type_commit(dataType);
}
// this function return if we have substring
int checkIfExist(int *subArray, int nprocs)
{
    for (int i = 0; i < nprocs; i++)
    {
        if (subArray[i] == YES)

            return YES;
    }
    return NO;
}

// this function print the message if we have substring
void printExistResult(char *strings, char *subString, char *res, int nprocs, int N)
{
    char tempString[2 * N + 1];
    printf("The Substring is :'%s' is exist in string:'%s'\n", subString, res);
    for (int j = 0; j < nprocs; j++)
    {
        for (int k = 0; k < N * 2; k++)
        {
            tempString[k] = strings[j * N * 2 + k];
        }
        printf("rank %d have string %s\n", j, tempString);
    }
}

// this function suffle the matrix by the right neighbored and down neighbored send there odd and even array
char *getOddEvenFromNeighbor(char *odd, char *even, char *string, int N, int right, int left, int down, int up, MPI_Comm cart_comm)
{
    MPI_Send(odd, N, MPI_CHAR, left, WORK, cart_comm);
    MPI_Recv(string, N, MPI_CHAR, right, WORK, cart_comm, MPI_STATUS_IGNORE);
    MPI_Send(even, N, MPI_CHAR, up, WORK, cart_comm);
    MPI_Recv(string + N, N, MPI_CHAR, down, WORK, cart_comm, MPI_STATUS_IGNORE);

    return string;
}
void informMasterIfExist(int *found, int rank, char *string, MPI_Comm cart_comm, int N, char **res, char *subString, int *tag)
{
    if (strstr(string, subString) != NULL)
    {
        *found = YES;
        *tag = STOP;
        if (rank != MASTER)
            MPI_Send(string, N * 2, MPI_CHAR, MASTER, *tag, cart_comm);
        else
            strcpy(*res, string);
    }
}
int checkTag(int tag, char *res, int N, MPI_Comm cart_comm, MPI_Status status, char *strings, char *subString, int nprocs)
{
    if (tag != STOP)
    {
        MPI_Recv(res, N * 2, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, cart_comm, &status);
        tag = status.MPI_TAG;
    }
    if (tag == STOP)
        printExistResult(strings, subString, res, nprocs, N);
    return tag;
}
// check if theree substring in the string that came from right and down neighbored
void shuffleMatrix(char *strings, int maxIterations, char *odd, char *even, MPI_Comm cart_comm, char *string, int N, int right, int left, int down, int up, int rank, char *subString, char *res, int nprocs)
{
    int tag = WORK;
    int finishTask = 0;
    int found = NO;
    int isExist = NO;
    MPI_Status status;

    int *subArray = (int *)calloc(nprocs, sizeof(int));

    if (subArray == NULL)
    {
        fprintf(stderr, "malloc failed\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    while (finishTask < maxIterations && tag == WORK)
    {

        string = getOddEvenFromNeighbor(odd, even, string, N, right, left, down, up, cart_comm);

        informMasterIfExist(&found, rank, string, cart_comm, N, &res, subString, &tag);
        // i use this function for create array that the number of cell is the number of process
        MPI_Allgather(&found, 1, MPI_INT, subArray, 1, MPI_INT, cart_comm);

        isExist = checkIfExist(subArray, nprocs);

        if (isExist)
        {
            MPI_Gather(string, N * 2, MPI_CHAR, strings, N * 2, MPI_CHAR, MASTER, cart_comm);

            if (rank == MASTER)
                tag = checkTag(tag, res, N, cart_comm, status, strings, subString, nprocs);
            break;
        }
        getEvenAndOddArrays(odd, even, N, string);
        finishTask++;
    }

    if (rank == MASTER && tag == WORK)
        printf("The substring is not exist!\n");

    free(subArray);
}
