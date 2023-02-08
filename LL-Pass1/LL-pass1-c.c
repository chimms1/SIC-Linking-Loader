// Contributed by Yash Deshpande and Shreyas Joshi
// Linking Loader Pass 1

// Requires ll-input.txt as input
// Writes output in ESTAB.txt

// 1. Processing Header records
// 2. Processing Define records
// 3. Added ESTAB search to check for errors

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

// Structure to hold components of Header Record
struct H_ESTAB
{
    char csname[SMOL];
    char extsym[SMOL];
    int address;
    int length;
};

// Structure to hold components of Define Record
struct D_ESTAB
{
    char csname[SMOL];
    char varname[SMOL];
    int varaddress;
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

// Function returns 1 if CSNAME is found in ESTAB, else returns 0
int check_H_ESTAB_Error(struct H_ESTAB Hrecord,struct H_ESTAB Hrecords[],struct D_ESTAB Drecords[],int Hcount,int Dcount)
{
    // rewind(fp2);

    // No need to scan ESTAB file
    // Search in Hrecords and Drecords Array instead

    if(Hcount==0)
    {
        return 0;
    }

    for(int i=0; i<Hcount;i++)
    {
        if(!strcmp(Hrecord.csname,Hrecords[i].csname))
        {
            return 1;
        }
    }

    if(Dcount==0)
    {
        return 0;
    }

    for(int i=0;i<Dcount;i++)
    {
        if(!strcmp(Hrecord.csname,Drecords[i].varname))
        {
            return 1;
        }
    }
    
    return 0;
}

// Function returns 1 if variable is found in ESTAB, else returns 0
int check_D_ESTAB_Error(struct D_ESTAB Drecord,struct H_ESTAB Hrecords[],struct D_ESTAB Drecords[],int Hcount,int Dcount)
{
    // rewind(fp2);

    // No need to scan ESTAB file
    // Search in Hrecords and Drecords Array instead

    if(Dcount==0)
    {
        return 0;
    }

    for(int i=0;i<Dcount;i++)
    {
        if(!strcmp(Drecord.varname,Drecords[i].varname))
        {
            return 1;
        }
    }

    if(Hcount==0)
    {
        return 0;
    }

    for(int i=0; i<Hcount;i++)
    {
        if(!strcmp(Drecord.varname,Hrecords[i].csname))
        {
            return 1;
        }
    }
    
    return 0;
}

// Process input file
int Process(FILE* fp1,FILE* fp2,struct H_ESTAB Hrecords[],struct D_ESTAB Drecords[],int PROGADDR)
{
    int line_count = 0;
    int Hcount = 0;
    int Dcount = 0;

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
            // Hrecords[Hcount] = getHeaderTokens(line);
            struct H_ESTAB Htemp = getHeaderTokens(line);

            // Check for repeated name
            int flag = check_H_ESTAB_Error(Htemp,Hrecords,Drecords,Hcount,Dcount);

            // If no error
            if(flag==0)
            {
                // Copy Htemp to Hrecords Array
                strcpy(Hrecords[Hcount].csname,Htemp.csname);
                strcpy(Hrecords[Hcount].extsym,Htemp.extsym);
                Hrecords[Hcount].address = Htemp.address;
                Hrecords[Hcount].length = Htemp.length;

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
            else if(flag==1)
            {
                printf("\nError at line %d: %s is repeated\n",line_count,Htemp.csname);
            }
        }

        // If line is a Define Record
        else if(line[0] == 'D' && line[1] == '^')
        {
            // printf("Here is a Define\n");
            char holder[SMOL];
            holder[0] = '\0';
            int toggle = 0;

            for(int i=2;i<strlen(line)+1;i++)
            {
                char temp = line[i];
                if(line[i]=='^'||line[i]=='\0')
                {
                    struct D_ESTAB Dtemp;
                    
                    // strcat(holder,"\0");
                    if(toggle == 0)
                    {
                        // printf("Here is a variable\n");
                        strcpy(Dtemp.varname,holder);
                        toggle = 1;
                    }

                    else if(toggle==1)
                    {
                        Dtemp.varaddress = (int)strtol(holder,NULL,16);
                        // int h = Hcount - 1;
                        Dtemp.varaddress += Hrecords[Hcount-1].address;
                        toggle = 0;

                        // Check for repeated name
                        int flag = check_D_ESTAB_Error(Dtemp,Hrecords,Drecords,Hcount,Dcount);

                        // If no error
                        if(flag == 0)
                        {
                            strcpy(Drecords[Dcount].varname,Dtemp.varname);
                            Drecords[Dcount].varaddress = Dtemp.varaddress;

                            // Write Define variables in ESTAB
                            printf("\t%s\t%04X\n",Drecords[Dcount].varname,Drecords[Dcount].varaddress);
                            fprintf(fp2,"\t%s\t%04X\n",Drecords[Dcount].varname,Drecords[Dcount].varaddress);
                            Dcount++;
                        }
                        else if(flag == 1)
                        {
                            printf("\nError at line %d: %s is repeated\n",line_count,Dtemp.varname);
                        }
                    }
                    holder[0] = '\0';
                    continue;
                }
            char temp_star[2] = {temp,'\0'}; /* gives {value of temp, \0} */
            strcat(holder,temp_star);

            }
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
    struct D_ESTAB Drecords[BIG];
    int PROGADDR;

    printf("Enter the starting location to load the program: ");
    scanf("%X",&PROGADDR);
    
    printf("\nCS_NAME SYM_NAME ADDR\tLEN\n");
    printf("---------------------------------------------\n");
    int line_count = Process(fp1,fp2,Hrecords,Drecords,PROGADDR);

    printf("---------------------------------------------\n");
    printf("\n  Number of lines in Input file = %d\n",line_count);

    fclose(fp1);
    fclose(fp2);

    printf("\n  --Files closed-- \n\n");
    return 0;   // Program executed successfully
}