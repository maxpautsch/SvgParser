
/***************************************************************************************
 * * Function name:           SvgParser
 ** Description:             Constructor 
 ***************************************************************************************/

#ifndef _SVG_PARSER_H_
#define _SVG_PARSER_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef ESP8266
#include "FS.h"
#define DBG_OUT Serial.printf
#else
#include <stdio.h>
#define DBG_OUT printf
#endif


#ifndef uint8_t
#define uint8_t unsigned char
#endif

#ifndef int8_t
#define int8_t char
#endif

#ifndef uint16_t
#define uint16_t unsigned short
#endif

#ifndef int16_t
#define int16_t short
#endif

#ifndef uint32_t
#define uint32_t unsigned int
#endif


// ATTENTION: whenever you modify this line, change 'svgTypeNames' in 'processTag' accordingly!
enum svgTypes_t {SVG, RECT, TEXT, TSPAN, GROUP, PATH, LINK, CIRCLE, NONE};
enum svgPropertyState_t {UNDEFINED, UNSET, SET};

struct svgStyle_t {
    int16_t  x_offset;
    int16_t  y_offset;
    int16_t  x;
    int16_t  y;
    float    x_scale;
    float    y_scale;
    
    struct svgLinkList_t *linkRef;
    
    uint32_t fill_color;
    uint32_t stroke_color;
    uint8_t  stroke_width;
    uint8_t  font_size;
    
    enum svgPropertyState_t  fill_color_set :2;
    enum svgPropertyState_t  stroke_color_set :2;
    enum svgPropertyState_t  stroke_width_set :2;
    enum svgPropertyState_t  font_size_set :2;
    
};

struct svgLinkList_t {
    char * link;
    struct svgLinkList_t *next;
};

struct svgLinkRefList_t {
    int16_t  x_min, y_min, x_max, y_max;
    struct svgLinkList_t *linkRef;
    struct svgLinkRefList_t *next;
};


class SvgOutput
{
public:  
    SvgOutput() {};
    
    virtual void circle(int16_t x, int16_t y, int16_t radius, struct svgStyle_t * style)
    {   
        DBG_OUT("CIRCLE: x %i y %i radius %i\n",x,y,radius);
    } 
    
    virtual void rect(int16_t x, int16_t y, int16_t width, int16_t height, struct svgStyle_t * style)
    { 
        DBG_OUT("RECT: x %i y %i width %i height %i\n",x,y,width, height);
        
    }
    virtual void text(int16_t x, int16_t y, char * text, struct svgStyle_t * style)
    { 
        DBG_OUT("TEXT: x %i y %i text \"%s\"\n",x,y,text);
        
    }
private:

};

// Class functions and variables
class SvgParser 

{
    
public:
    SvgParser(SvgOutput *newout);
    uint8_t print(int16_t start_x=0, int16_t start_y=0);
    uint8_t readFile(char * fileName);
    uint8_t linkManagement(uint8_t cleanup=0);
    uint8_t onClick(uint16_t x, uint16_t y, char **link);
    
private:
    void trimStr();
    
    uint8_t processElement(char * start, enum svgTypes_t type, struct svgStyle_t * style);
    uint8_t processTag(char * start, char ** tagStart, uint16_t *processed, uint8_t parents, char *parentEnd, struct svgStyle_t * parentStyle);

    uint8_t getProperty(char * start, const char * property, float * data);
    uint8_t getProperty(char * start, const char * property, int16_t * data);
    uint8_t getProperty(char * start, const char * property, char ** data);
    char *  getPropertyStart(char * start, const char * property);
    uint8_t parseStyle(char * start, struct svgStyle_t * style);
    
    uint8_t addLinkReference(int16_t x_min, int16_t y_min, int16_t x_max, int16_t y_max, struct svgStyle_t * style);
    
    uint16_t _bufLen, _curPos;
    int8_t  * _data;
    struct svgLinkList_t * _linkList = NULL;
    struct svgLinkRefList_t * _linkRefList = NULL;
    SvgOutput *_output;
    void cleanup(){
        // clean up link lists
        linkManagement(1);
        free(_data);
        _bufLen = 0;
        _curPos = 0;
    }
    
};




#endif
