#include <stdio.h>
#include <stdarg.h>

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"

int INFO(const char *format, ...)
{
    printf("[INFO] ");
    
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    printf("\r\n");
}

int SUCCESS(const char *format, ...)
{
    printf("%s[SUCCESS] ", KGRN);
    
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    printf("%s\r\n", KNRM);
}

int WARNING(const char *format, ...)
{
    printf("%s[WARNING] ", KYEL);
    
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    printf("%s\r\n", KNRM);
}

int ERROR(const char *format, ...)
{
    printf("%s[ERROR] ", KRED);
    
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    printf("%s\r\n", KNRM);
}

int FATAL(const char *format, ...)
{
    printf("%s[FATAL] ", KRED);
    
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    printf("%s\r\n", KNRM);

    while(1);
}

