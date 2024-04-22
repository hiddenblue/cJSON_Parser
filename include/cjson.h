
#ifndef __CJSON_H
#define __CJSON_H

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
const char *parse_string(jsonNode *node, const char *string);
const char *parse_number(jsonNode *node, const char *string);
const char *parser_array(jsonNode *node, const char *string);
const char *parser_object(jsonNode *node, const char *stirng);

const char *skipWhiteSpace(const char *input);

#endif
