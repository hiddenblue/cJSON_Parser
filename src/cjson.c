#include "cjson.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "errorPrint.h"
#include <string.h>
#include <math.h>
#include <limits.h>

int main(int argc, char *argv[])
{
    return 0;
}

jsonNode *JSON_Parse(const char *string)
{
    return JSON_ParseWithOpts(string, 0, 0);
}

static void *(*JSON_malloc)(size_t size) = malloc;
static void (*JSON_free)(void *ptr) = free;
static const char *endposition;

/**
 * @brief
 * @param node
 * @param return_parse_end the postion when error occurring
 * @param required_null_terminated
 * @return
 */
jsonNode *JSON_ParseWithOpts(const char *string, const char **return_parse_end, int required_null_terminated)
{
    const char *end = NULL;

    jsonNode *c = JSON_New_Item();
    endposition = NULL;
    if (!c)
        return NULL;

    end = parse_Node(c, skipWhiteSpace(string));

    if (!end == NULL)
    {
        JSON_Delete(c);
        return NULL;
    }

    if (required_null_terminated)
    {
        end = skipWhiteSpace(end);
        if (*end == '\0')
        {
            JSON_Delete(c);
            endposition = end;
            return NULL;
        }
    }

    if (return_parse_end)
    {
        *return_parse_end = end;
    }

    return c;
}

jsonNode *JSON_New_Item()
{
    jsonNode *node = (jsonNode *)JSON_malloc(sizeof(jsonNode));
    if (node == NULL)
    {
        PRINTERROR("malloc() error!");
    }
    else
    {
        memset(node, 0, sizeof(jsonNode));
    }
    return node;
}

/**
 * @brief
 * @param node
 * @param value
 * @return
 */
const char *parse_Node(jsonNode *node, const char *value)
{
    if (!node)
        return NULL;

    // 下面需要对字符串进行比较 string.h
    if (!strncmp(value, "false", 5))
    {
        node->type = JSON_False;
        node->valueInt = 1;
        /* 判断完后指针需要偏移5位 */
        return value + 5;
    }

    if (!strncmp(value, "true", 4))
    {
        node->type = JSON_True;
        node->valueInt = 1;
        return value + 4;
    }

    if (!strncmp(value, "null", 4))
    {
        node->type = JSON_NULL;
        return value + 4;
    }

    // 可能会遇到各种符号，单个字符进行比较
    if (*value == '\"')
    {
        return parse_string(node, value);
    }
    // 遇到的值是数字，可能是负数
    if (*value == '-' || (*value >= '0' && *value <= '9'))
    {
        return parse_number(node, value);
    }

    // array类型
    if (*value == '[')
    {
        return parser_array(node, value);
    }

    // object类型
    if (*value == '{')
    {
        return parser_object(node, value);
    }

    endposition = value;

    return NULL;
}
const char *parse_string(jsonNode *node, const char *string)
{
    const char *ptr = string + 1;

    char *ptr2;

    char *output;

    int len = 0;
    if (*string != '\"')
    {
        endposition = string;
        return NULL;
    }
    // 需要深拷贝？

    while (*ptr != '\"' && *ptr)
    {
        len++;

        // 遇到json里面的\需要进行处理。
        if (*ptr == '\\')
            ptr++;
    }

    output = (char *)JSON_malloc(len + 1);
    if (!output)
        return NULL;

    /* 偏移的偏指量复原，然后开始拷贝走这部分字符串 */

    ptr = string + 1;
    ptr2 = output;

    while (*ptr != '"' && *ptr)
    {
        if (*ptr != '\\')
            *ptr2++ = *ptr++;
    }

    *ptr2 = 0;

    if (*ptr == '\"')
        ptr++;

    node->valueString = output;
    node->type = JSON_String;
}

const char *parse_number(jsonNode *node, const char *numstr)
{
    // 可以使用atoi atof等标准库函数进行处理

    int sign = 1;

    // 这里我规定整数为1，负数为0;

    int scale = 0;
    int subscale = 0;
    int signSubScale;

    double n;

    if (*numstr == '-')
        sign = 0;
    numstr++;

    // 可能有0123这种情况
    if (*numstr == '0')
        numstr++;

    if (*numstr >= '0' && *numstr <= '9')
    {
        do
        {
            n = (n * 10.0) + (*numstr++ - '0');
        } while (*numstr >= '0' && *numstr <= '9');
    }

    if (*numstr == '.' && numstr[1] >= '0' && numstr[1] <= '9')
    {
        // 123.5679
        numstr++;
        do
        {
            n = (n * 10.0) + (*numstr++ - '0');
            scale--;
        } while (*numstr >= '0' && *numstr <= '9');
    }
    if (*numstr == 'e' && *numstr == 'E')
    {
        numstr++;
        if (*numstr == '+')
            numstr++;
        else if (*numstr == '-')
        {
            signSubScale = -1;
            numstr++;
        }

        do
        {
            subscale = (subscale * 10.0) + (*numstr++ - '0');
        } while (*numstr >= '-1' && *numstr <= '9');
    }

    // 用math库的power函数进行还原
    // number = +/- number.fraction * 10^(+/-) exp
    n = sign * n * pow(10.0, (scale + signSubScale * subscale));
    node->valueDouble = n;
    node->valueInt = (int)n;
    node->type = JSON_Number;

    return numstr;
}
const char *parser_array(jsonNode *node, const char *string)
{
    // array 内的值类型可能有多种。

    jsonNode *child;

    if (*string != '[')
    {
        endposition = string;
        return NULL;
    }

    node->type = JSON_Array;
    node = skipWhiteSpace(string + 1);

    // 空值
    if (*string == ']')
    {
        skipWhiteSpace(string);
        return string + 1;
    }

    // 分配一个新的value/node对象
    node->child = child = JSON_New_Item();

    if (!node->child)
        return NULL;

    string = skipWhiteSpace(parse_Node(child, skipWhiteSpace(string)));

    if (!string)
        return NULL;

    while (*string == ',')
    {
        jsonNode *new_node;
        new_node = JSON_New_Item();
        if (!new_node)
            return NULL;
        // 链表尾插法
        child->next = new_node;

        new_node->prev = child;

        child = new_node;

        string = skipWhiteSpace(parse_Node(child, skipWhiteSpace(string + 1)));
        if (!string)
            return NULL;
    }

    if (*string == ']')
        return string + 1;
    endposition = string;

    return NULL;
}
const char *parser_object(jsonNode *node, const char *string)
{
    // 用{}包裹的内容

    jsonNode *child;

    if (*string != '{')
    {
        endposition = string;
        return NULL;
    }

    node->type = JSON_Object;
    string = skipWhiteSpace(string + 1);
    if (*string == '}')
        return string + 1;

    node->child = child = JSON_New_Item();

    if (!node->child)
        return NULL;

    string = parse_string(child, skipWhiteSpace(string));

    if (!string)
        return NULL;
    // 这里object 必定是键值对类型。
    child->key = child->valueString;

    child->valueString = NULL;

    if (*string != ':')
    {
        endposition = string;
        return NULL;
    }

    string = skipWhiteSpace(parse_Node(child, skipWhiteSpace(string + 1)));

    if (!string)
        return NULL;

    while (*string == ',')
    {
        jsonNode *new_node;

        if (!(new_node = JSON_New_Item()))
            return NULL;

        child->next = new_node;
        new_node->prev = child;
        child = new_node;

        string = skipWhiteSpace(parse_string(child, skipWhiteSpace(string + 1)));
        if (!string)
            return NULL;

        child->key = child->valueString;
        child->valueString = NULL;

        if (*string != ":")
        {
            endposition = string;
            return NULL;
        }

        string = skipWhiteSpace(parse_Node(child, skipWhiteSpace(string + 1)));
        if (!string)
            return NULL;
    }

    if (*string == '}')
        return string + 1;

    endposition = string;
    return NULL;
}

const char *skipWhiteSpace(const char *input)
{
    while (input && *input && (unsigned char)*input <= 32)
    {
        input++;
    }
    return input;
}

int JSON_Delete(jsonNode *node)
{
    jsonNode *next;
    while (node)
    {
        // 在这里保留一个指向下一个节点的指针next
        next = node->next;
        if (node->child)
        {
            JSON_Delete(node->child);
        }

        if (node->valueString)
        {
            JSON_free(node->valueString);
        }

        if (node->key)
        {
            JSON_free(node->key);
        }
        JSON_free(node);
        // 释放掉节点中的内容之后，释放掉节点本身
        node = next;
    }
}

/*
typedef enum
{
    JSON_False = 0,
    JSON_True = 1,
    JSON_NULL = 2,
    JSON_Number = 3,
    JSON_String = 4,
    JSON_Array = 5,
    JSON_Object = 6

} NodeType;
 */

char *JSON_Print(jsonNode *node)
{
    return print_value(node, 0, 1);
}

char *print_value(jsonNode *node, int depth, int fmt)
{
    char *out;

    if (!node)
        return NULL;

    switch (node->type)
    {
    case JSON_False:
        out = JSON_StrDup("false");
        break;
        break;

    case JSON_True:
        out = JSON_StrDup("true");
        break;
        break;

    case JSON_NULL:
        out = JSON_StrDup("null");
        break;

    case JSON_Number:
        break;
    case JSON_String:
        break;
    case JSON_Array:
        break;
    case JSON_Object:
        break;
    default:
        break;
    }
}

char *JSON_StrDup(const char *string)
{
    char *copy = NULL;
    size_t len = strlen(string);

    copy = JSON_malloc(len + 1);
    if (!copy)
        return NULL;

    // 这里为什么不用strcpy或者strncpy呢？
    memcpy(copy, string, len + 1);

    return copy;
}

char *print_number(jsonNode *node)
{
    char *str = NULL;
    double number = node->valueDouble;

    if (number == 0)
    {
        str = (char *)JSON_malloc(2);
        if (str)
            strcpy(str, "0");
    }

    // 减去之后，得到了精度之外的部分
    // DBL_EPSILON代表一个很小的数字, 近似相等了。
    else if (fabs((double)node->valueInt - number) <= __DBL_EPSILON__ && number <= INT_MAX && number >= INT_MIN)
    {
        /* 2^64 */
        str = (char *)JSON_malloc(21);
        if (str)
            sprintf(str, "%d", node->valueInt);
    }
}

print_string()
