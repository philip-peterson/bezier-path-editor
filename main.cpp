#include <GL/freeglut.h>
#include "math.h"
#include "stdio.h"
#include "vec2.h"
#include "programstate.h"
#include <cmath>

#include <vector>


vec2 projectPointOntoLine(vec2 a, vec2 b, vec2 q) {
   if (q == a) {
      return a;
   }
   vec2 bma = b-a;
   vec2 r_qprime_rel_a = (q-a).proj(bma);
   if ( (bma | r_qprime_rel_a) < 0.0f ) {
      return a + r_qprime_rel_a;
   }
   else {
      return a - (r_qprime_rel_a)/(r_qprime_rel_a.magnitude()) * 0.1f;
   }
}

bool isFifthPoint(int order) {
   return (order > 1 && (order-1) % 3 == 0);
}

int bezierNumFromVertNum(int i) {
   return std::max(0, (i-1)/3);
}

static programstate ps;

static vec2 screen2view(vec2 screen) {
   vec2 view;
   view.x = glutGet(GLUT_WINDOW_WIDTH);
   view.x = (2*screen.x/view.x-1);
   view.y = glutGet(GLUT_WINDOW_HEIGHT);
   view.y = -(2*screen.y/view.y-1);
   return view;
}

enum drawmode {
   PARAMETRIC,
   DE_CASTELJAU,
   OPENGL,
   SUBDIVISION
};

static void drawBezierParametric(vec2 p0, vec2 p1, vec2 p2, vec2 p3) {
   glColor4f(0.0f, 0.0f, 1.0f, 1.0f);
   glBegin(GL_LINE_STRIP);
   for (int s = 0; s <= ps.level; s++) {
      float t = s / float(ps.level);
      float t2 = t*t;
      float t3 = t2*t;
      vec2 drawpoint = p3*t3
         + p2*3*(t2-t3)
         + p0*(1.0f - 3*(t - t2) - t3)
         + p1 * (3*(t + t3) - 6*(t2));
      glVertex2f(drawpoint.x,  drawpoint.y);
   }
   glEnd();
}

static vec2 lerp(vec2 a, vec2 b, float t) {
   return a*(1 - t) + b*t;
}

static void drawBezierDeCasteljau(vec2 p0, vec2 p1, vec2 p2, vec2 p3) {
   glColor4f(1.0f, 0.0f, 1.0f, 1.0f);
   glBegin(GL_LINE_STRIP);

   for (int s = 0; s <= ps.level; s++) {
      float t = s / float(ps.level);
      vec2 q0 = lerp(p0, p1, t);
      vec2 q1 = lerp(p1, p2, t);
      vec2 q2 = lerp(p2, p3, t);
      vec2 drawpoint = lerp(lerp(q0, q1, t), lerp(q1, q2, t), t);
      glVertex2f(drawpoint.x,  drawpoint.y);
   }
   glEnd();
}


static void drawBezierOpenGL(vec2 p0, vec2 p1, vec2 p2, vec2 p3) {
   glColor4f(0.9764f, 0.451f, 0.0235f, 1.0f);
   float points[12] = {
      p0.x, p0.y, 0.0f,
      p1.x, p1.y, 0.0f,
      p2.x, p2.y, 0.0f,
      p3.x, p3.y, 0.0f
   };

   glEnable(GL_MAP1_VERTEX_3);

   glMap1f(GL_MAP1_VERTEX_3, 0.0f, 1.0f, 3, 4, points);
   glBegin(GL_LINE_STRIP);
   for (int s = 0; s <= ps.level; s++) {
      glEvalCoord1f(s/float(ps.level));
   }
   glEnd();
}

static void _drawBezierSubdivHelper(int level, int maxlevel, vec2 p0, vec2 p1, vec2 p2, vec2 p3) {
   vec2 q0 = lerp(p0, p1, 0.5f);
   vec2 q1 = lerp(p1, p2, 0.5f);
   vec2 q2 = lerp(p2, p3, 0.5f);

   vec2 r0 = lerp(q0, q1, 0.5f);
   vec2 r1 = lerp(q1, q2, 0.5f);

   vec2 s = lerp(r0, r1, 0.5f);
   
   if (level < maxlevel) {
      _drawBezierSubdivHelper(level+1, maxlevel, p0, q0, r0, s);
   }
   else {
      glVertex2f(p0.x, p0.y);
   }
   glVertex2f(s.x, s.y);
   if (level < maxlevel) {
      _drawBezierSubdivHelper(level+1, maxlevel, s, r1, q2, p3);
   }
   else {
      glVertex2f(p3.x, p3.y);
   }
}

static void drawBezierSubdiv(vec2 p0, vec2 p1, vec2 p2, vec2 p3) {
   glColor4f(1.0f, 1.0f, 0.0f, 1.0f);

   glBegin(GL_LINE_STRIP);
   _drawBezierSubdivHelper(1, ps.level, p0, p1, p2, p3);
   glEnd();
}

static void drawBezier(drawmode mode) {
   int size = ps.points.size();
   for (int i = 0; i < (size-1)/3; i++) {
      vec2 a = ps.points[i*3+0];
      vec2 b = ps.points[i*3+1];
      vec2 c = ps.points[i*3+2];
      vec2 d = ps.points[i*3+3];
      switch(mode) {
         case PARAMETRIC:
            drawBezierParametric(a, b, c, d);
            break;
         case DE_CASTELJAU:
            drawBezierDeCasteljau(a, b, c, d);
            break;
         case OPENGL:
            drawBezierOpenGL(a, b, c, d);
            break;
         case SUBDIVISION:
            drawBezierSubdiv(a, b, c, d);
            break;
      }
   }
}

static bool findLastTwoNonzero(vec2& a, vec2& b) {
   int i = ps.points.size()-1;
   if (i < 1) {
      return false;
   }
   b = ps.points.back();
   for (; i >= 0; i--) {
      a = ps.points.at(i);
      if (a != b) {
         return true;
      }
   }
   return false;
}

static void displayBuffer() {
   glClear(GL_COLOR_BUFFER_BIT);

   glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
   glPointSize(3.0f);
   glBegin(GL_POINTS);

   vec2 mousePos (ps.cursorX, ps.cursorY);
   mousePos = screen2view(mousePos);
   //printf("%f \n",  mousePos.x);

   std::vector<vec2>::iterator it;
   for (it = ps.points.begin(); it != ps.points.end(); it++) {
      if (ps.drawDots) {
         glVertex2f((*it).x, (*it).y);
      }
   }

   int size = ps.points.size();
   int bezierNum = bezierNumFromVertNum(size);
   if (isFifthPoint(size) || (ps.inBlackhole && bezierNum != ps.blackholeEnterer)) {
      vec2 b, a;
      bool doTangent = findLastTwoNonzero(a, b);
      if (doTangent) {
         glColor4f(0.0f, 0.0f, 1.0f, 1.0f);
         mousePos = projectPointOntoLine(b, a, mousePos);
      }
   }
   if (ps.drawDots) {
      glVertex2f(mousePos.x, mousePos.y);
   }
   glEnd();

   glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
   drawBezier((drawmode)ps.renderMode);
   //glFlush();
   //glFinish();


   glutSwapBuffers();
}

static void keyboard(unsigned char key, int x, int y) {
   int update = 0;
   switch (key) {
      case 46:
         ps.drawDots = !ps.drawDots;
         break;
      case 49:
         ps.renderMode = PARAMETRIC;
         break;
      case 50:
         ps.renderMode = DE_CASTELJAU;
         break;
      case 51:
         ps.renderMode = OPENGL;
         break;
      case 52:
         ps.renderMode = SUBDIVISION;
         break;
      case 43:
      case 61:
         ps.level++;
         update = 1;
         break;
      case 45:
         ps.level--;
         if (ps.level <= 0) {
            ps.level = 1;
         }
         update = 1;
         break;
   }
   if (update) {
      printf("\rCurrent level: %d    ", ps.level);
      fflush(stdout);
   }
}

static void mouse(int button, int state, int x, int y) {
   vec2 mousePos (x,y); //(ps.cursorX, ps.cursorY);
   mousePos = screen2view(mousePos);
   int size = ps.points.size();

   int bezierNum = bezierNumFromVertNum(size);
   if (isFifthPoint(size) || (ps.inBlackhole && bezierNum != ps.blackholeEnterer)) {
      vec2 a, b;
      bool doTangent = findLastTwoNonzero(a, b);
      if (doTangent) {
         //printf("Projecting <%f, %f> onto line <%f, %f> <%f, %f>\n", mousePos.x, mousePos.y, a.x, a.y, b.x, b.y);
         mousePos = projectPointOntoLine(b, a, mousePos);
      }
   }
   if (button == 0 && state == 0) {
      int bezierNumPrev = bezierNumFromVertNum(size-1);
      if (size > 0)  {
         if (mousePos == ps.points.back() && !ps.inBlackhole && ps.blackholeEnterer != bezierNumPrev) {
            ps.blackholeEnterer = bezierNumPrev;
            ps.inBlackhole = true;
            //printf("In blackhole! (enterer = %i)\n", bezierNumPrev);
         }
         else if (ps.inBlackhole && mousePos != ps.points.back()) {
            ps.blackholeEnterer = -1;
            ps.inBlackhole = false;
            //printf("escaped blackhole!\n");
         }
      }
      ps.points.push_back(vec2(mousePos.x, mousePos.y));
      //printf("Adding point <%f, %f>\n", mousePos.x, mousePos.y);
   }

}

void passiveMotion(int x, int y) {
   ps.cursorX = x;
   ps.cursorY = y;
}

void timer(int t) {
   glutPostRedisplay();
   glutTimerFunc(10, timer, 0);
}

int main(int argc, char** argv) {

   printf("Keyboard controls:\n\n");
   printf("+/- to change # of steps or level of subdivision\n");
   printf("Modes:\n");
   printf("   1 Key - Use parametric scheme (default)\n");
   printf("   2 Key - Use De Casteljau scheme\n");
   printf("   3 Key - Use OpenGL scheme\n");
   printf("   4 Key - Use subdivision scheme\n");
   printf("\n\n");

   glutInit(&argc, argv);
   glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGB );
   glutInitWindowSize( 512, 512 ); 
   glutInitWindowPosition( 100, 100 );
   glutCreateWindow( "Curves" );

   glutPassiveMotionFunc(passiveMotion);

   glutDisplayFunc(displayBuffer);
   glutMouseFunc(mouse);
   glutKeyboardFunc(keyboard);
   glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
   timer(0);
   glutMainLoop();

   return 0;
}
