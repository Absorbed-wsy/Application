// src/core/config.c
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

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

Config *config_create() {
    Config *config = malloc(sizeof(Config));
    if (config) {
        config->sections = NULL;
        config->section_count = 0;
    }
    return config;
}

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

static char *trim(char *str) {
    while (isspace((unsigned char)*str)) str++;
    if (*str == 0) return str;

    char *end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return str;
}

int config_load(Config *config, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) return 0;

    char line[256];
    Section *current_section = NULL;

    while (fgets(line, sizeof(line), file)) {
        char *ptr = trim(line);
        if (*ptr == ';' || *ptr == '#' || *ptr == '\0') continue;

        if (*ptr == '[') {
            char *end = strchr(ptr, ']');
            if (!end) continue;
            *end = '\0';
            char *section_name = trim(ptr + 1);

            // 查找或创建section
            current_section = NULL;
            for (int i = 0; i < config->section_count; i++) {
                if (strcmp(config->sections[i].name, section_name) == 0) {
                    current_section = &config->sections[i];
                    break;
                }
            }

            if (!current_section) {
                config->section_count++;
                config->sections = realloc(config->sections, config->section_count * sizeof(Section));
                current_section = &config->sections[config->section_count - 1];
                current_section->name = strdup(section_name);
                current_section->pairs = NULL;
                current_section->pair_count = 0;
            }
        } else if (current_section) {
            char *sep = strchr(ptr, '=');
            if (!sep) continue;
            *sep = '\0';
            char *key = trim(ptr);
            char *value = trim(sep + 1);

            current_section->pair_count++;
            current_section->pairs = realloc(current_section->pairs, current_section->pair_count * sizeof(KeyValue));
            KeyValue *pair = &current_section->pairs[current_section->pair_count - 1];
            pair->key = strdup(key);
            pair->value = strdup(value);
        }
    }

    fclose(file);
    return 1;
}

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

int config_get_int(Config *config, const char *section, const char *key, int default_value) {
    const char *str = config_get_string(config, section, key, NULL);
    if (str) {
        return atoi(str);
    }
    return default_value;
}

double config_get_double(Config *config, const char *section, const char *key, double default_value) {
    const char *str = config_get_string(config, section, key, NULL);
    if (str) {
        return atof(str);
    }
    return default_value;
}

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
