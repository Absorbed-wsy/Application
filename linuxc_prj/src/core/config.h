// src/core/config.h
#ifndef CONFIG_H
#define CONFIG_H

typedef struct main_config {
    int loop;
    int debug;
    int nthread;
} Aconf;

typedef struct Config Config;

Config *config_create();
void config_free(Config *config);
int config_load(Config *config, const char *filename);
const char *config_get_string(Config *config, const char *section, const char *key, const char *default_value);
int config_get_int(Config *config, const char *section, const char *key, int default_value);
double config_get_double(Config *config, const char *section, const char *key, double default_value);
int config_get_bool(Config *config, const char *section, const char *key, int default_value);

int config_initialize(const char* filename, struct main_config *app);

#endif // CONFIG_H
