#include "SvgParser.h"


SvgParser::SvgParser(SvgOutput *newout)
{
    _output = newout;
}


uint8_t SvgParser::getProperty(char * start, const char * property, float * data){
    char *ptr = getPropertyStart(start, property);
    //  DBG_OUT("get float %s\n",property);
    if(ptr == NULL)
        return false;
    return sscanf(ptr,"%f",data);
}

uint8_t SvgParser::getProperty(char * start, const char * property, int16_t * data){
    char *ptr = getPropertyStart(start, property);
    //   DBG_OUT("get int %s\n",property);
    
    if(ptr == NULL)
        return false;
    return sscanf(ptr,"%i",data);
}

uint8_t SvgParser::getProperty(char * start, const char * property, char ** data){
    char *ptr2;
    char *ptr = getPropertyStart(start, property);
    
    //  DBG_OUT("get char %s\n",property);
    
    if(ptr == NULL)
        return false;
    
    ptr2 = strstr (ptr,"\"");
    if(ptr2 == NULL) return false;
    
    *data = (char *)malloc((ptr2-ptr)+1);
    if(*data == NULL)
        return false;
    
    memcpy(*data,ptr,ptr2-ptr);
    (*data)[ptr2-ptr]=0;   
    
    return true;
}

char * SvgParser::getPropertyStart(char * start, const char * property){
    char *ptr, *ptr2, *searchString;
    uint16_t pLen = strlen(property);
    uint16_t sLen;
    uint8_t retVal;
    
    searchString = (char *)malloc(pLen+4);
    if(searchString == NULL)
        return NULL;
    memcpy(searchString+1,property,pLen);
    searchString[0]=' ';
    searchString[pLen+1]='=';
    searchString[pLen+2]='"';
    searchString[pLen+3]=0;
    
    ptr = strcasestr ((char *)start,searchString);
    free(searchString);
    
    if(ptr == NULL) return NULL;
    return ptr+pLen+3;
}

uint8_t SvgParser::parseStyle(char * start, struct svgStyle_t * style){
    char *ptr = start; 
    char *ptr2;
    enum svgPropertyState_t tmp_state;
    uint32_t tmp_val;
    float    tmp_val_f;
    
    const char *propNames[] = {"fill:", "stroke:", "stroke-width:", "font-size:", NULL};
    
    //  printf("search style: #%s#\n",start);
    
    for(;;){
        if(*ptr == 0)
            break;
        ptr2 = strstr(ptr,";");
        if(ptr2)
            *ptr2 = 0;
        
        //         printf("style prop : %s\n",ptr);
        
        for(int8_t i=0; ;i++){
            if(propNames[i] == NULL) break;
            if(strncasecmp(ptr,propNames[i],strlen(propNames[i])) == 0){
                tmp_state = SET;
                if(strncasecmp(ptr+strlen(propNames[i]),"none",4) == 0){
                    tmp_state = UNSET;
                    tmp_val = 0;
                    tmp_val_f = 0;
                } else if(*(ptr+strlen(propNames[i])) == '#') {
                    if(!sscanf(ptr+strlen(propNames[i])+1,"%x",&tmp_val))
                        return false;
                } else {//if(*ptr+strlen(propNames[i]) >= '0' && *ptr+strlen(propNames[i]) <='9') {
                    //      printf("parse float\n");
                    if(!sscanf(ptr+strlen(propNames[i]),"%f",&tmp_val_f))
                        return false;
                }
                
                switch(i) {
                    case 0: // fill
                        style->fill_color_set = tmp_state;
                        style->fill_color = tmp_val;
                        break;
                    case 1: // stroke
                        style->stroke_color_set = tmp_state;
                        style->stroke_color = tmp_val;
                        break;
                    case 2: // stroke-width
                        style->stroke_width_set = tmp_state;
                        if(tmp_val_f>0 && tmp_val_f<1) tmp_val_f = 1;
                        style->stroke_width = tmp_val_f;
                        break;
                    case 3: // font-size
                        style->font_size_set = tmp_state;
                        if(tmp_val_f>0 && tmp_val_f<1) tmp_val_f = 1;
                        style->font_size = tmp_val_f;
                        break;
                }
            }
        }
        
        if(ptr2)
            ptr = ptr2 +1;
        else
            *ptr = 0;
    }
}

uint8_t SvgParser::onClick(uint16_t x, uint16_t y, char **link){
    
    struct svgLinkRefList_t ** tLinkRefList = &_linkRefList;
    
    while(*tLinkRefList != NULL){
        if(x >= (*tLinkRefList)->x_min && x <= (*tLinkRefList)->x_max && y >= (*tLinkRefList)->y_min && y <= (*tLinkRefList)->y_max) {
                        *link = ((*tLinkRefList)->linkRef)->link;
                        return true;
        }
        tLinkRefList = &(*tLinkRefList)->next; 
    }
    return false;
}

uint8_t SvgParser::linkManagement(uint8_t cleanup){
    if(!cleanup) DBG_OUT("\nlink management\n");
    
    // clean or print link references
    struct svgLinkRefList_t ** tLinkRefListNext, ** tLinkRefList = &_linkRefList;
    
    while(*tLinkRefList != NULL){
        tLinkRefListNext = &(*tLinkRefList)->next;
        if(cleanup){
            free(*tLinkRefList);
            *tLinkRefList = NULL;
        }
        else
            DBG_OUT("link reference entry: x_min %i x_max %i y_min %i y_max %i link %s\n",(*tLinkRefList)->x_min,(*tLinkRefList)->x_max,(*tLinkRefList)->y_min,(*tLinkRefList)->y_max,((*tLinkRefList)->linkRef)->link);
        tLinkRefList = tLinkRefListNext; 
    }
    
    // clean or print link list
    struct svgLinkList_t ** tLinkListNext, ** tLinkList = &_linkList;
    
    while(*tLinkList != NULL){
        tLinkListNext = &(*tLinkList)->next;
        if(cleanup){
            free(((*tLinkList)->link));
            free(*tLinkList);
            *tLinkList = NULL;
        }
        else
            DBG_OUT("link entry: #%s#\n",(*tLinkList)->link);
        tLinkList = tLinkListNext; 
    }
}

uint8_t SvgParser::addLinkReference(int16_t x_min, int16_t y_min, int16_t x_max, int16_t y_max, struct svgStyle_t * style){
        struct svgLinkRefList_t ** tLinkRefList = &_linkRefList;
        
        if(style->linkRef == NULL) return false;
        
        while(*tLinkRefList != NULL){
            tLinkRefList = &(*tLinkRefList)->next;
        }
        *tLinkRefList = (struct svgLinkRefList_t *)malloc(sizeof(svgLinkRefList_t));
        if(*tLinkRefList == NULL)
            return false;
        (*tLinkRefList)->x_min = x_min;
        (*tLinkRefList)->y_min = y_min;
        (*tLinkRefList)->x_max = x_max;
        (*tLinkRefList)->y_max = y_max;
        (*tLinkRefList)->linkRef = style->linkRef;
        (*tLinkRefList)->next = NULL;
        return true;
}


uint8_t SvgParser::processElement(char * start, enum svgTypes_t type, struct svgStyle_t * style){
    float x,y,height,width;
    char * styleString;
    //  printf("process elment type: %i \n",type);
    
    if(type == GROUP){
        char *transform;
        if(getProperty(start, "transform", &transform)){
            if(strncasecmp(transform,"translate(",strlen("translate("))==0){
                float x, y;
                if(!sscanf(transform+strlen("translate("),"%f,%f",&x,&y))
                    return false;
                
                printf("translate: %f*%f \n",x,y);
                style->x_offset += x* style->x_scale;
                style->y_offset += y* style->y_scale;
            }
            free(transform);
        }
    } 
    else if(type == TEXT || type == TSPAN){
        if(!getProperty(start, "x", &x))
            return false;
        if(!getProperty(start, "y", &y))
            return false;
        // check if there is a style string
        getProperty(start, "style", &styleString);
        style->x = x*style->x_scale+style->x_offset;
        style->y = y*style->y_scale+style->y_offset;
        
    } 
    else if(type == LINK){
        char * link;
        uint16_t cnt = 0;
        
        struct svgLinkList_t ** tLinkList = &_linkList;
        
        if(!getProperty(start, "xlink:href", &link))
            return false;
        DBG_OUT("link to: #%s#\n",link);
        
        while(*tLinkList != NULL){
            tLinkList = &(*tLinkList)->next;
            cnt ++;
        }
        *tLinkList = (struct svgLinkList_t *)malloc(sizeof(svgLinkList_t));
        (*tLinkList)->link = link;
        (*tLinkList)->next = NULL;
        
        // all childs will get this attribute
        style->linkRef = *tLinkList;
    } 
    else if(type == RECT){
        if(!getProperty(start, "x", &x))
            return false;
        if(!getProperty(start, "y", &y))
            return false;
        if(!getProperty(start, "width", &width))
            return false;
        if(!getProperty(start, "height", &height))
            return false;
        if(!getProperty(start, "style", &styleString))
            return false;
        x = x*style->x_scale+style->x_offset;
        y = y*style->y_scale+style->y_offset;
        height = height*style->x_scale;
        width = width*style->y_scale;
        DBG_OUT("rect: x: %f y: %f height: %f width: %f \n",x,y,height,width);
        
        parseStyle(styleString, style);
        DBG_OUT("fill: %X stroke color %X stroke width %i \n",style->fill_color, style->stroke_color,style->stroke_width);
        
        addLinkReference(x, y, x+width, y+height, style);

        _output->rect(x, y, width, height, style);
        free(styleString);
    } 
    else if(type == CIRCLE){
        float radius;
        if(!getProperty(start, "cx", &x))
            return false;
        if(!getProperty(start, "cy", &y))
            return false;
        if(!getProperty(start, "r", &radius))
            return false;
        if(!getProperty(start, "style", &styleString))
            return false;
        x = x*style->x_scale+style->x_offset;
        y = y*style->y_scale+style->y_offset;
        //TODO: radius scale?!
        
        printf("circle: x: %f y: %f radius: %f \n",x,y,radius);
        
        parseStyle(styleString, style);
        printf("fill: %X stroke color %X stroke width %i \n",style->fill_color, style->stroke_color,style->stroke_width);
        
        addLinkReference(x-radius, y-radius, x+radius, y+radius, style);

        free(styleString);
    } 
    else if(type == PATH){
        char * data, *ptr, *ptr2;
        uint8_t absolutePos = true;
        uint8_t first = true;
        float last_x, last_y;
        
        if(!getProperty(start, "d", &data))
            return false;
        if(!getProperty(start, "style", &styleString))
            return false;
        
        parseStyle(styleString, style);
        printf("stroke color %X stroke width %i \n",style->stroke_color,style->stroke_width);        
        free(styleString);
        
        //      printf("data %s \n",data);        
        
        ptr = data;
        for(;;){
            if(*ptr == 0)
                break;
            ptr2 = strstr(ptr," ");
            if(ptr2)
                *ptr2 = 0;
            
            // printf("style prop : %s\n",ptr);
            if(*ptr == 'M') absolutePos = true;
            else if (*ptr == 'm') absolutePos = false;
            else if (*ptr >= '0' && *ptr <= '9'){
                if(!sscanf(ptr,"%f,%f",&x,&y)){
                    free(data);
                    return false;
                }
                x *= style->x_scale;
                y *= style->y_scale;
                
                if(first){
                    first = false;
                    last_x = style->x_offset + x ;
                    last_y = style->y_offset + y;
                } else {
                    if(!absolutePos){ x+=last_x; y+=last_y; }
                    else {x+=style->x_offset; y+=style->y_offset; }
                    //     printf("path from: %f %f to %f %f\n",last_x,last_y,x,y);
                    if(!absolutePos){
                        last_x=x;
                        last_y=y;
                    }
                }
                
            }
            // ignore curves, arcs etc.
            
            if(ptr2)
                ptr = ptr2 +1;
            else
                *ptr = 0;
        }
        free(data);
    } 
    else if(type == SVG){
        char * viewbox;
        float viewboxData[4];
        if(!getProperty(start, "width", &width))
            return false;
        if(!getProperty(start, "height", &height))
            return false;
        if(getProperty(start, "viewbox", &viewbox)){
            if(!sscanf(viewbox,"%f %f %f %f",&viewboxData[0],&viewboxData[1],&viewboxData[2],&viewboxData[3]))
                return false;
            style->x_offset = viewboxData[0];
            style->y_offset = viewboxData[1];
            style->x_scale = width / viewboxData[2];
            style->y_scale = height / viewboxData[3];
            free(viewbox);
        }
        
        //     printf("svg: height: %f width: %f \n",height,width);
    }
    
    // printf("success: %d\n",sscanf(ptr + strlen("x=\""),"%f",&x));
    
    return true;
}

// returns stop of tag
// 0: error
// 1: file end
// 2: parent end
// 3: success
uint8_t SvgParser::processTag(char * start, char ** tagStart, uint16_t *processed, uint8_t parents, char * parentEnd, struct svgStyle_t * parentStyle){
    const char *svgTypeNames[] = {"svg", "rect", "text", "tspan", "g", "path", "a", "circle", NULL};
    
    char *curPos, *thisTagEnd, *childTagStart, *searchResult, *endTag;
    bool endAfterTag = false; // tag is completely finished
    bool halfAfterTag = false; // first part is finished and > is removed
    bool hasChilds = false;
    uint16_t childProcessed;
    enum svgTypes_t type = NONE;  
    struct svgStyle_t style;
    
    *processed = 0;
    // search for new tag
    for(curPos = start; curPos < _data+_bufLen; curPos++){
        // start of new tag?
        if(*curPos == '<')
            break;
    }
    
    *tagStart = curPos;
    
    // EOF?
    if(curPos == _data+_bufLen){
        *processed = (uint16_t)(_data + _bufLen - start);
        return 1;
    }
    
    curPos++;
    
    // search for end of tag name
    for(searchResult = curPos; searchResult < _data+_bufLen; searchResult++){
        if(*searchResult == ' ' || *searchResult == '>' || *searchResult == 0){
            if(*searchResult == '>'){
                halfAfterTag = true;
                thisTagEnd = searchResult+1;
            }
            break;
        }
    } 
        
    // no end of tag name was found!
    if(searchResult == _data+_bufLen){
        *processed = _data + _bufLen - start;
        return 0;
    }
    
    if(*(searchResult-1) == '/' || *(searchResult-1) == '?'){
        endAfterTag = true;
        searchResult--;
    }
    
    *searchResult = 0;
    endTag = curPos-1;
    *endTag = '/';
    
    
    if(parentEnd != NULL){
        if(strcasecmp(curPos,parentEnd) == 0){
            
            for(int8_t i=0;i<parents-1;i++) { DBG_OUT(" "); } DBG_OUT("end of parent tag #%s#\n",parentEnd);
            
            *processed =  strlen(parentEnd)+2;
            return 2;
        }
    }
    
    // ok. here we have an independent tag. copy stlye of parent
    if(parentStyle != 0){
        memcpy(&style,parentStyle,sizeof(style));
    } else {
        style.x_scale = 1;
        style.y_scale = 1;
        style.fill_color = 0xFFFFFFFF;
        style.stroke_color = 0;
        style.linkRef = NULL;
        
        style.fill_color_set = UNDEFINED;
        style.stroke_color_set = UNDEFINED;
        style.stroke_width_set = UNDEFINED;
        style.font_size_set = UNDEFINED;
    }
    // printf("x start %i y start %i x scale %f y scale %f\n",style.x_offset, style.y_offset, style.x_scale, style.y_scale);
    for(int8_t i=0;i<parents;i++) { DBG_OUT(" "); } DBG_OUT("tag: #%s#\n",curPos);
    
    // convert tag name to svgType
    type = NONE; // if unknown
    for(int8_t i=0; ;i++){
        if(svgTypeNames[i] == NULL) break;
        if(strcmp(curPos,svgTypeNames[i]) == 0){
            type = (svgTypes_t)i;
            DBG_OUT("found %s\n",svgTypeNames[i]);
        }
    }
    
    curPos = searchResult;
    
    for( ; curPos < _data+_bufLen; curPos++){
        if(*curPos == '>' || endAfterTag || halfAfterTag) {
            if(!halfAfterTag)
                thisTagEnd = curPos+1;
            halfAfterTag = true;
            
            *searchResult = ' ';
            *curPos = 0;
            
            processElement(searchResult, type, &style);
            *searchResult = 0;  
            
            // completely ready with tag?
            if (*(curPos-1)=='/'  || *(curPos-1)=='?' || endAfterTag){ // end of tag? could be removed previously
                *processed = curPos-start;
                return 3;
            }
            // search for sub tags
            
            else {
                //                  DBG_OUT("sub tag search\n");
                
                for(;;){
                    uint16_t retVal = processTag(curPos,&childTagStart,&childProcessed, parents+1,endTag,&style);
                    if(retVal == 0)
                        return 0;
                    else if(retVal == 1)
                        return 1;
                    else if(retVal == 2){
                        // parent end!
                        *processed = curPos +childProcessed- start;
                        // check for tag value widthin > <
                   
                        if(!hasChilds) {
                            *childTagStart=0;
                            *tagStart = childTagStart;
                            if(strlen(thisTagEnd)){

                               DBG_OUT("TEXT: x %i y %i stroke color %X stroke width %i value #%s#\n",style.x,style.y,style.stroke_color,style.stroke_width,thisTagEnd);
                            }
                        }
                        return 3;
                    }
                    else if(retVal == 3){
                        hasChilds = true;
                        // child end. search next child
                        curPos += childProcessed;
                        
                    }
                    //*tagStart = curPos;
                    thisTagEnd = curPos;
                }
            }
        }
    }
    
    return 0;
}

uint8_t SvgParser::print(int16_t start_x, int16_t start_y){
    uint16_t processed;
    uint16_t done = 0;
    uint8_t retVal=0;
    char *childTagEnd;
    //uint8_t processTag(char * start,  uint16_t *processed, uint8_t parents, char *parentEnd);
    while(done!=_bufLen){
        DBG_OUT("print run last val %i start %i\n",retVal,done);
        retVal = processTag(_data+done,&childTagEnd,&processed,0,NULL,NULL);
        if(retVal != 3)
            return false;
        
        done += processed;
    }  
    return true;
}


void SvgParser::trimStr() {
    int lastCopy = 0;
    int startCut = 0;
    int pos;
    char *tmp;
    
    enum {NORMAL, LAST_SPACE, WITHIN_SOME_SPACES} lastWasSpace;
    
    // remove all newline, carriage return and tabs
    for (int i = 0; i < _bufLen; i++) {
        if ((_data)[i] == '\n' || (_data)[i] == '\r' || (_data)[i] == '\t')
            (_data)[i] = ' ';
    }
    
    lastWasSpace = LAST_SPACE;
    
    for (pos = 0; pos < _bufLen; pos++) {
        switch (lastWasSpace) {
            
            case NORMAL:
                if ((_data)[pos] == ' ') {
                    lastWasSpace = LAST_SPACE;
                }
                break;
                
            case LAST_SPACE:
                if ((_data)[pos] == ' ') {
                    lastWasSpace = WITHIN_SOME_SPACES;
                    startCut = pos;
                } else
                    lastWasSpace = NORMAL;
                break;
                
            case WITHIN_SOME_SPACES:
                // start cutting with first non space character
                if ((_data)[pos] != ' ') {
                    memcpy(&((_data)[startCut]), &((_data)[pos]), _bufLen - pos + 1);
                    pos = startCut;
                    lastWasSpace = NORMAL;
                }
                break;
        }
    }
    
    // some spaces at the end?
    if (lastWasSpace == WITHIN_SOME_SPACES) {
        (_data)[startCut] = 0;
    }
    
    _bufLen = strlen((const char *)_data);
    if ((_data)[_bufLen - 1] == ' ') {
        (_data)[_bufLen - 1] = 0;
        _bufLen++;
    }
}

uint8_t SvgParser::readFile(char * fileName){
    // clean up first
    cleanup();
    
#ifdef ESP8266
     if (!SPIFFS.begin())
         return false;

  // check if calibration file exists
    File f = SPIFFS.open(fileName, "r");
    if (f) {
            _data = (int8_t *)malloc(f.size() + 1); 

      if (f.readBytes(_data, f.size()) != f.size()){
          free(_data);
          return false;
      }
      f.close();
    }
    
#else
    FILE *f = fopen(fileName, "rb");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);  //same as rewind(f);
    
    _data = (int8_t *)malloc(fsize + 1); 
    fread(_data, fsize, 1, f);
    _data[fsize] = 0; // terminate 
    fclose(f);
    
    _bufLen = fsize;
    
#endif
    _curPos = 0;
    DBG_OUT("length read: %i\n",_bufLen);
    trimStr();
    _data = (char *)realloc((void *)_data,strlen(_data));
    DBG_OUT("length after trim: %i\n",_bufLen);
    
}
