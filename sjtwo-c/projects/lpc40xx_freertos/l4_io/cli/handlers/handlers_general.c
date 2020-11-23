#include "cli_handlers.h"

#include "FreeRTOS.h"
#include "task.h"

#include "uart_printf.h"

#include <stdio.h>
static void cli__task_list_print(sl_string_t user_input_minus_command_name, app_cli__print_string_function cli_output);

app_cli_status_e cli__uart3_transmit(app_cli__argument_t argument, sl_string_t user_input_minus_command_name,
                                     app_cli__print_string_function cli_output) {
  uart_puts(UART__3, user_input_minus_command_name);

  cli_output(NULL, "Output to UART3: ");
  cli_output(NULL, user_input_minus_command_name);
  cli_output(NULL, "\r\n");

  return APP_CLI_STATUS__SUCCESS;
}

app_cli_status_e cli__crash_me(app_cli__argument_t argument, sl_string_t user_input_minus_command_name,
                               app_cli__print_string_function cli_output) {
  uint32_t *bad_pointer = (uint32_t *)0x00000001;
  *bad_pointer = 0xDEADBEEF;
  return APP_CLI_STATUS__SUCCESS;
}

app_cli_status_e cli__task_list(app_cli__argument_t argument, sl_string_t user_input_minus_command_name,
                                app_cli__print_string_function cli_output) {
  const int sleep_time = sl_string__to_int(user_input_minus_command_name);
  if (sleep_time > 0) {
    vTaskResetRunTimeStats();
    vTaskDelay(sleep_time);
  }

  // re-use user_input_minus_command_name as 'output_string' to save memory:
  sl_string_t output_string = user_input_minus_command_name;
  cli__task_list_print(output_string, cli_output);

  return APP_CLI_STATUS__SUCCESS;
}

static void cli__task_list_print(sl_string_t output_string, app_cli__print_string_function cli_output) {
  void *unused_cli_param = NULL;

#if (0 != configUSE_TRACE_FACILITY)
  // Enum to char : eRunning, eReady, eBlocked, eSuspended, eDeleted
  static const char *const task_status_table[] = {"running", " ready ", "blocked", "suspend", "deleted"};

  // Limit the tasks to avoid heap allocation.
  const unsigned portBASE_TYPE max_tasks = 10;
  TaskStatus_t status[max_tasks];
  uint32_t total_cpu_runtime = 0;
  uint32_t total_tasks_runtime = 0;

  const uint32_t total_run_time = portGET_RUN_TIME_COUNTER_VALUE();
  const unsigned portBASE_TYPE task_count = uxTaskGetSystemState(&status[0], max_tasks, &total_cpu_runtime);

  sl_string__printf(output_string, "%10s  Status Pr Stack CPU%%          Time\n", "Name");
  cli_output(unused_cli_param, output_string);

  for (unsigned priority_number = 0; priority_number < configMAX_PRIORITIES; priority_number++) {
    /* Print in sorted priority order */
    for (unsigned i = 0; i < task_count; i++) {
      const TaskStatus_t *task = &status[i];
      if (task->uxBasePriority == priority_number) {
        total_tasks_runtime += task->ulRunTimeCounter;

        const unsigned cpu_percent = (0 == total_cpu_runtime) ? 0 : task->ulRunTimeCounter / (total_cpu_runtime / 100);
        const unsigned time_us = task->ulRunTimeCounter;
        const unsigned stack_in_bytes = (sizeof(void *) * task->usStackHighWaterMark);

        sl_string__printf(output_string, "%10s %s %2u %5u %4u %10u us\n", task->pcTaskName,
                          task_status_table[task->eCurrentState], (unsigned)task->uxBasePriority, stack_in_bytes,
                          cpu_percent, time_us);
        cli_output(unused_cli_param, output_string);
      }
    }
  }

  sl_string__printf(output_string, "Overhead: %u uS\n", (unsigned)(total_run_time - total_tasks_runtime));
  cli_output(unused_cli_param, output_string);
#else
  cli_output(unused_cli_param, "Unable to provide you the task information along with their CPU and stack usage.\n");
  cli_output(unused_cli_param, "configUSE_TRACE_FACILITY macro at FreeRTOSConfig.h must be non-zero\n");
#endif
}

app_cli_status_e cli__suspend_resume(app_cli__argument_t argument, sl_string_t user_input_minus_command_name,
                                     app_cli__print_string_function cli_output) {
  // sl_string is a powerful string library, and you can utilize the sl_string.h API to parse parameters of a command

  // Sample code to output data back to the CLI
  sl_string_t s = user_input_minus_command_name; // Re-use a string to save memory
  sl_string_t tname = user_input_minus_command_name;
  sl_string_t suspend_or_resume = user_input_minus_command_name;
  printf(user_input_minus_command_name);
  char sch = 's';
  printf("\n");
  if (sl_string__begins_with(user_input_minus_command_name, "s")) {
    sl_string__erase_first_word(tname, ' ');
    printf(tname);
    printf("\n");
    TaskHandle_t task_suspend = xTaskGetHandle(tname);
    vTaskSuspend(task_suspend);
  } else if (sl_string__begins_with(user_input_minus_command_name, "r")) {
    sl_string__erase_first_word(tname, ' ');
    printf(tname);
    printf("\n");
    TaskHandle_t task_resume = xTaskGetHandle(tname);
    vTaskResume(task_resume);
  }

  // printf("     ");
  // printf(suspend_or_resume);
  printf("\n");
  sl_string__printf(s, "CLI Command for suspend or resume has been executed\n");
  cli_output(NULL, s);

  return APP_CLI_STATUS__SUCCESS;
}

QueueHandle_t song_name_queue;
typedef char songname_t[32];

app_cli_status_e cli__play(app_cli__argument_t argument, sl_string_t user_input_minus_command_name,
                           app_cli__print_string_function cli_output) {
  // sl_string is a powerful string library, and you can utilize the sl_string.h API to parse parameters of a command

  // Sample code to output data back to the CLI
  sl_string_t s = user_input_minus_command_name; // Re-use a string to save memory
  sl_string_t tname = user_input_minus_command_name;
  sl_string_t suspend_or_resume = user_input_minus_command_name;
  printf(user_input_minus_command_name);
  char sch = 's';
  printf("\n");
  songname_t s_name = {0};
  sl_string__copy_to(s, s_name, sizeof(s_name) - 1);
  // strncpy(s_name, s, sizeof(s_name) - 1)
  // strncpy(s_name, *p, sizeof(s_name)); // How do I pass in the string value from the CLI?
  // That value will be the file name, then pass the song
  if (xQueueSend(song_name_queue, &s_name, 0)) {
    puts("Songname on queue");
  } else {
    puts("Songname failed to queue");
  }
  // if (sl_string__begins_with(user_input_minus_command_name, "s")) {
  // sl_string__erase_first_word(tname, ' ');
  //   printf(tname);
  //   printf("\n");
  //   TaskHandle_t task_suspend = xTaskGetHandle(tname);
  //   vTaskSuspend(task_suspend);
  // } else if (sl_string__begins_with(user_input_minus_command_name, "r")) {
  //   sl_string__erase_first_word(tname, ' ');
  //   printf(tname);
  //   printf("\n");
  //   TaskHandle_t task_resume = xTaskGetHandle(tname);
  //   vTaskResume(task_resume);
  // }

  // printf("     ");
  // printf(suspend_or_resume);
  printf("\n");
  sl_string__printf(s, "CLI Command for play has been executed\n");
  cli_output(NULL, s);

  return APP_CLI_STATUS__SUCCESS;
}

app_cli_status_e cli__pause_resume(app_cli__argument_t argument, sl_string_t user_input_minus_command_name,
                                   app_cli__print_string_function cli_output) {
  // sl_string is a powerful string library, and you can utilize the sl_string.h API to parse parameters of a command

  // Sample code to output data back to the CLI
  sl_string_t s = user_input_minus_command_name; // Re-use a string to save memory
  // sl_string_t tname = user_input_minus_command_name;
  // sl_string_t suspend_or_resume = user_input_minus_command_name;
  printf(user_input_minus_command_name);
  char sch = 's';
  // printf("\n");
  if (sl_string__begins_with(user_input_minus_command_name, "p")) {
    // sl_string__erase_first_word(tname, ' ');
    // printf(tname);
    // printf("\n");
    TaskHandle_t task_suspend = xTaskGetHandle("player"); // check whether it should be player or reader to suspend
    vTaskSuspend(task_suspend);
  } else if (sl_string__begins_with(user_input_minus_command_name, "r")) {
    // sl_string__erase_first_word(tname, ' ');
    // printf(tname);
    // printf("\n");
    TaskHandle_t task_resume = xTaskGetHandle("player");
    vTaskResume(task_resume);
  }

  // printf("     ");
  // printf(suspend_or_resume);
  // printf("\n");
  // sl_string__printf(s, "CLI Command for suspend or resume music has been executed\n");
  cli_output(NULL, s);

  return APP_CLI_STATUS__SUCCESS;
}