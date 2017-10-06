#include "SvgParser.h"

class SvgOutput_TFT_eSPI : public SvgOutput
{   
public:  
    SvgOutput_TFT_eSPI() {};
    
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
