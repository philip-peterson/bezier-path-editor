#ifndef _H__VEC2
#define _H__VEC2

#include <cmath>

typedef struct vec2 {
   float x, y;
   public:
      vec2(float x, float y) : x(x), y(y) {}
      vec2() : x(-1), y(-1) {}

      // Dot product
      float operator| (vec2 b) {
         return this->x*b.x + this->y*b.y;
      }

      vec2 operator- (vec2 b) {
         return vec2(this->x-b.x, this->y-b.y);
      }

      vec2 operator+ (vec2 b) {
         return vec2(this->x+b.x, this->y+b.y);
      }

      vec2 operator* (float b) {
         return vec2(this->x*b, this->y*b);
      }

      vec2 operator/ (float b) {
         return vec2(this->x/b, this->y/b);
      }

      vec2 proj (vec2 other) {
         return other * ((*this | other) / (other | other));
      }

      float magnitude () {
         return sqrt( *this | *this );
      }

      bool operator== (vec2 other) {
         return (this->x == other.x) && (this->y == other.y);
      }

      bool operator!= (vec2 other) {
         return !(*this == other);
      }
} vec2;

#endif
