#include "cjson.h"
#include <stdio.h>

int main(int argc, char *argv[])
{

    char str[] = "{ \
            \"rootExperience\": { \
                \"configRef\": {   \
                    \"experienceType\": \"EntryPointViewsWC\", \
                        \"instanceSrc\": \"default\" \
                    } }}";

    JSON_ParseWithOpts(str, "", "");
    return 0;
}
