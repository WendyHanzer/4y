#include <stdio.h>
#include <stdlib.h>
#include <string.h>

 
char *textFileRead(char *fileName) 
{
    //initialize variables
    FILE *fpointer;
    char *fcontent = NULL;
    long fsize = 0;
 
    if (fileName != NULL) 
    {
        //open file
        fpointer = fopen(fileName,"r");
 
        
        if (fpointer != NULL) 
        {     
            //get size of file
            fseek(fpointer, 0, SEEK_END);
            fsize = ftell(fpointer);
            rewind(fpointer);

            //allocate memory to store shader information
            if (fsize > 0) 
            {
                fcontent = (char *)malloc(sizeof(char) * (fsize+1));
                fsize = fread(fcontent,sizeof(char),fsize,fpointer);
                fcontent[fsize] = '\0';
            }
        
        //close file   
        fclose(fpointer);    
        }
    }
    return fcontent;
}
