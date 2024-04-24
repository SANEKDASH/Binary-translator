#ifndef colorprint

#define colorprint

typedef enum
{
    kBlack   = 30,
    kRed     = 31,
    kGreen   = 32,
    kYellow  = 33,
    kBlue    = 34,
    kMagenta = 35,
    kCyan    = 36,
    kWhite   = 37
} ColorCode_t;

void ColorPrintf(ColorCode_t color_code, const char *fmt, ...);

#endif
