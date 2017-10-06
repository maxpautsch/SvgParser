#include <iostream>
#include <stdio.h>
#include "SvgParser.h"

int main()
{
char *link;
SvgOutput svgOutput = SvgOutput();
SvgParser svg = SvgParser(&svgOutput);

svg.readFile((char *)"../examples/TFT_eSPI_Demo/data/img.svg");
svg.print();
//svg.linkManagement(1);

svg.linkManagement();
if(svg.onClick(2,420, &link)) printf("pressed: #%s#\n",link);
if(svg.onClick(20,420, &link)) printf("pressed: #%s#\n",link);

    return 0;
}

