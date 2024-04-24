
#ifndef __CJSON_H
#define __CJSON_H

// NodeType 如果从0开始，好像会和默认的NULL 等于(void *)0有点冲突
// 默认就变成了NULL类型节点，所以从1开始可能更好一点
typedef enum
{
    JSON_False = 1,
    JSON_True = 2,
    JSON_NULL = 3,
    JSON_Number = 4,
    JSON_String = 5,
    JSON_Array = 6,
    JSON_Object = 7

} NodeType;

#define JSON_AddFalseToObject(object, name) \
    JSON_AddNodeToObject(object, name, JSON_CreateFalse())

#define JSON_AddTrueToObject(object, name) \
    JSON_AddNodeToObject(object, name, JSON_CreateTrue())

#define JSON_AddNullToObject(object, name) \  
    JSON_AddNodeToObject(object, name, JSON_CreateNull())

#define JSON_AddNumberToObject(object, name) \
    JSON_AddNodeToObject(object, name, JSON_CreateNumber())

#define JSON_AddStringToObject(object, name) \
    JSON_AddNodeToObject(object, name, JSON_CreateString())

#define JSON_SetIntValue(object, value) \
    (object) ? (object)->valueInt = (object)->valueDouble = (value) : (value)

#define JSON_SetIntValue(object, value) \
    (object) ? (object)->valueInt = (object)->valueDouble = (value) : (value)

typedef struct jsonNode
{
    // the key of of this node
    char *key;

    // dual chain linklist node pointer;
    struct jsonNode *prev, *next;

    // pointer to sub node;
    struct jsonNode *child;

    // speicify the node type and content
    NodeType type;

    // if this node has no sub node. these are content in value;
    char *valueString;
    int valueInt;
    double valueDouble;

    // array and object need sub node so that contain long string and number
} jsonNode;

/**
 * @brief parser string to json that could be easily read
 * @param target A string could be parser by json
 * @return A pointer to the root the jsonNode
 */
jsonNode *JSON_Parse(const char *node);

/**
 * @brief convert a jsonNode type node to printable string
 * @param Node  jsonNode * pointer to a Node
 * @return A char * type pointer to a printable string
 * remember to free the member of Node return by JSON_Print()
 */
char *JSON_Print(const jsonNode *node);

/**
 * @brief recursively delete the jsonNode of given node
 * @param jsonNode *node
 * @return 0 for Suceess or -1 for fail.
 */
int JSON_Delete(jsonNode *node);

/**
 * @brief  create a jsonNode and its Nodetype is json Object
 * @param void
 * @return a jsonNode type pointer
 */
jsonNode *JSON_CreateObject(void);

/**
 * @brief  create a string type jsonNode
 * @param string char *  type
 * @return  jsonNode type pointer
 */
jsonNode *JSON_CreateString(const char *string);

/**
 * @brief  create a  type jsonNode
 * @param number double type
 * @return  jsonNode type pointer
 * int type jsonNode also create from double type.
 */
jsonNode *JSON_CreateNumber(double number);

/**
 * @brief  create a  type jsonNode
 * @param None
 * @return  jsonArray jsonNode pointer   */
jsonNode *JSON_CreateArray(void);

/**
 * @brief  create a  bool type jsonNode
 * @param int bool 1 or 0
 * @return  jsonBoll type jsonNode pointer   */
jsonNode *JSON_CreateBool(int Bool);

/**
 * @brief  create a  bool type jsonNode
 * @param int bool 1 or 0
 * @return  jsonBoll type jsonNode pointer   */
jsonNode *JSON_CreateFalse(void);

/**
 * @brief  create a  bool type jsonNode
 * @param int bool 1 or 0
 * @return  jsonBoll type jsonNode pointer   */
jsonNode *JSON_CreateTrue(void);

/**
 * @brief  create a  bool type jsonNode
 * @param int bool 1 or 0
 * @return  jsonBoll type jsonNode pointer   */
jsonNode *JSON_CreateNull(void);

/**
 * @brief add a child node to parent object type node
 * @param parent node pointer
 * @param string node name
 * @param child  node pointer
 * @return 0 for success or -1 for fail
 * if parent jsonNode is not an empty Node, attach the child to its node linklist
 */
int JSON_AddNodeToObject(jsonNode *parent, char *string, jsonNode *child);

/**
 * @brief
 * @param array
 * @param item
 * @return
 */
int JSON_AddNodeToArray(jsonNode *array, jsonNode *item);

// set attr get attr

jsonNode *JSON_ParseWithOpts(const char *node, const char **return_parse_end, int required_null_terminated);

jsonNode *JSON_New_Item();
const char *parse_Node(jsonNode *node, const char *value);
/**
 * @brief 可以对json中各种""中的字符进行解析，放在传入的node的valueString当中
 * 默认修改type为JSON_String类型
 * @param node
 * @param string
 * @return 返回最后成功解析的位置
 */
const char *parse_string(jsonNode *node, const char *string);
/**
 * @brief 对parse_node当中遇到的数字进行处理，默认将数字存储在valueDouble节点当中，int部分存在valueInt当中。
 * 默认type是JSON_Number类型
 * @param node
 * @param string
 * @return
 */
const char *parse_number(jsonNode *node, const char *string);

/*
 然后里面调用paser_node对内容进行解析，新配新节点，然后储存遇到的字符。
 遇到,逗号会一直重复解析，分配新节点

*/

/**
 * @brief 对parse_node当中的array进行处理，默认修改传入node的type为JSON_Array

 * @param node
 * @param string
 * @return 返回最后成功解析的位置，默认就是] 后面一个位置
 */
const char *parser_array(jsonNode *node, const char *string);
/**
 * @brief 对位于:后面的{或者最开始的{内容进行解析
 * @param node
 * @param stirng
 * @return
 */
const char *parser_object(jsonNode *node, const char *stirng);

const char *skipWhiteSpace(const char *input);

char *print_value(jsonNode *node, int depth, int fmt);

char *JSON_StrDup(const char *string);

char *print_string(jsonNode *node);

char *print_number(jsonNode *node);

char *print_array(jsonNode *node, int depth, int fmt);

char *print_object(jsonNode *node, int depth, int fmt);

char *print_node(jsonNode *node);

#endif
