#include "BufferedDisplay.h"

void centerString(BufferedDisplay* g, const char* str,  int16_t x, int16_t y, int16_t* wOut = 0, int16_t* hOut = 0);
void centerRightString(BufferedDisplay* g, const char* str,  int16_t x, int16_t y, int16_t* wOut = 0, int16_t* hOut = 0);
void drawMultilineCenteredText(BufferedDisplay* g, const String& text, int x, int y, int maxWidth, int lineHeight, int* _w, int* _h);
void centerLeftString(BufferedDisplay* g, const char* str,  int16_t x, int16_t y, int16_t* wOut = 0, int16_t* hOut = 0);
void leftString(BufferedDisplay* g, const char* str,  int16_t x, int16_t y, int16_t* wOut = 0, int16_t* hOut = 0);