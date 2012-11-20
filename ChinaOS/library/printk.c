/******************************************************Copyright (c)**************************************************
**                                              胆怯是阻止一切可能的根本缘由
**
**                                             E-Mail: ChinaFengliang@163.com
**
**---------File Information-------------------------------------------------------------------------------------------
** File name:           printk.c
**
** Last version:        V1.00
** Descriptions:        库文件之打印组件.
** Hardware platform:   lpc23xx
** SoftWare platform:   ChinaOS
**--------------------------------------------------------------------------------------------------------------------
** Created by:          Fengliang
** Created date:        2009-10-07
** Version:             V1.00
** Descriptions:        The original version
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Version:
** Descriptions:
**
*********************************************************************************************************************/


/*********************************************************************************************************************
                                                   头文件区
*********************************************************************************************************************/
#include    <stdarg.h>
#include    <stdio.h>
#include    "./sysio/sysio.h"
#include    <include/macro.h>

#define ZEROPAD 1       /* pad with zero */
#define SIGN    2       /* unsigned/signed long */
#define PLUS    4       /* show plus */
#define SPACE   8       /* space if plus */
#define LEFT    16      /* left justified */
#define SPECIAL 32      /* 0x */
#define LARGE   64      /* use 'ABCDEF' instead of 'abcdef' */

#define is_digit(c) ((c) >= '0' && (c) <= '9')

static int skip_atoi(const char **s)
{
    int i=0;

    while (is_digit(**s))
        i = i*10 + *((*s)++) - '0';
    return i;
}

unsigned int strnlen(const char * s, unsigned int count)
{
    const char *sc;

    for (sc = s; count-- && *sc != '\0'; ++sc)
        /* nothing */;
    return sc - s;
}

INLINE int do_div(int *n, int base)
{
    int res;

    res = ((unsigned long) *n) % (unsigned) base;
    *n /= (unsigned) base;
    return res;
}

static char * number(char * str, long num, int base, int size, int precision,int type)
{
    char c,sign,tmp[66];
    const char *digits="0123456789abcdefghijklmnopqrstuvwxyz";
    int i;

    if (type & LARGE)
        digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    if (type & LEFT)
        type &= ~ZEROPAD;
    if (base < 2 || base > 36)
        return 0;
    c = (type & ZEROPAD) ? '0' : ' ';
    sign = 0;
    if (type & SIGN) {
        if (num < 0) {
            sign = '-';
            num = -num;
            size--;
        } else if (type & PLUS) {
            sign = '+';
            size--;
        } else if (type & SPACE) {
            sign = ' ';
            size--;
        }
    }
    if (type & SPECIAL) {
        if (base == 16)
            size -= 2;
        else if (base == 8)
            size--;
    }
    i = 0;
    if (num == 0)
        tmp[i++]='0';
    else while (num != 0)
        tmp[i++] = digits[do_div((int*)&num,base)];
    if (i > precision)
        precision = i;
    size -= precision;
    if (!(type&(ZEROPAD+LEFT)))
        while(size-->0)
            *str++ = ' ';
    if (sign)
        *str++ = sign;
    if (type & SPECIAL) {
        if (base==8)
            *str++ = '0';
        else if (base==16) {
            *str++ = '0';
            *str++ = digits[33];
        }
    }
    if (!(type & LEFT))
        while (size-- > 0)
            *str++ = c;
    while (i < precision--)
        *str++ = '0';
    while (i-- > 0)
        *str++ = tmp[i];
    while (size-- > 0)
        *str++ = ' ';
    return str;
}

/*********************************************************************************************************************
** Function name:           vsprintf
** Descriptions:            
** Input parameters:        
** Output parameters:       
** Returned value:          ==OK : 操作成功
**                          !=OK : 操作失败(包含出错信息)
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Feng Liang
** Created Date:            2011-11-12  22:53:52
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
int vsprintf(char *buf, const char *fmt, va_list args)
{
    int                 len;
    unsigned long       num;
    int                 i, base;
    char               *str;
    const char         *s;

    int flags;                                                              // flags to number() 

    int field_width;                                                        // width of output field 
    int precision;                                                          // min. # of digits for integers; max
                                                                            //number of chars for from string 
    int qualifier;                                                          // 'h', 'l', or 'L' for integer fields 

    for (str=buf ; *fmt ; ++fmt) {
        if (*fmt != '%') {
            *str++ = *fmt;
            continue;
        }

        /* process flags */
        flags = 0;
        repeat:
            ++fmt;      /* this also skips first '%' */
            switch (*fmt) {
                case '-': flags |= LEFT; goto repeat;
                case '+': flags |= PLUS; goto repeat;
                case ' ': flags |= SPACE; goto repeat;
                case '#': flags |= SPECIAL; goto repeat;
                case '0': flags |= ZEROPAD; goto repeat;
                }

        /* get field width */
        field_width = -1;
        if (is_digit(*fmt))
            field_width = skip_atoi(&fmt);
        else if (*fmt == '*') {
            ++fmt;
            /* it's the next argument */
            field_width = va_arg(args, int);
            if (field_width < 0) {
                field_width = -field_width;
                flags |= LEFT;
            }
        }

        /* get the precision */
        precision = -1;
        if (*fmt == '.') {
            ++fmt;
            if (is_digit(*fmt))
                precision = skip_atoi(&fmt);
            else if (*fmt == '*') {
                ++fmt;
                /* it's the next argument */
                precision = va_arg(args, int);
            }
            if (precision < 0)
                precision = 0;
        }

        /* get the conversion qualifier */
        qualifier = -1;
        if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L') {
            qualifier = *fmt;
            ++fmt;
        }

        /* default base */
        base = 10;

        switch (*fmt) {
        case 'c':
            if (!(flags & LEFT))
                while (--field_width > 0)
                    *str++ = ' ';
            *str++ = (unsigned char) va_arg(args, int);
            while (--field_width > 0)
                *str++ = ' ';
            continue;

        case 's':
            s = va_arg(args, char *);
            if (!s)
                s = "<NULL>";

            len = strnlen(s, precision);

            if (!(flags & LEFT))
                while (len < field_width--)
                    *str++ = ' ';
            for (i = 0; i < len; ++i)
                *str++ = *s++;
            while (len < field_width--)
                *str++ = ' ';
            continue;

        case 'p':
            if (field_width == -1) {
                field_width = 2*sizeof(void *);
                flags |= ZEROPAD;
            }
            str = number(str,
                (unsigned long) va_arg(args, void *), 16,
                field_width, precision, flags);
            continue;


        case 'n':
            if (qualifier == 'l') {
                long * ip = va_arg(args, long *);
                *ip = (str - buf);
            } else {
                int * ip = va_arg(args, int *);
                *ip = (str - buf);
            }
            continue;

        case '%':
            *str++ = '%';
            continue;

        /* integer number formats - set up the flags and "break" */
        case 'o':
            base = 8;
            break;

        case 'X':
            flags |= LARGE;
        case 'x':
            base = 16;
            break;

        case 'd':
        case 'i':
            flags |= SIGN;
        case 'u':
            break;

        default:
            *str++ = '%';
            if (*fmt)
                *str++ = *fmt;
            else
                --fmt;
            continue;
        }
        if (qualifier == 'l')
            num = va_arg(args, unsigned long);
        else if (qualifier == 'h') {
            num = (unsigned short) va_arg(args, int);
            if (flags & SIGN)
                num = (short) num;
        } else if (flags & SIGN)
            num = va_arg(args, int);
        else
            num = va_arg(args, unsigned int);
        str = number(str, num, base, field_width, precision, flags);
    }
    *str = '\0';
    return str-buf;
}

/*********************************************************************************************************************
** Function name:           printk
** Descriptions:            内核调试信息打印函数
** input parameters:        Format   : 格式化字符串
**                          ...      : 扩展参数
** output parameters:       none
** Returned value:          输出字符串长度
** Created by:
** Created Date:
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
*********************************************************************************************************************/
int printk (const char *Format, ...)
{
    va_list      args;
    unsigned int i;
    char aPrintBuf[256];

    va_start(args, Format);

    /* For this to work, PrintBuf must be larger than
     * anything we ever want to print.
     */
    i = vsprintf(aPrintBuf, Format, args);
    va_end(args);

    /* Print the string
     * eg: printf("%s", aPrintBuf);
     */
    prints(aPrintBuf);

    return i;
}

/*********************************************************************************************************************
  END OF FILE
*********************************************************************************************************************/

