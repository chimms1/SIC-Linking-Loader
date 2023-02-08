// Contributed by Yash Deshpande and Shreyas Joshi
// Linking Loader
// 1. Processing Header record

/*
Control | Symbol | Address | Length
Section | Name   |            
------------------------------
PROGA	        	500 	63
PROGB		        563     7F
PROGC		        5E2     51
*/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define BIG 1024
#define SMOL 128

struct H_ESTAB
{
    char csname[SMOL];
    char extsym[SMOL];
    int address;
    int length;
};

struct H_ESTAB getHeaderTokens(char line[])
{
    struct H_ESTAB temp_record;
    char* type;

    type = strtok(line,"^");
    strcpy(temp_record.csname,strtok(NULL,"^"));
    strcpy(temp_record.extsym,"");
    temp_record.address = (int)strtol(strtok(NULL,"^"),NULL,16);
    temp_record.length = (int)strtol(strtok(NULL,"^"),NULL,16);

    return temp_record;
}

int Process(FILE* fp1,FILE* fp2,struct H_ESTAB Hrecords[],int PROGADDR)
{
    int line_count = 0;
    int Hcount = 0;

    while(!feof(fp1))
    {
        char line[SMOL];

        fgets(line,SMOL,fp1);

        // If there is an empty line
        if(line[0] == '\n')
        {
            continue;
        }

        // fgets things
        if(line[strlen(line)] == '\n')
        {
            line[strlen(line)-1] = '\0';
        }

        // If line is a Header Record
        if(line[0] == 'H' && line[1] == '^')
        {
            Hrecords[Hcount] = getHeaderTokens(line);

            // Add PROGADDR from user to first record
            if(Hcount==0)
            {
                Hrecords[Hcount].address += PROGADDR;
            }
            
            // Next records continue after previous ones
            else if(Hcount>0)
            {
                Hrecords[Hcount].address += Hrecords[Hcount-1].address + Hrecords[Hcount-1].length;
            }


            // Writing in ESTAB file
            printf("%s\t%s\t%04X\t%04X\n",Hrecords[Hcount].csname,Hrecords[Hcount].extsym,Hrecords[Hcount].address,Hrecords[Hcount].length);
            fprintf(fp2,"%s\t%s\t%04X\t%04X\n",Hrecords[Hcount].csname,Hrecords[Hcount].extsym,Hrecords[Hcount].address,Hrecords[Hcount].length);

            Hcount++;   
        }

        // If line is a Define Record
        else if(line[0] == 'D' && line[1] == '^')
        {
            
        }

        // If line is a Refer Record
        else if(line[0] == 'R' && line[1] == '^')
        {
            
        }

        else if(line[0] == 'E' && line[1] == '^')
        {
            
        }

        line_count++;
    }


    return line_count;
}

int main()
{
    FILE *fp1 = fopen("ll-input.txt","r"); //Open input file in read mode
    FILE *fp2 = fopen("ESTAB.txt","w"); //Open output file in write mode

    if(fp1==NULL){
        perror("  => Error opening the Input read file");
        return -1;
    }
    if(fp2==NULL){
        printf("  => Error opening the ESTAB write file");
        return -2;
    }

    struct H_ESTAB Hrecords[BIG];
    int PROGADDR;

    printf("Enter the starting location to load the program: ");
    scanf("%X",&PROGADDR);
    
    int line_count = Process(fp1,fp2,Hrecords,PROGADDR);

    return 0;   // Program executed successfully
}