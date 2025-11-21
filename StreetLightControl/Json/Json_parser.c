#include "Json_parser.h"

/**
 * @brief 解析调光控制命令 (dim command)
 * @param root: cJSON对象根节点
 * @return 0: 成功, -1: 失败
 */
int parse_dim_command(cJSON *root)
{
    cJSON *command = cJSON_GetObjectItemCaseSensitive(root, "command");
    cJSON *value = cJSON_GetObjectItemCaseSensitive(root, "value");
    cJSON *group = cJSON_GetObjectItemCaseSensitive(root, "group");
    cJSON *lamp_id = cJSON_GetObjectItemCaseSensitive(root, "lamp_id");
    cJSON *timestamp = cJSON_GetObjectItemCaseSensitive(root, "timestamp");

    // 检查是否为dim命令
    if (!cJSON_IsString(command) || strcmp(command->valuestring, "dim") != 0) {
        return -1;
    }
    
    // 检查必需字段
    if (!cJSON_IsNumber(value) || !cJSON_IsString(group) || 
        !cJSON_IsString(lamp_id) || !cJSON_IsString(timestamp)) {
        my_printf(USART1, "Invalid dim command format\r\n");
        return -1;
    }

    // 处理调光命令
    my_printf(USART1, "Dim Command Received:\r\n");
    my_printf(USART1, "  Lamp ID: %s\r\n", lamp_id->valuestring);
    my_printf(USART1, "  Group: %s\r\n", group->valuestring);
    my_printf(USART1, "  Brightness Value: %d%%\r\n", value->valueint);
    my_printf(USART1, "  Timestamp: %s\r\n", timestamp->valuestring);

    // 执行调光操作

    return 0;
}

/**
 * @brief 解析开灯控制命令 (switch command)
 * @param root: cJSON对象根节点
 * @return 0: 成功, -1: 失败
 */
int parse_switch_command(cJSON *root)
{
    cJSON *command = cJSON_GetObjectItemCaseSensitive(root, "command");
    cJSON *value = cJSON_GetObjectItemCaseSensitive(root, "value");
    cJSON *group = cJSON_GetObjectItemCaseSensitive(root, "group");
    cJSON *lamp_id = cJSON_GetObjectItemCaseSensitive(root, "lamp_id");
    cJSON *timestamp = cJSON_GetObjectItemCaseSensitive(root, "timestamp");

    // 检查是否为switch命令
    if (!cJSON_IsString(command) || strcmp(command->valuestring, "switch") != 0) {
        return -1;
    }

    // 检查必需字段
    if (!cJSON_IsString(value) || !cJSON_IsString(group) || 
        !cJSON_IsString(lamp_id) || !cJSON_IsString(timestamp)) {
        my_printf(USART1, "Invalid switch command format\r\n");
        return -1;
    }
    
    // 处理开关命令
    my_printf(USART1, "Switch Command Received:\r\n");
    my_printf(USART1, "  Lamp ID: %s\r\n", lamp_id->valuestring);
    my_printf(USART1, "  Group: %s\r\n", group->valuestring);
    my_printf(USART1, "  Switch Value: %s\r\n", value->valuestring);
    my_printf(USART1, "  Timestamp: %s\r\n", timestamp->valuestring);

    // 执行开关操作

    return 0;
}

/**
 * @brief 解析MQTT接收到的JSON控制命令
 * @param json_str: 接收到的JSON字符串
 * @return 0: 成功, -1: 失败
 */
int parse_control_command(const char* json_str)
{
    cJSON *root = cJSON_Parse(json_str);
    if(root == NULL)
    {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL)
        {
            my_printf(USART1, "Error before: %s\r\n", error_ptr);
        }
        return -1;
    }

    // 解析dim命令
    if (parse_dim_command(root) == 0)
    {
        cJSON_Delete(root);
        return 0;
    }

     // 尝试解析switch命令
    if (parse_switch_command(root) == 0) {
        cJSON_Delete(root);
        return 0;
    }

    my_printf(USART1, "Invalid command\r\n");
    cJSON_Delete(root);
    return -1;
}

int parse_status_command(const char* json_str)
{
    
    return 0;
}
