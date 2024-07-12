#include "common.h"

#define memcl(x) memset(x, 0, sizeof x)

typedef struct {
    char val_type[20];
    char value[256];
} t_object;

typedef struct
{
    const char* item_name;
    t_object obj;
} t_json_object;

t_object get_value_from_item(const char* data, const char* item_name) {
    t_object obj;

    memcl(obj.value);
    memcl(obj.val_type);

    if (data == NULL)
        puts("DATA NULL");

    if (item_name == NULL)
        puts("ITEM_NAME NULL");

    int item_index = (strstr(data, item_name) - data);

    bool started_reading_value = false;
    bool read_first = false;
    
    while (true) {
        const char c = data[item_index];
        const char next_c = data[item_index + 1]; 
        const size_t value_len = strlen(obj.value);

        if (strcmp(obj.val_type, "integer") && c == '.') {
            strcpy_s(obj.val_type, 20, "double");
            continue;
        }

        if (c == ':' && !started_reading_value) {
            started_reading_value = true;
            item_index++;
            continue;
        }

        if (started_reading_value && !read_first) {
            if (c == '"')
                strcpy_s(obj.val_type, 20, "string");
            else if (isdigit(c))
                strcpy_s(obj.val_type, 20, "integer");
            else if (c == 't' || c == 'f')
                strcpy_s(obj.val_type, 20, "boolean");
            else
                strcpy_s(obj.val_type, 20, "unknown");

            read_first = true;
            continue;
        }

        if (c == ',' || c == '\n' || c == '}' && started_reading_value) {
            if (c == data[strlen(data) - 1] || next_c == '"')
                break;
            else
                started_reading_value = false;
        }

        if (started_reading_value)
            if (c != '"')
                obj.value[value_len] = c;
        
        item_index++;
    }

    //printf_s("DEBUG: ITEM -> %s, VALUE = %s\n", item_name, obj.value);

    // check val type

    // if empty
    if (strcmp(obj.value, "") == 0) {
        strcpy_s(obj.val_type, 20, "string");
        return obj;
    }

    return obj;
}

void parse(FILE *file, t_json_object** ret)
{
    char file_data[2048];
    memcl(file_data);

    if (fread_s(&file_data, 2048, 1, 2048, file) < 1)
    {
        printf_s("Failed to read buffer");
        return;
    }

    // removes every space
    int file_num_char = strlen(file_data);
    int file_data_lenght = file_num_char - 1;
    bool started_on_string = false;

    char data[2048];
    memcl(data);
    int curr_data_index = 0;
    for (size_t i = 0; i < file_num_char; i++)
    {
        const char c = file_data[i];
        if (file_data[file_num_char] == c)
            break;

        if (c == ' ' || c == '\n' || c == '\t')
            continue;
        else {
            data[curr_data_index] = c;
            curr_data_index++;
        }
    }

    int data_num_char = strlen(data);
    int data_lenght = data_num_char - 1;

    FILE* newf;
    fopen_s(&newf, "../new.json", "w");
    fwrite(data, data_num_char, 1, newf);
    fclose(newf);

    if (data[0] != '{' || data[data_lenght] != '}')
    {
        printf_s("JSON file format not recognized");
        return;
    }

    int item_count = 0;
    int curr_index = 0;
    bool started_item_count = false;
    char buffer[256];
    char* items[256];
    memcl(buffer);
    memcl(items);

    while (true) {
        const char c = data[curr_index];

        if (curr_index == data_lenght)
            break;

        if (c == '"' && !started_item_count)
            if (data[curr_index - 1] != ':')
                started_item_count = true;

        if (started_item_count)
            if (c != '"' && c != '{' && c != '}' && c != ':' && c != ',')
                buffer[strlen(buffer)] = c;

        if (c == '"' && started_item_count && data[curr_index + 1] == ':') {
            // found an item an adding to item count
            items[item_count] = strdup(buffer);
            memcl(buffer);
                
            item_count++;
            started_item_count = false;
        }

        curr_index++;
    }

    //printf_s("Item count: %i\n", item_count);

   t_json_object results[item_count];
   
    for (int i = 0; i < item_count; i++) {
        t_object obj = get_value_from_item(data, items[i]);

        t_json_object json_obj;
        json_obj.item_name = items[i];
        json_obj.obj = obj;
        
        results[i] = json_obj;
   }

   *ret = results;
}

int main(int, char **)
{

    FILE *json_file;
    int result = fopen_s(&json_file, "../test.json", "r");
    if (result != 0)
    {
        printf_s("Failed to open file");
        return 0;
    }

    t_json_object* json = NULL; 
    parse(json_file, &json);

    for (int i = 0; i < sizeof(json) / 2; i++) {
         printf_s("[+] Found object: name = %s, type = %s, value = %s\n", json[i].item_name, json[i].obj.val_type, json[i].obj.value);
    }

    fclose(json_file);
    return 0;
}
