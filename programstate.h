#ifndef _H__PROGRAMSTATE
#define _H__PROGRAMSTATE

#include <vector>

struct programstate {
   int cursorX, cursorY;
   std::vector<vec2> points;
   int renderMode;
   int level;
   int blackholeEnterer;
   bool inBlackhole;
   bool drawDots;

   public:
   programstate() : cursorX(-1), cursorY(-1), renderMode(0), level(5), blackholeEnterer(-1), inBlackhole(false), drawDots(true) {

   }

};

#endif
