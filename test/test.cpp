#include <iostream>
#include <stdio.h>
#include "SvgParser.h"

char * returnTest(int argc, char* argv[]){
    // first argv is allways name of function
    char *output = "RET VAL for #";
    char *ptr = (char *)malloc(strlen(output)+strlen(argv[0])+2);
    strcpy(ptr,output);
    strcpy(ptr+strlen(output),argv[0]);
    ptr[strlen(output)+strlen(argv[0])]='#';
    ptr[strlen(output)+strlen(argv[0])+1]=0;
    return ptr;
    
}


int counter=42;

char * increment(int argc, char* argv[]){
    counter++;
    return NULL;
}
char * decrement(int argc, char* argv[]){
    counter--;
    return NULL;
}

char * printCounter(int argc, char* argv[]){
    char *ptr = (char *)malloc(10);
    if(ptr == NULL) return NULL;
    sprintf(ptr,"%i",counter);
    return ptr;
}

char * printTime(int argc, char* argv[]){
    char *ptr = (char *)malloc(10);
    if(ptr == NULL) return NULL;
    sprintf(ptr,"%i",2323);
    return ptr;
}

int main()
{
    char *link;
    SvgOutput svgOutput = SvgOutput();
    SvgParser svg = SvgParser(&svgOutput);

    
  svg.addCallback("time",printTime);
  svg.addCallback("nr",printCounter);
  svg.addCallback("dec",decrement);
  svg.addCallback("inc",increment);

  
    svg.readFile((char *)"../../../TFT_eSPI_Demo/data/index.svg");
    
     svg.print();
    //svg.linkManagement(1);
    
    svg.linkManagement();
    if(svg.onClick(2,420, &link)) printf("pressed: #%s#\n",link);
    if(svg.onClick(20,420, &link)) printf("pressed: #%s#\n",link);
    
    
    svg.callbackManagement();

    return 0;
}

