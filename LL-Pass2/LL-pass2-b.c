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

int Process(FILE* fp1,FILE* fp2,FILE* fp3,struct H_ESTAB Hrecords[],int PROGADDR,int EXECADDR,int CSLTH)
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

            CSLTH = Hrecords[Hcount].length;

            // Writing in ESTAB file
            printf("%s\t%s\t%04X\t%04X\n",Hrecords[Hcount].csname,Hrecords[Hcount].extsym,Hrecords[Hcount].address,Hrecords[Hcount].length);
            // fprintf(fp2,"%s\t%s\t%04X\t%04X\n",Hrecords[Hcount].csname,Hrecords[Hcount].extsym,Hrecords[Hcount].address,Hrecords[Hcount].length);
            // printf("CSLTH = %X\n",CSLTH);

            Hcount++;   
        }

        if(line[0] == 'T' && line[1] == '^')
        {
            printf("%s\n",line);
            int itr=1;
            char T_start_add_string[8];
            int T_start_add;
            int j=0;
            int i;

            for(i=itr+1;i<=itr+1+4;i++)
            {
                // printf("%c\n",line[i]);
                T_start_add_string[j] = line[i];
                j++;
            }

            T_start_add_string[j] = '\0';
            itr = 6;

            T_start_add = (int)strtol(T_start_add_string,NULL,16);

            if(Hcount==1)
                T_start_add = T_start_add + Hrecords[Hcount-1].address;
            else
                T_start_add = T_start_add + Hrecords[Hcount-2].address + Hrecords[Hcount-2].length;

            printf("T_start_add: %X\n",T_start_add);
            // fprintf(fp3,"%04X",T_start_add);

            char T_len_string[3];
            int T_len;
            j=0;
            for(i=itr+1;i<=itr+1+2;i++)
            {
                // printf("%c\n",line[i]);
                T_len_string[j] = line[i];
                j++;
            }
            T_len_string[j] = '\0';

            T_len = (int)strtol(T_len_string,NULL,16);
            printf("T_len: %X\n",T_len);

            itr = 9;
            i=itr+1;
            int k=0;

            int itr_len = T_start_add;

            
            int mem_word_count = 0x00 + T_start_add;
            int location = (mem_word_count/10)*10;
            int mem_line = 0;

            while(line[i]!='\0')
            {
                char object[9];
                j=i;
                k=0;
                // printf("j=%d ----- %c\n",j,line[j]);
                while(1)
                {
                    if(line[j]=='^'||line[j]=='\0'||line[j]=='\n')
                        break;
                    object[k] = line[j];
                    j++;
                    i++;
                    k++;
                }
                itr = i;
                i = itr+1;
                object[k] = '\0';

                if(mem_line==0)
                {
                    fprintf(fp3,"%X ",mem_word_count);
                    mem_line += 1;
                }

                printf("%s\n",object);

                fprintf(fp3,"%s ",object);

                itr_len += strlen(object)/2;

            }
            fprintf(fp3,"\n");
        }
        
        line_count++;
    }


    return line_count;
}

int main()
{
    FILE *fp1 = fopen("ll-input.txt","r"); //Open input file in read mode
    FILE *fp2 = fopen("ESTAB.txt","r"); //Open output file in write mode
    FILE *fp3 = fopen("ll-output.txt","w"); //Open output file in write mode


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

    int CSADDR = PROGADDR;
    int EXECADDR = PROGADDR;
    int CSLTH = 0;
    
    int line_count = Process(fp1,fp2,fp3,Hrecords,PROGADDR,EXECADDR,CSLTH);



    return 0;   // Program executed successfully
}