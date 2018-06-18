/******************************************************************************
 *   Автоматический регулятор скорости вращения вентиляторов видеокарт AMD    *
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libconfig.h>
#include <syslog.h>
#include <zconf.h>
#include <stdbool.h>
#include <signal.h>
#include <fcntl.h>


void make_full_config_name(char *config_dir_name, char *config_file_name, char *full_config_name);
void string_value_config_read(char *config_file_name, char *parameter_name, const char *local_value);
int int_value_config_read(char *config_file_name, char *parameter_name);
int get_gpu_number(void);
void set_initial_fan_speed(int gpu_number, int init_fan_speed);
void set_fan_speed(int gpu_number, int new_fan_speed);
int get_fan_speed(int gpu_number);
int get_temp(int gpu_number);
void set_new_fan_speed_for_all(int gpu_number, int init_fan_speed, int low_temp,
                               int high_temp, int speed_step, int min_fan_speed);
bool valid_range(int minimum, int maximum, int variable);
//static void hdl(int sig, siginfo_t *siginfo, void *context);
static void hdl(int sig);

struct sigaction act;



void main()
{

    openlog("c_amd_fan", LOG_PERROR, LOG_DEBUG);
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


    /***************************************
     * Full name of the configuration file *
     ***************************************/
    char full_config_file_name[50] = "";
    make_full_config_name(config_dir_name, config_file_name, full_config_file_name);


    /***************************************
     *  Read parameters from config file   *
     ***************************************/
    string_value_config_read(full_config_file_name, "DEBUG_LEVEL", debug_level);
    high_temp = int_value_config_read(full_config_file_name, "HIGH_TEMP");
    low_temp = int_value_config_read(full_config_file_name, "LOW_TEMP");
    sleep_time = int_value_config_read(full_config_file_name, "SLEEP_TIME");
    speed_step = int_value_config_read(full_config_file_name, "SPEED_STEP");
    init_fan_speed = int_value_config_read(full_config_file_name, "INIT_FAN_SPEED");
    min_fan_speed = int_value_config_read(full_config_file_name, "MIN_FAN_SPEED");


    /************************************
     *          Get GPU number          *
     ************************************/
    gpu_number = get_gpu_number();
    if(gpu_number == 0) {
        syslog(LOG_ERR,"GPU not found");
        exit(1);
    }


    /************************************
     *     Set initial fan speed        *
     ************************************/
    set_initial_fan_speed(gpu_number, init_fan_speed);


    /************************************
     *        Main program cycle        *
     ************************************/
    while(1) {
        syslog(LOG_DEBUG, "-=========================================================-");
        syslog(LOG_DEBUG, "A new cycle of change of fan speed. Cycle time: %d seconds.", sleep_time);
        set_new_fan_speed_for_all(gpu_number, init_fan_speed, low_temp, high_temp, speed_step, min_fan_speed);
        syslog(LOG_DEBUG, "-=========================================================-");
        sleep((unsigned int) sleep_time);

        act.sa_sigaction = (void (*)(int, siginfo_t *, void *)) &hdl;
        if (sigaction(SIGINT, &act, NULL)<0) {
            perror("sigaction");
            exit(1);
        }
    }
}


//static void hdl(int sig, siginfo_t *siginfo, void *context)
static void hdl(int sig)
{
    if (sig==SIGINT)
    {
        syslog(LOG_DEBUG, "\nServer was closed by user with Ctrl+C\n\n");
        exit(0);
    }
}

/*****************************************************************************
 * Sets the fan speed for all graphics cards                                 *
 *     Param: gpu_count - Number of video cards in the system                *
/*****************************************************************************/
void set_new_fan_speed_for_all(int gpu_number, int init_fan_speed, int low_temp,
                               int high_temp, int speed_step, int min_fan_speed) {

    int new_fan_speed = init_fan_speed;
    int current_temp;
    int current_fan_speed;


    for(int gpu = 0; gpu <= gpu_number; gpu++) {

        current_temp = get_temp(gpu);
        current_fan_speed = get_fan_speed(gpu);
        syslog(LOG_DEBUG, "GPU %d: Temp: %d°C, Fan speed: %d%%", gpu, current_temp, current_fan_speed);

        if(!valid_range(low_temp, high_temp, current_temp)) {

            syslog(LOG_DEBUG, "GPU %d: Out of temperature range %d...%d °C", gpu, low_temp, high_temp);

            // Increase the speed
            if(current_temp > high_temp) {
                if(current_fan_speed < 100) {
                    new_fan_speed = current_fan_speed + speed_step;

                }
            }

            // Decrease the speed
            if(current_temp < low_temp) {
                if(current_fan_speed > min_fan_speed) {
                    new_fan_speed = current_fan_speed - speed_step;
                }
            }


            if(((current_temp > high_temp) && (current_fan_speed < 100)) ||
               ((current_temp < low_temp) && (current_fan_speed > min_fan_speed))){
                set_fan_speed(gpu, new_fan_speed);
                syslog(LOG_DEBUG, "GPU %d: Set new fan speed: %d %%", gpu, new_fan_speed);
            }
        }

    }
}

/*****************************************************************************
 * Return the temperature of a given GPU                                     *
 *     Param: gpu_number - Number of video cards in the system               *
 *     Return: GPU temperature                                               *
 *****************************************************************************/
int get_temp(int gpu_number) {      // TODO: Вынести выполнение команды в отдельную функцию!!!
    FILE * file;
    size_t last_char;
    char command_result[75];
    char command[] = "";
    int current_temp;

    sprintf(command, "ethos-smi -g %d | grep \"* Temperature\" | cut -f 4 -d \" \" | rev | cut -c 2- | rev", gpu_number);

    file = popen(command, "r");
    last_char = fread(command_result, 1, 75, file);
    command_result[last_char] = '\0';

    current_temp = (int) strtol(command_result, (char **)NULL, 10);

    pclose(file);


    return current_temp;
}

/*****************************************************************************
 * Sets the initial fan speed when the program starts                        *
 *     Param: gpu_number - Number of video cards in the system               *
 *****************************************************************************/
void set_initial_fan_speed(int gpu_number, int init_fan_speed) {
    for(int gpu = 0; gpu <= gpu_number; gpu++) {
        set_fan_speed(gpu, init_fan_speed);
    }
}


/*****************************************************************************
 * Sets a new fan speed for a given GPU                                      *
 *     Param: gpu_number - Number of video cards in the system               *
 *     Param: new_fan_speed: New speed in percentage                         *
 *****************************************************************************/
void set_fan_speed(int gpu_number, int new_fan_speed) {

    FILE * file;
    size_t last_char;
    char command_result[75];
    char command[] = "";
    sprintf(command, "sudo ethos-smi --gpu %d --fan %d", gpu_number, new_fan_speed);

    file = popen(command, "r");
    last_char = fread(command_result, 1, 75, file);
    command_result[last_char] = '\0';

    pclose(file);
}


/*****************************************************************************
 * Return fan speed of a given GPU                                           *
 *     Param: gpu_number - Number of video cards in the system               *
 *     Return: Fan speed                                                     *
 *****************************************************************************/
int get_fan_speed(int gpu_number) {

    int fan_speed;

    FILE * file;
    size_t last_char;
    char command_result[75];
    char command[] = "";
    sprintf(command, "ethos-smi -g %d | grep \"* Fan Speed\" | cut -f 5 -d \" \" | rev | cut -c 2- | rev", gpu_number);

    file = popen(command, "r");
    last_char = fread(command_result, 1, 75, file);
    command_result[last_char] = '\0';

    fan_speed = (int) strtol(command_result, (char **)NULL, 10);

    pclose(file);

    return fan_speed;
}

/*****************************************************************************
 * Returns the number of GPU                                                 *
 *****************************************************************************/
int get_gpu_number(void) {

    int gpu_number = 0;
    FILE * file;
    size_t last_char;
    char command_result[75];

    char command[] = "ethos-smi | grep \"\\[\" | grep \"\\]\" | grep GPU | tail -1 | cut -f 1 -d \" \" | cut -c 4,5";

    file = popen(command, "r");
    last_char = fread(command_result, 1, 75, file);
    command_result[last_char] = '\0';

    gpu_number = (int) strtol(command_result, (char **)NULL, 10);

    syslog(LOG_DEBUG,"GPU count: %d", gpu_number);

    pclose(file);

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


/*****************************************************************************
 * Check range of valid values                                               *
 *    Param: minimum - Minimum value                                         *
 *    Param: maximum - Maximum value                                         *
 *    Param: variable - Check value                                          *
 *    Return: True - value is valid                                          *
 *****************************************************************************/
bool valid_range(int minimum, int maximum, int variable) {

    bool result;

    if((variable >= minimum) && (variable <= maximum))
        result = true;
    else
        result = false;

    return result;
}
