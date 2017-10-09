#include "SvgParser.h"
#include <TFT_eSPI.h>

extern TFT_eSPI tft;

class SvgOutput_TFT_eSPI : public SvgOutput
{   
public:  
    SvgOutput_TFT_eSPI() {};
    

    
    virtual void circle(int16_t x, int16_t y, int16_t radius, struct svgStyle_t * style)
    {   
        DBG_OUT("CIRCLE: x %i y %i radiuss %i stroke width: %i\n",x,y,radius,style->stroke_width);
        tft.drawCircle(x, y, radius, style->stroke_color);
        int16_t start=0, end; // radius is in middle of stroke

        if(style->stroke_color_set != UNSET && style->stroke_width) {
            uint16_t stroke_color = convertColor(style->stroke_color);

            start = style->stroke_width*style->x_scale/2;
            end = start;
            end += (style->stroke_width*style->x_scale -start -end);
                
            // check if this can be done by printing to filled circles
            if(style->fill_color_set == SET){
                start = style->stroke_width*style->x_scale;
                tft.fillCircle(x, y, radius+end, stroke_color);
            } else {
                
                DBG_OUT("start: %i end: %i\n",start,end);
                for(uint16_t i=radius-start; i<radius+end; i++)
                  tft.drawCircle(x, y, i, stroke_color);
            }
        }
        
        // filled circle?
        if(style->fill_color_set == SET) {
            tft.fillCircle(x, y, radius-start, convertColor(style->fill_color));
        }
        
    }   

    virtual void rect(int16_t x, int16_t y, int16_t width, int16_t height, struct svgStyle_t * style)
    { 
        DBG_OUT("RECT: x %i y %i width %i height %i\n",x,y,width, height);
        
        // filled rect?
        if(style->fill_color_set == SET) {
            tft.fillRect(x, y, width, height, convertColor(style->fill_color));
        }
        
        if(style->stroke_color_set != UNSET && style->stroke_width) {
            uint16_t stroke_color = convertColor(style->stroke_color);
            tft.drawRect(x, y, width, height, stroke_color);
        }
    }
    virtual void text(int16_t x, int16_t y, char * text, struct svgStyle_t * style)
    { 
        DBG_OUT("TEXT: x %i y %i size %i text \"%s\"\n", x, y, style->font_size, text);
        if(style->stroke_color_set == UNSET && style->fill_color_set == UNSET) return;
        
        tft.setTextColor(convertColor(style->stroke_color));
                tft.setTextSize(1);

        DBG_OUT("cur font height: %i\n",tft.fontHeight());
        uint8_t newHeight = round(style->font_size*style->y_scale/ tft.fontHeight());
        if(!newHeight) newHeight++;
                DBG_OUT("height factor: %i\n",newHeight);

        tft.setTextSize(newHeight);
        
        tft.setTextDatum(BL_DATUM);
        tft.drawString(text, x, y + (style->font_size*style->y_scale - tft.fontHeight())/2);
       
    }
    
    virtual void path(uint16_t *data, uint16_t len, struct svgStyle_t * style)
    { 
        if(len<2) return;
        if(style->stroke_color_set == UNSET) return;
        uint16_t color = convertColor(style->stroke_color);
        
        for(uint16_t i = 1; i<len; i++)
            tft.drawLine(data[(i-1)*2], data[(i-1)*2+1], data[i*2], data[i*2 + 1], color);

        DBG_OUT("PATH: len: %i \n",len);
    }    
    
private:

    uint16_t convertColor(uint32_t color){
        return tft.color565((color & 0x00FF0000) >> 16, (color & 0x0000FF00) >> 8, color & 0x000000FF);
    }
};
