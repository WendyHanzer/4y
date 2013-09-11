#include <stdio.h>
#include <stdlib.h>
#include <string.h>

 
char *textFileRead(char *fileName) {
 
    FILE *filePointer;
    char *content = NULL;
 
    int count=0;
 
    if (fileName != NULL) {
        filePointer = fopen(fileName,"rt");
 
       if (filePointer != NULL) {
         fseek(filePointer, 0, SEEK_END);
         count = ftell(filePointer);
         rewind(filePointer);

         if (count > 0) {
            content = (char *)malloc(sizeof(char) * (count+1));
            count = fread(content,sizeof(char),count,filePointer);
            content[count] = '\0';
          }
            fclose(filePointer);
       }
     }
    return content;
}
