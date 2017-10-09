# embeddedSvg
This project implements a basic SVG parser for GUI applications on ESP8266. The GUI can be output on a display or as a website.
Currently it supports the TFT_eSPI display driver, but it can be very easily ported to other displays!

The following SVG elements are supported right now: svg, rects, circles, text, tspan, g (group), path, a (link).

In order to add live content to the output on the display, a callback mechanism is implemented. A specified tag is replaced by the return value of a function provided by the user program.

Touch displays are supported. With them, the link system can be used. Links to other SVGs can be executed automatically. The callback system applies also to the link system.

![demo index page](./doc/img/index.png)
'''
  svg.addCallback("time", printTime);
  svg.addCallback("nr", printCounter);
  svg.addCallback("dec", decrement);
  svg.addCallback("inc", increment);
  
  int counter = 42;

char * increment(int argc, char* argv[]) {
  counter++;
  return NULL;
}
char * decrement(int argc, char* argv[]) {
  counter--;
  return NULL;
}

char * printCounter(int argc, char* argv[]) {
  char *ptr = (char *)malloc(10);
  if (ptr == NULL) return NULL;
  sprintf(ptr, "%i", counter);
  return ptr;
}
'''


![demo test page](./doc/img/test.png)

