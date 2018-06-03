/******************************************************************************/
/*                                                                            */
/******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <libconfig.h>

int fan_config_read();

void main()
{
    char *config_file_name = "amd_fan.cfg";
    char *config_dir_name = "/usr/local/etc/amd_fan";

    int *high_temp;

    // Read parameters from config file
    int status_code = fan_config_read(config_dir_name, config_file_name);
    printf("status_code = %d", status_code);

    
}


int fan_config_read(char *config_dir_name, char *config_file_name)
{
    char full_config_file_name[100];
    config_t cfg; 
    config_setting_t *setting;
    const char *str;
    int high_temp;
    int low_temp;
    
    // Полное имя файла конфигурации
    strcat(full_config_file_name, config_dir_name);
    strcat(full_config_file_name, "/");
    strcat(full_config_file_name, config_file_name);
    printf("Read configuration from file '%s'\n", full_config_file_name);
    
    // Инициализация
    config_init(&cfg);
    
    // Читать файл. Если ошибка, то завершить работу
    if(! config_read_file(&cfg, full_config_file_name))
    {
      fprintf(stderr, "%s:%d - %s\n", config_error_file(&cfg), config_error_line(&cfg), config_error_text(&cfg));
      
      // Освободить память обязательно, если это не конец программы
      config_destroy(&cfg);
      return(EXIT_FAILURE);
    }
    
    // Поиск значений
    if(config_lookup_string(&cfg, "DEBUG_LEVEL", &str))
      printf("DEBUG_LEVEL: %s\n", str);
    else
      fprintf(stderr, "No 'DEBUG_LEVEL' setting in configuration file.\n");
      
    if(config_lookup_int(&cfg, "HIGH_TEMP", &high_temp))
      printf("HIGH_TEMP: %d\n", high_temp);
    else
      fprintf(stderr, "No 'HIGH_TEMP' setting in configuration file.\n");
    
    if(config_lookup_int(&cfg, "LOW_TEMP", &low_temp))
      printf("LOW_TEMP: %d\n", low_temp);
    else
      fprintf(stderr, "No 'LOW_TEMP' setting in configuration file.\n");

    // Освободить память обязательно, если это не конец программы
    config_destroy(&cfg);
    return(EXIT_SUCCESS);
    
}    

