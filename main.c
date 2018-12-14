//
// Chicago Crime Lookup, using Hashing
//
// Cody Hasty
// chasty2
// 66068521
// Written in Visual Studio Code on Ubuntu 16.04 LTS
// U. of Illinois, Chicago
// CS251, Spring 2017
// Project #05 
//

// ignore stdlib warnings if working in Visual Studio:
#define _CRT_SECURE_NO_WARNINGS 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

//booleans
#define _TRUE  1
#define _FALSE 0

//declare crime report struct and HTKey
typedef char* HTKey;

typedef struct crimeReport
{
    struct crimeReport* next;      //for chaining
    int area;
    char* areaName;
    char* caseNumber;
    char* IUCR;
    char* dateTime;
    char arrest[6];
} crimeReport;

//
// getFileName: 
//
// Inputs a filename from the keyboard, make sure the file can be
// opened, and returns the filename if so.  If the file cannot be 
// opened, an error message is output and the program is exited.
//
char *getFileName()
{
  char filename[512];
  int  fnsize = sizeof(filename) / sizeof(filename[0]);

  // input filename from the keyboard:
  fgets(filename, fnsize, stdin);
  filename[strcspn(filename, "\r\n")] = '\0';  // strip EOL char(s):

  // make sure filename exists and can be opened:
  FILE *infile = fopen(filename, "r");
  if (infile == NULL)
  {
    printf("**Error: unable to open '%s'\n\n", filename);
    exit(-1);
  }

  fclose(infile);

  // duplicate and return filename:
  char *s = (char *)malloc((strlen(filename) + 1) * sizeof(char));
  strcpy(s, filename);

  return s;
}

//
// getFileSizeInBytes was given to us in the project description.
// it takes a file as its parameter and returns the size in bytes
//
long getFileSizeInBytes(char *filename)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL)
        return -1;

    fseek(file, 0, SEEK_END);
    long size = ftell(file);

    fclose(file);

    return size;
}

//
// hashErrorCheck is called by hash() to verify that the input case # is valid
//
int hashErrorCheck(HTKey key)
{
    int check = _FALSE;
    //error check: key is 8 characters
    if (strlen(key) != 8)
    {
        printf("** invalid case #, try again...\n\n");
        return check;
    }

    //error check: key begins with 'R'
    if (key[0] != 'R')
    {
        printf("** invalid case #, try again...\n\n");
        return check;
    }

    //retrieve area
    char *areaPtr = &key[1];
    int area = atoi(areaPtr);
    //error check: area is between 0-77
    if (area < 0 || area > 77)
    {
        printf("** invalid case #, try again...\n\n");
        return check;
    }

    //if we got here, input is valid
    check = _TRUE;
    return check; 
}

//
// hash takes a case number (string) as a parameter, and returns an index
// corresponding to it's location in the hash table.
//
long hash (HTKey key, long N)
{  
    //declare variables
    long index = 0;
    //retrieve area
    char* areaPtr = &key[1];
    int area = atoi(areaPtr);
    //allocate 260000 indicies per area
    index += ((260000)*area);
    //retrieve letter at key[3], allocate 10000 indicies per letter
    int letterTwo = (int)key[3];
    letterTwo = letterTwo - 'A';
    index += (letterTwo*(10000));
    //add last four digits to index
    areaPtr = &key[4];
    int lastFour = atoi(areaPtr);
    index += lastFour;
    //take abs of index and mod by size (unnecessary, but a precaution)
    index = abs(index);
    index %= N;
    //return index
    return index;  
}

//
// search: Once the hashTable is built, the search function inputs a 
// case number. The case number is error checked, then the hash index
// is determined and the value is searched for in the hashtable.
//
void search(crimeReport** hashTable, long N, char* input)
{
    //error check
    if (!hashErrorCheck(input))
        return;
    else 
    {
        long index = hash(input, N);
        //search HT by traversing LL at HT[index]
        crimeReport *cur = hashTable[index];
        printf(">> hash index: %ld <<\n", index);
        while(cur != NULL)
        {
            if(strcmp(cur->caseNumber, input) == 0)
            {
                //found, print information
                printf("%s:\n", cur->caseNumber);
                printf("   date/time: %s\n", cur->dateTime);
                printf("   city area: %d => %s\n", cur->area, cur->areaName);
                printf("   IUCR code: %s\n", cur->IUCR);
                printf("   arrested:  %s\n\n", cur->arrest);
                return;
            }
            else
                cur = cur->next;
        }
    }
    //report not found if this point is reached
    printf("** Case not found...\n");
    return;
}



//
// store: this function is called by buildHashTable to insert 
// values read in from Crimes.csv to a crimeReport struct. hash()
// is then called to generate a location in the hashTable to be 
// stored. Returns collision count
//
int store(crimeReport** hashTable, char* inputCaseNumber, char* inputIUCR,
            char* inputDateTime, char* inputArrest, char** areas, long N, int colCount)
{
    //malloc struct
    crimeReport* insertReport = (crimeReport*)malloc(sizeof(crimeReport));

    //retrieve area
    char *areaPtr = &inputCaseNumber[1];
    int insertArea = atoi(areaPtr);

    //insert values
    insertReport->area = insertArea;
    insertReport->areaName = areas[insertArea];
    insertReport->caseNumber = inputCaseNumber;
    insertReport->IUCR = inputIUCR;
    insertReport->dateTime = inputDateTime;
    insertReport->next = NULL;
    //change arrest from T/F to true/false
    if(strcmp(inputArrest,"T") == 0)
        strcpy(insertReport->arrest, "true");
    else
        strcpy(insertReport->arrest, "false");

    //hash case number
    long index = hash(insertReport->caseNumber, N);

    //insert crimeReport
    if (hashTable[index] == NULL)
        hashTable[index] = insertReport;
    //collision, chain values
    else
    {
        crimeReport* cur = hashTable[index];
        crimeReport* prev = NULL;
        while(cur != NULL)
        {
            prev = cur;
            cur = cur->next;
        }
        //reached end of struct LL, insert crimeReport
        prev->next = insertReport;
        colCount++;
    }
    //insert successful, return collision count
    return colCount;
}

//
// buildAreas: this function reads in values from Areas.csv to create
// an array of strings, where the index of each element corresponds to
// the number of a particular area. This will be used to fill in 
// crime reports
//
char **buildAreas(char *areasFile)
{
    //open file
    FILE* infile = fopen(areasFile, "r");
    //declare variables for reading in values
    char  line[512];
    int   linesize = sizeof(line) / sizeof(line[0]);
    char *token;
    int i = 0;
    int j = 0;
    //declare areas Array and initialize it
    char** areas = (char**)malloc(78 * sizeof(char*));
    for(j = 0; j < 78; j++)
    {
        areas[j] = NULL;
    }

    //first fgets, ignore first line
    fgets(line, linesize, infile);

    //while loop reads in values to areas array
    fgets(line, linesize, infile);
    while(!feof(infile))
    {
        line[strcspn(line, "\r\n")] = '\0'; //strip EOL char(s)

        //format: number (ignore), area
        //store area name in areas array
        token = strtok(line, ",");
        token = strtok( NULL, ",");
        assert(token != NULL);
        areas[i] = (char*)malloc((strlen(token) + 1)*sizeof(char));
        strcpy(areas[i], token);

        //increment i and get next line
        i++;
        fgets(line, linesize, infile);
    }
    fclose(infile);
    //return areas array
    return areas;
}

//
// mallocHT allocates memory for the hash table, and sets all values
// to null
//
crimeReport **mallocHT(long N)
{
     //create array of pointers to crimeReports. Ensure malloc worked
    crimeReport **hashTable = (crimeReport**)malloc(N * sizeof(crimeReport*));

    if (hashTable == NULL)
    {
        printf("** Error: malloc failed to allocate hash table (%ld elements).\n", N);
        exit(-1);
    }
    //set HT values to null
    int i = 0;
    for(i = 0; i < N; i++);
    {
        hashTable[i] = NULL;
    }
    //return HT
    return hashTable;
}

//
// buildHashTable: the magnum opus of this project. This function takes
// in size N, Crimes.csv and Areas.csv as parameters. 
//
// 1) mallocHT is called to create an array of struct pointers is 
//    created and initialized to null with size based off of the size of 
//    Crimes.csv ((filesize/50) * 5). there's about 50 bytes per crime.
//    Goal is 20% load value). 
//
// 2) buildHashTable calls buildAreas, which will return an array to be 
//    used to fill in crimeReports 
//
// 3) Next, crimeReport values are read in and stored into the table
//    with a call to store(). Number of crimes and collisions are counted
//
// 4) print # of crimes, collisions and HT size
//
// 5) return hashTable
//
crimeReport **buildHashTable(char *crimesFile, char *areasFile, long N)
{
    //malloc space for HT
    crimeReport **hashTable = mallocHT(N);
    //call buildAreas
    char** areas = buildAreas(areasFile);
    //open Crimes.csv
    FILE* infile = fopen(crimesFile, "r");
    //declare variables for reading in values
    char  line[512];
    int   linesize = sizeof(line) / sizeof(line[0]);
    char *token;
    //declare variables for input into crimeReport
    char* inputCaseNumber;
    char* inputIUCR;
    char* inputDateTime;
    char* inputArrest;
    //declare variables for counting crimes and collisions
    long crimeCount = 0;
    long colCount = 0;
    //ignore first line
    fgets(line, linesize, infile);
    //while loop reads in crimes
    fgets(line, linesize, infile);
    while(!feof(infile))
    {
        line[strcspn(line, "\r\n")] = '\0'; //strip EOL char(s)
        //format: case#, IUCR, datetime, skip, skip, skip, arrest
        //store case number
        token = strtok(line, ",");
        assert(token != NULL);
        inputCaseNumber = (char*)malloc((strlen(token) + 1)*sizeof(char));
        strcpy(inputCaseNumber, token);
        //store IUCR
        token = strtok(NULL, ",");
        assert(token != NULL);
        inputIUCR = (char*)malloc((strlen(token) + 1)*sizeof(char));
        strcpy(inputIUCR, token);
        //store datetime
        token = strtok(NULL, ",");
        assert(token != NULL);
        inputDateTime = (char*)malloc((strlen(token) + 1)*sizeof(char));
        strcpy(inputDateTime, token);
        //skip x3
        token = strtok(NULL, ",");
        token = strtok(NULL, ",");
        token = strtok(NULL, ",");
        //store arrest
        token = strtok(NULL, ",");
        assert(token != NULL);
        inputArrest = (char*)malloc((strlen(token) + 1)*sizeof(char));
        strcpy(inputArrest, token);
        //store values in crimeReport struct
        colCount = store(hashTable, inputCaseNumber, inputIUCR, 
                        inputDateTime, inputArrest, areas, N, colCount);
        //get next line and increment crimeCount
        crimeCount++;
        fgets(line, linesize, infile);
    }
    fclose(infile);
    //build complete. Print crimeCount, HT size, colCount
    printf(">> # crimes:    %ld\n", crimeCount);
    printf(">> HT size:     %ld\n", N);
    printf(">> Collisions:  %ld\n\n", colCount);
    //return HT
    return hashTable;
}


int main()
{
    printf("** Chicago Crime Lookup **\n");
    //get filenames from stdin
    char *crimesFile = getFileName();
    char *areasFile = getFileName();

    //calculate size of crimesFile and use it to determine HT size
    long size = getFileSizeInBytes(crimesFile);
    printf(">> Crimes File: %ld bytes\n", size);
    //N = number of unique case numbers
    long N = 20280000;
    // build hashTable
    crimeReport **hashTable = buildHashTable(crimesFile, areasFile, N);
    //declare cmd for command input
    char input[64];
    //search hashTable for input case number until user inputs empty string 
    printf("Enter a case number> ");
    while (fgets(input, 64, stdin) && strcmp(input, "\n"))
    {
        //strip EOL char(s)
        input[strcspn(input, "\r\n")] = '\0'; 
        //search HT
        search(hashTable, N, input);
        //repeat
        printf("Enter a case number> ");
    }

    printf("** Done **\n");
    return 0;
}