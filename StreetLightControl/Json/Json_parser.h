#ifndef __JSON_PARSER_H__
#define __JSON_PARSER_H__ 

#include "cJSON.h"
#include "string.h"
#include "Serial.h"

int parse_dim_command(cJSON *root);
int parse_switch_command(cJSON *root);
int parse_control_command(const char* json_str);
int parse_status_command(const char* json_str);

#endif
