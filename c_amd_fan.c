/******************************************************************************/
/*                                                                            */
/******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libconfig.h>


void make_full_config_name(char *config_dir_name, char *config_file_name, char *full_config_name);
void string_value_config_read(char *config_file_name, char *parameter_name, char *);
int int_value_config_read(char *config_file_name, char *parameter_name);

void main()
{
    char *config_file_name = "amd_fan.cfg";
    char *config_dir_name = "/usr/local/etc/amd_fan";

    char debug_level[] = "";
    char debug_level_parameter_name[] = "DEBUG_LEVEL";

    int high_temp;
    char high_temp_parameter_name[] = "HIGH_TEMP";

    int low_temp;
    char low_temp_parameter_name[] = "LOW_TEMP";


    // Full name of the configuration file
    char full_config_file_name[50] = "";
    make_full_config_name(config_dir_name, config_file_name, full_config_file_name);

    // Read parameters from config file

    string_value_config_read(full_config_file_name, debug_level_parameter_name, debug_level);
    printf("Уровень отладки: %s\n\n", debug_level);

    high_temp = int_value_config_read(full_config_file_name, high_temp_parameter_name);
    printf("Верхний уровень температуры: %d\n\n", high_temp);

    low_temp = int_value_config_read(full_config_file_name, low_temp_parameter_name);
    printf("Нижний уровень температуры: %d\n", low_temp);



}

void string_value_config_read(char *config_file_name, char *parameter_name, char *value)
{
    config_t cfg;
    const char *parameter_value;

    // Инициализация
    config_init(&cfg);

    // Читать файл. Если ошибка, то завершить работу
    if(! config_read_file(&cfg, config_file_name))
    {
        fprintf(stderr, "%s:%d - %s\n", config_error_file(&cfg), config_error_line(&cfg), config_error_text(&cfg));

        // Освободить память обязательно, если это не конец программы
        config_destroy(&cfg);
    }

    // Поиск значения
    if(config_lookup_string(&cfg, parameter_name, &parameter_value))
        printf("%s: %s\n", parameter_name, parameter_value);
    else
        fprintf(stderr, "No '%s' setting in configuration file.\n", parameter_name);

    // Освободить память обязательно, если это не конец программы
    config_destroy(&cfg);

    //strcat(value, parameter_value);
    *value = *parameter_value;

}


int int_value_config_read(char *config_file_name, char *parameter_name)
{
    config_t cfg;
    int parameter_value;
//    config_setting_t *setting;
//    const char *str;
//    int config_parameter;
//    int low_temp;


    // Инициализация
    config_init(&cfg);

    // Читать файл. Если ошибка, то завершить работу
    if(! config_read_file(&cfg, config_file_name))
    {
        fprintf(stderr, "%s:%d - %s\n", config_error_file(&cfg), config_error_line(&cfg), config_error_text(&cfg));

        // Освободить память обязательно, если это не конец программы
        config_destroy(&cfg);
        return(EXIT_FAILURE);
    }

    // Поиск значения

//    if(config_lookup_string(&cfg, "DEBUG_LEVEL", &str))
//        printf("DEBUG_LEVEL: %s\n", str);
//    else
//        fprintf(stderr, "No 'DEBUG_LEVEL' setting in configuration file.\n");

    if(config_lookup_int(&cfg, parameter_name, &parameter_value))
        printf("%s: %d\n", parameter_name, parameter_value);
    else
        fprintf(stderr, "No '%s' setting in configuration file.\n", parameter_name);

    // Освободить память обязательно, если это не конец программы
    config_destroy(&cfg);
    return parameter_value;

}

void make_full_config_name(char *config_dir_name, char *config_file_name, char *full_config_name)
{
    // Полное имя файла конфигурации
    strcat(full_config_name, config_dir_name);
    strcat(full_config_name, "/");
    strcat(full_config_name, config_file_name);

    printf("Read configuration from file '%s'\n", full_config_name);

    // return full_config_name;
}
