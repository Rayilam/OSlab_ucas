#include <os/console.h>

//GETCHAR
char console_getchar() {
    int ch;
    while ((ch=bios_getchar()) == -1);
    return ch;
}
//显回
char CONSOLE_GETPRINT(int bs) {
    char ch = console_getchar();
    // echo
    switch (ch) {
    case '\b': case 127:   // delete
        // "\33[K": clear the content from the cursor to the end
        if (bs)
            bios_putstr("\b\33[K");
        ch = 127;
        break;
    case '\n': case '\r':  // new line
        bios_putstr("\n\r");
        ch = '\n';
        break;
    default:               // echo
        bios_putchar(ch);
    }
    return ch;
}
//GET a line
int CONSOLE_GETLINE(char buf[], int bufsize) {
    int i = 0;
    while ((buf[i]=CONSOLE_GETPRINT(i>0)) != '\n' && i < bufsize-1) {
        if (buf[i] == 127) {
            if (i>0) i--;
        }
        else i++;
    }
    buf[i] = '\0';
    return i;
}
//'_'
int format(char *fmt, char *buf, int len, char *val) {
    int skip = 0, escape = 0;
    for (int i=0; i<len; i++) {
        *buf = fmt[i];
        if (!escape && fmt[i] == '\\') {
            escape = 1;
        }
        else if (!escape && fmt[i] == '_') {
            if (!skip) {
                *buf++ = *val++;
                skip += *val==0;
            } else {
                skip ++;
            }
        } else {
            buf++;
            escape = 0;
        }
    }
    return len - skip;
}
//PRINT
void CONSOLE_PRINT(char *fmt, char *val) {
    int len = strlen(fmt);
    char buf[len+2];
    len = format(fmt, buf, len, val);
    buf[len+1] = '\0';
    bios_putstr(buf);
}

