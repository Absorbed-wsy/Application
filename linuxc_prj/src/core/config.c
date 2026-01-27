// src/core/config.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "config.h"
#include "logger.h"

typedef struct {
    char *key;
    char *value;
} KeyValue;

typedef struct {
    char *name;
    KeyValue *pairs;
    int pair_count;
} Section;

struct Config {
    Section *sections;
    int section_count;
};

/**
 * @brief Create configuration structure
 * @return Returns Config pointer on success, NULL on failure
 */
Config *config_create() {
    Config *config = malloc(sizeof(Config));
    if (config) {
        config->sections = NULL;
        config->section_count = 0;
    }
    return config;
}

/**
 * @brief Free configuration structure memory
 * @param config Configuration structure pointer
 */
void config_free(Config *config) {
    if (config) {
        for (int i = 0; i < config->section_count; i++) {
            Section *sec = &config->sections[i];
            for (int j = 0; j < sec->pair_count; j++) {
                free(sec->pairs[j].key);
                free(sec->pairs[j].value);
            }
            free(sec->pairs);
            free(sec->name);
        }
        free(config->sections);
        free(config);
    }
}

/**
 * @brief Remove leading and trailing whitespace from string
 * @param str Input string
 * @return Pointer to processed string
 */
static char *trim(char *str) {
    if (!str) return str;
    
    char *start = str;
    while (*start && isspace((unsigned char)*start)) 
        start++;
    
    if (*start == '\0') {
        *str = '\0';
        return str;
    }
    
    char *end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end))
        end--;
    
    *(end + 1) = '\0';
    
    if (start != str) {
        memmove(str, start, end - start + 2);
    }
    
    return str;
}

/**
 * @brief Load configuration from file
 * @param config Configuration structure pointer
 * @param filename Configuration file name
 * @return Returns 1 on success, 0 on failure
 */
int config_load(Config *config, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open config file");
        return 0;
    }

    char line[256];
    Section *current_section = NULL;
    int line_num = 0;

    while (fgets(line, sizeof(line), file)) {
        line_num++;
        
        line[sizeof(line) - 1] = '\0';
        
        char *newline = strchr(line, '\n');
        if (newline) *newline = '\0';
        
        char *ptr = trim(line);
        if (*ptr == ';' || *ptr == '#' || *ptr == '\0') continue;

        if (*ptr == '[') {
            char *end = strchr(ptr, ']');
            if (!end) {
                fprintf(stderr, "Line %d: Missing closing bracket\n", line_num);
                continue;
            }
            
            char section_buf[64];
            size_t len = end - ptr - 1;
            if (len >= sizeof(section_buf)) len = sizeof(section_buf) - 1;
            
            strncpy(section_buf, ptr + 1, len);
            section_buf[len] = '\0';
            
            char *section_name = trim(section_buf);
            
            current_section = NULL;
            for (int i = 0; i < config->section_count; i++) {
                if (strcmp(config->sections[i].name, section_name) == 0) {
                    current_section = &config->sections[i];
                    break;
                }
            }

            if (!current_section) {
                Section *new_sections = realloc(config->sections, 
                                              (config->section_count + 1) * sizeof(Section));
                if (!new_sections) {
                    perror("Memory allocation failed for sections");
                    fclose(file);
                    return 0;
                }
                
                config->sections = new_sections;
                current_section = &config->sections[config->section_count];
                config->section_count++;
                
                memset(current_section, 0, sizeof(Section));
                current_section->name = strdup(section_name);
                if (!current_section->name) {
                    perror("strdup failed for section name");
                    fclose(file);
                    return 0;
                }
            }
        } 
        else if (current_section) {
            char *sep = strchr(ptr, '=');
            if (!sep) {
                fprintf(stderr, "Line %d: Missing equals sign\n", line_num);
                continue;
            }
            
            char key_buf[64], value_buf[256];
            
            size_t key_len = sep - ptr;
            if (key_len >= sizeof(key_buf)) key_len = sizeof(key_buf) - 1;
            strncpy(key_buf, ptr, key_len);
            key_buf[key_len] = '\0';
            char *key = trim(key_buf);
            
            char *val_start = sep + 1;
            size_t val_len = strlen(val_start);
            if (val_len >= sizeof(value_buf)) val_len = sizeof(value_buf) - 1;
            strncpy(value_buf, val_start, val_len);
            value_buf[val_len] = '\0';
            char *value = trim(value_buf);
            
            int found = 0;
            for (int j = 0; j < current_section->pair_count; j++) {
                if (strcmp(current_section->pairs[j].key, key) == 0) {
                    free(current_section->pairs[j].value);
                    current_section->pairs[j].value = strdup(value);
                    if (!current_section->pairs[j].value) {
                        perror("strdup failed for value");
                        fclose(file);
                        return 0;
                    }
                    found = 1;
                    break;
                }
            }
            
            if (found) continue;
            
            KeyValue *new_pairs = realloc(current_section->pairs, 
                                        (current_section->pair_count + 1) * sizeof(KeyValue));
            if (!new_pairs) {
                perror("Memory allocation failed for key-value pairs");
                fclose(file);
                return 0;
            }
            
            current_section->pairs = new_pairs;
            KeyValue *pair = &current_section->pairs[current_section->pair_count];
            memset(pair, 0, sizeof(KeyValue));
            
            pair->key = strdup(key);
            if (!pair->key) {
                perror("strdup failed for key");
                fclose(file);
                return 0;
            }
            
            pair->value = strdup(value);
            if (!pair->value) {
                perror("strdup failed for value");
                free(pair->key);
                fclose(file);
                return 0;
            }
            
            current_section->pair_count++;
            
            //printf("Config Added: [%s] %s = %s", current_section->name, key, value);
        }
    }

    fclose(file);
    return 1;
}

/**
 * @brief Get string type configuration value
 * @param config Configuration structure pointer
 * @param section Configuration section name
 * @param key Configuration key name
 * @param default_value Default value
 * @return Configuration value string pointer
 */
const char *config_get_string(Config *config, const char *section, const char *key, const char *default_value) {
    for (int i = 0; i < config->section_count; i++) {
        Section *sec = &config->sections[i];
        if (strcmp(sec->name, section) == 0) {
            for (int j = 0; j < sec->pair_count; j++) {
                KeyValue *pair = &sec->pairs[j];
                if (strcmp(pair->key, key) == 0) {
                    return pair->value;
                }
            }
            break;
        }
    }
    return default_value;
}

/**
 * @brief Get integer type configuration value
 * @param config Configuration structure pointer
 * @param section Configuration section name
 * @param key Configuration key name
 * @param default_value Default value
 * @return Configuration integer value
 */
int config_get_int(Config *config, const char *section, const char *key, int default_value) {
    const char *str = config_get_string(config, section, key, NULL);
    if (str) {
        return atoi(str);
    }
    return default_value;
}

/**
 * @brief Get floating-point type configuration value
 * @param config Configuration structure pointer
 * @param section Configuration section name
 * @param key Configuration key name
 * @param default_value Default value
 * @return Configuration floating-point value
 */
double config_get_double(Config *config, const char *section, const char *key, double default_value) {
    const char *str = config_get_string(config, section, key, NULL);
    if (str) {
        return atof(str);
    }
    return default_value;
}

/**
 * @brief Get boolean type configuration value
 * @param config Configuration structure pointer
 * @param section Configuration section name
 * @param key Configuration key name
 * @param default_value Default value
 * @return Configuration boolean value (1:true, 0:false)
 */
int config_get_bool(Config *config, const char *section, const char *key, int default_value) {
    const char *str = config_get_string(config, section, key, NULL);
    if (str) {
        if (strcasecmp(str, "true") == 0 || strcasecmp(str, "yes") == 0 || strcmp(str, "1") == 0) {
            return 1;
        } else if (strcasecmp(str, "false") == 0 || strcasecmp(str, "no") == 0 || strcmp(str, "0") == 0) {
            return 0;
        }
    }
    return default_value;
}

/**
 * @brief Initialize application configuration
 * @param filename Configuration file name
 * @param app Application configuration structure pointer
 * @return Execution status code
 */
int config_initialize(const char* filename, struct main_config *app)
{
    Config* conf = config_create();
    config_load(conf, filename);

    // Log level
    app->debug = config_get_int(conf, "Logging", "level", 1);
    LOG_DEBUG("Main debug level = %d",app->debug);

    // Main loop enable
    app->loop = config_get_bool(conf, "Config", "main_loop", 1);
    LOG_DEBUG("Main loop %s",app->loop ? "Enable" : "Disable");

    // Number of application threads
    app->nthread = config_get_int(conf, "Thread", "nthread", 1);
    LOG_DEBUG("The number of application threads is %d",app->nthread);

    // Other configuration

    config_free(conf);

    return 0;
}
