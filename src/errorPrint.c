
#include "errorPrint.h"

void DieWithUserMessage(const char *msg, const char *detail)
{
    fputs(msg, stderr);
    fputs(':  ', stderr);
    fputs(detail, stderr);
    fputs("\n", stderr);
    exit(-1);
}

void DieWithSystemMessage(const char *msg)
{
    fputs(msg, stderr);
    fputs("\n", stderr);
    exit(-1);
}