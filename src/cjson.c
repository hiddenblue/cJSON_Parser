#include "cjson.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "errorPrint.h"
#include <string.h>
#include <math.h>
#include <limits.h>

// int main(int argc, char *argv[])
// {
//     return 0;
// }

jsonNode *JSON_Parse(const char *string)
{
    return JSON_ParseWithOpts(string, 0, 0);
}

static void *(*JSON_malloc)(size_t size) = malloc;
static void (*JSON_free)(void *ptr) = free;

// endposition 是在解析失败的时候需要输出的一个当前解析到的最后的位置
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

    jsonNode *root = JSON_New_Item();
    endposition = NULL;
    if (!root)
        return NULL;

    end = parse_Node(root, skipWhiteSpace(string));

    if (*end != NULL)
    {
        JSON_Delete(root);
        return NULL;
    }

    if (required_null_terminated)
    {
        end = skipWhiteSpace(end);
        if (*end == '\0')
        {
            JSON_Delete(root);
            endposition = end;
            return NULL;
        }
    }

    if (return_parse_end)
    {
        *return_parse_end = end;
    }

    return root;
}

jsonNode *JSON_New_Item()
{
    // 包含了node动态分配和memset归零
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
 * @brief 可以对 json中：冒号后面的节点进行解析。对类型直接设置当前对应的类型。
 * @param node
 * @param value
 * @return 返回值是解析成功的最后的位置
 */
const char *parse_Node(jsonNode *node, const char *value)
{
    // 每一个成功解析最后解析到的位置要进行返回
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

    // 这里是整个程序递归返回的关键，在这里遇到value里面而"将进行parse_string，进而修改parse_object调用时的处理位置
    if (*value == '\"')
    {
        // 这里面没有设置node的type，需要prase_string自己在里面设置好type。
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

    // 解析失败，上面的情况都没有执行，返回NULL。
    endposition = value;

    return NULL;
}

const char *parse_string(jsonNode *node, const char *string)
{
    const char *ptr = string + 1;

    char *ptr2;

    char *output;

    int len = 0;

    // 对传入的string检验第一个字符是否是"符号，失败错误返回。
    if (*string != '\"')
    {
        endposition = string;
        return NULL;
    }
    // 需要深拷贝？ 对双引号里面的字符进行遍历

    while (*ptr != '\"' && *ptr)
    {
        len++;
        ptr++;

        // 遇到json里面的\需要进行处理。直接不计长度，直接跳过就行了。但是/符号需要保留
        if (*ptr == '\\')
            ptr++;
    }

    // 为需要拷贝的字符分配一片空间，额外增加一个空字符空间
    output = (char *)JSON_malloc(len + 1);
    if (!output)
        return NULL;

    /* 偏移的偏指量复原，然后开始拷贝走这部分字符串 */

    ptr = string + 1; // 从"后面的第一个字符开始复制
    ptr2 = output;

    while (*ptr != '"' && *ptr)
    {
        if (*ptr != '\\')
            *ptr2++ = *ptr++;
    }

    *ptr2 = 0; // 空字符

    if (*ptr == '\"')
        ptr++;

    node->valueString = output;
    // 这里面的string node默认type都为string，后面可能会改动
    node->type = JSON_String;
    return skipWhiteSpace(ptr);
}

const char *parse_number(jsonNode *node, const char *numstr)
{
    // 可以使用atoi atof等标准库函数进行处理

    int sign = 1;

    // 这里我规定整数为1，负数为0;

    int scale = 0;
    int subscale = 0;
    int signSubScale;

    double n = 0;

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
        } while (*numstr >= '1' && *numstr <= '9');
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
    // 这里object 必定是键值对类型。因为在string后方
    child->key = child->valueString;

    child->valueString = NULL;

    // 在这个冒号: 前面的空格被我在prase_string里面strip掉了
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

        // parse_string的返回值会修改当前处理的位置变量，往后挪动。
        string = skipWhiteSpace(parse_string(child, skipWhiteSpace(string + 1)));
        if (!string)
            return NULL;

        child->key = child->valueString;
        child->valueString = NULL;

        // 对冒号之前的部分进行解析 前面的调用parse_string，后面的调用parse_object

        if (*string != ':')
        {
            endposition = string;
            return NULL;
        }
        // parse_node 如果遇到后面是""这样的字符就会从parse_string返回给parse_node,然后也修改当前的处理位置
        // 也是整个程序递归返回的关键位置
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
    // 这里是否可以使用标准库中检查字符是否可以打印的函数呢？
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

char *JSON_Print(const jsonNode *node)
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
    else
    {
        // 处理浮点数
        str = (char *)JSON_malloc(64);
        if (str)
        {
            // 1234.0
            if ((fabs(floor(number) - number) <= __DBL_EPSILON__) && (fabs(number) < 1.0e60)) // 向下取整
            {
                sprintf(str, "%.0f", number);
            }
            else if (fabs(number) < 1.0e-6 || fabs(1.0e9))
                sprintf(str, "%e", number);
            else
                sprintf(str, "%f", number);
        }
    }

    return str;
}

char *print_string(jsonNode *node)
{
    // 对jsonstring类型的节点进行打印，其"aaaa":"bbbb" bbbb位于valueString节点当中
    char *str = node->valueString;

    unsigned char token;

    char *ptr = str;
    char *ptr2;
    char *out = 0;

    int flag = 0, len = 0;

    // 32位的值以下的是控制字符。

    for (ptr = str; *ptr; ptr++)
    {
        flag = (*ptr > 0) && (*ptr < 32) || (*ptr == '\\') ? 1 : 0;
    }

    if (!flag)
    {
        len = ptr - str;
        out = (char *)JSON_malloc(len + 2 + 1);
        if (!len)
            return NULL;

        ptr2 = out;

        *out++ = '\"';

        strcpy(ptr2, str);

        ptr2[len] = '\"';
        ptr2[len + 1] = '\0';

        return out;
    }

    if (!str)
    {
        out = (char *)JSON_malloc(3);
        if (!out)
            return NULL;
        strcpy(out, "\"\"");
        return out;
    }

    ptr = str;

    while ((token = *ptr) && ++len)
    {
        if (strchar("\"\n\t\b\r\\"), token)
        {
            len++;
        }
        else if (token < 32)
        {
            len += 5;
            ptr++;
        }
    }

    out = (char *)JSON_malloc(len + 3);

    ptr2 = out;
    ptr = str;

    *ptr2 = '\"';

    while (*ptr)
    {
        if ((unsigned char)*ptr > 31 && *ptr != '\"' && *ptr != '\\')
            *ptr2++ = *ptr++;
        else
        {
            *ptr2++ = '\\';
            switch (token = *ptr)
            {
            case '\\':
                *ptr2++ = '\\';
                break;
            case '\"':
                *ptr2++ = '\"';
                break;
            case '\t':
                *ptr2++ = '\t';
                break;
            case '\r':
                *ptr2++ = '\r';
                break;
            case '\n':
                *ptr2++ = '\n';
                break;
            case '\b':
                *ptr2++ = '\b';
                break;
            case '\f':
                *ptr2++ = 'f';
                break;

            default:
                sprintf(str, "u%04x", token);
                ptr2++;
                break;
            }
        }
    }

    *ptr2++ = '\"';
    *ptr2++ = '\0';

    return out;
}

char *print_array(jsonNode *node, int depth, int fmt)
{
    char **entries;

    char *output = NULL;
    char *ptr;

    int len = 5; // "[]"\0
    int i = 0;
    int isFail = 0;

    jsonNode *child = node->child;

    int numEntries = 0;

    while (child)
    {
        // 判断同级的节点数量
        numEntries++;

        child = child->next;
    }

    while (!numEntries)
    {
        output = (char *)JSON_malloc(3); // []\0
        if (output)
            strcpy(output, "[]");
        return output;
    }

    entries = (char *)JSON_malloc(numEntries * sizeof(char *));
    if (entries)
        return NULL;

    memset(entries, numEntries, numEntries * sizeof(char *));

    child = node->child; // 重新恢复以下

    char *ret;
    while (child)
    {
        ret = print_value(child, depth + 1, 1);
        entries[i++] = ret;
        if (ret)
        {
            len += strlen(ret) + 2 + (fmt ? 1 : 0);
        }
        else
            isFail = 1;
        child = child->next;
    }

    if (!isFail)
    {
        output = (char *)JSON_malloc(len);
    }
    if (!output)
        return NULL;

    if (isFail)
    {
        // 释放掉需要的函数
        for (int i = 0; i < numEntries; i++)
        {
            if (entries[i])
            {
                JSON_free(entries[i]);
            }
        }
        JSON_free(entries);
    }

    *output = '[';
    ptr = output + 1;
    *ptr = '\0';

    int templen = 0;
    for (i = 0; i < numEntries; i++)
    {
        templen = (entries[i]);
        memcpy(ptr, entries, templen);
        ptr += templen;

        // ,号的区别
        if (i != numEntries - 1)
        {
            *ptr++ = ',';
            if (fmt)
            {
                *ptr++ = ' ';
            }
            *ptr = '\0';
        }

        JSON_free(entries[i]);
    }

    JSON_free(entries);

    *ptr++ = ']';
    *ptr++ = '\0';

    // 或者用*ptr="" 可以吗？

    return output;
}

char *print_object(jsonNode *node, int depth, int fmt)
{
    // 思路和print_array差不多
    // key 和 value
    char **entries, **names = NULL;
}