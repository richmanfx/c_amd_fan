/******************************************************************************
*    Автоматический регулятор скорости вращения вентиляторов видеокарт AMD    *
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libconfig.h>
#include <syslog.h>


void make_full_config_name(char *config_dir_name, char *config_file_name, char *full_config_name);
void string_value_config_read(char *config_file_name, char *parameter_name, const char *local_value);
int int_value_config_read(char *config_file_name, char *parameter_name);
int get_gpu_number();

void main()
{

    openlog("c_amd_fan", LOG_PERROR, LOG_USER);
    syslog(LOG_DEBUG, "Start 'c_amd_fan'");

    char *config_file_name = "amd_fan.cfg";
    char *config_dir_name = "/usr/local/etc/amd_fan";

    char debug_level[] = "DEBUG";
    int high_temp;
    int low_temp;
    int sleep_time;
    int speed_step;
    int init_fan_speed;
    int min_fan_speed;
    int gpu_number;


    /***************************************/
    /* Full name of the configuration file */
    /***************************************/
    char full_config_file_name[50] = "";
    make_full_config_name(config_dir_name, config_file_name, full_config_file_name);


    /***************************************/
    /*  Read parameters from config file   */
    /***************************************/
    string_value_config_read(full_config_file_name, "DEBUG_LEVEL", debug_level);
    high_temp = int_value_config_read(full_config_file_name, "HIGH_TEMP");
    low_temp = int_value_config_read(full_config_file_name, "LOW_TEMP");
    sleep_time = int_value_config_read(full_config_file_name, "SLEEP_TIME");
    speed_step = int_value_config_read(full_config_file_name, "SPEED_STEP");
    init_fan_speed = int_value_config_read(full_config_file_name, "INIT_FAN_SPEED");
    min_fan_speed = int_value_config_read(full_config_file_name, "MIN_FAN_SPEED");


    /************************************/
    /*          Get GPU number          */
    /************************************/
    gpu_number = get_gpu_number();

    closelog();
}

int get_gpu_number() {

    int gpu_number = 0;
    FILE * file;
    size_t last_char;
    char command_result[80];

    char command[] = "ethos-smi | grep \"\\[\" | grep \"\\]\" | grep GPU | tail -1 | cut -f 1 -d \" \" | cut -c 4,5";

    file = popen(command, "r");
    last_char = fread(command_result, 1, 80, file);
    command_result[last_char] = '\0';

    if(command_result[0] == '\000') {
        syslog(LOG_ERR,"GPU not found");
    } else {
        if(command_result[1] == '\012' && command_result[2] == '\000')
            gpu_number = ((int)command_result[0] - 48) +1;
        else {
            if (command_result[2] == '\012' && command_result[3] == '\000') {  // Здесь ещё не известно - нет более 10-ти GPU для тестов
                gpu_number = (((int) command_result[1] - 48) * 10) + ((int) command_result[0] - 48) + 1;
            }
        }
    }

    syslog(LOG_DEBUG,"GPU count: %d", gpu_number);

    return gpu_number;
}

void string_value_config_read(char *config_file_name, char *parameter_name, const char *local_value) {
    config_t cfg;

    // Инициализация
    config_init(&cfg);

    // Читать файл. Если ошибка, то завершить работу
    if(! config_read_file(&cfg, config_file_name)) {
        syslog(LOG_ERR, "Read config file error. %s:%d - %s\n", config_error_file(&cfg), config_error_line(&cfg), config_error_text(&cfg));
        config_destroy(&cfg);   // Освободить память обязательно, если это не конец программы
    }

    // Поиск значения
    if(config_lookup_string(&cfg, parameter_name, &local_value))
        syslog(LOG_DEBUG, "%s: %s\n", parameter_name, local_value);
    else
        syslog(LOG_ERR, "Parameter '%s' not found in config file", parameter_name);

    // Освободить память обязательно, если это не конец программы
    config_destroy(&cfg);

}


int int_value_config_read(char *config_file_name, char *parameter_name) {
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
        syslog(LOG_ERR, "Read config file error. %s:%d - %s\n", config_error_file(&cfg), config_error_line(&cfg), config_error_text(&cfg));

        // Освободить память обязательно, если это не конец программы
        config_destroy(&cfg);
        return(EXIT_FAILURE);
    }

    // Поиск значения
    if(config_lookup_int(&cfg, parameter_name, &parameter_value))
        syslog(LOG_DEBUG, "%s: %d\n", parameter_name, parameter_value);
    else
        syslog(LOG_ERR, "Parameter '%s' not found in config file", parameter_name);

    // Освободить память обязательно, если это не конец программы
    config_destroy(&cfg);
    return parameter_value;

}

void make_full_config_name(char *config_dir_name, char *config_file_name, char *full_config_name) {
    // Полное имя файла конфигурации
    strcat(full_config_name, config_dir_name);
    strcat(full_config_name, "/");
    strcat(full_config_name, config_file_name);

    syslog(LOG_DEBUG, "Name of configuration file: '%s'", full_config_name);
}
