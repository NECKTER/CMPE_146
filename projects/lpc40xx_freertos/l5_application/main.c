#include <stdio.h>

#include "FreeRTOS.h"
#include "lpc40xx.h"
#include "task.h"

#include "board_io.h"
#include "common_macros.h"
#include "gpio.h"
#include "gpio_lab.h"
#include "periodic_scheduler.h"
#include "semphr.h"
#include "sj2_cli.h"

#include "gpio_isr.h"
#include "lpc_peripherals.h"

static void create_blinky_tasks(void);
static void create_uart_task(void);
static void blink_task(void *params);
static void uart_task(void *params);
void led_task(void *task_parameter);
void switch_task(void *task_parameter);
void gpio_interrupt(void);
void configure_your_gpio_interrupt();
void sleep_on_sem_task(void *p);
void pin29_isr(void);
void pin30_isr(void);

static SemaphoreHandle_t switch_press_indication;
static SemaphoreHandle_t switch_pressed_signal;

#define outOfTheBox 0
#define Lab2 0
#define Lab3 1

int main(void) { // main function for project
  puts("Starting RTOS");

#if outOfTheBox
  create_blinky_tasks();
  create_uart_task();
#else
  gpio0__attach_interrupt(29, GPIO_INTR__RISING_EDGE, pin29_isr);
  gpio0__attach_interrupt(30, GPIO_INTR__FALLING_EDGE, pin30_isr);

  NVIC_EnableIRQ(GPIO_IRQn); // Enable interrupt gate for the GPIO
#endif

  vTaskStartScheduler(); // This function never returns unless RTOS scheduler runs out of memory and fails

  return 0;
}
#if Lab2
void led_task(void *task_parameter) {
  gpio_pin *led = (gpio_pin *)task_parameter;
  while (true) {
    // Note: There is no vTaskDelay() here, but we use sleep mechanism while waiting for the binary semaphore (signal)
    if (xSemaphoreTake(switch_press_indication, 1000)) {
      // TODO: Blink the LED
      {
        gpio0__set_high(*led);
        vTaskDelay(100);

        gpio0__set_low(*led);
        vTaskDelay(100);
      }
      xSemaphoreGive(switch_press_indication);
    } else {
      puts("Timeout: No switch press indication for 1000ms");
    }
    vTaskDelay(10);
  }
}

void switch_task(void *task_parameter) {
  gpio_pin *sw = (gpio_pin *)task_parameter;
  gpio0__set_as_input(*sw);

  while (true) {
    // TODO: If switch pressed, set the binary semaphore
    if (gpio0__get_level(*sw)) {
      xSemaphoreGive(switch_press_indication);
    } else
      xSemaphoreTake(switch_press_indication, 100);

    // Task should always sleep otherwise they will use 100% CPU
    // This task sleep also helps avoid spurious semaphore give during switch debeounce
    vTaskDelay(10);
  }
}
#endif

#if Lab3
// Objective of the assignment is to create a clean API to register sub-interrupts like so:
void pin29_isr(void) {
  fprintf(stderr, "Interrupt pin30\n");
  gpio_s Led = gpio__construct_as_output(1, 18);
  gpio__toggle(Led);
}
void pin30_isr(void) {
  fprintf(stderr, "Interrupt pin31\n");
  gpio_s Led = gpio__construct_as_output(1, 24);
  gpio__toggle(Led);
}
#endif

static void create_blinky_tasks(void) {
  /**
   * Use '#if (1)' if you wish to observe how two tasks can blink LEDs
   * Use '#if (0)' if you wish to use the 'periodic_scheduler.h' that will spawn 4 periodic tasks, one for each LED
   */
#if (1)
  // These variables should not go out of scope because the 'blink_task' will reference this memory
  static gpio_s led0, led1;

  led0 = board_io__get_led0();
  led1 = board_io__get_led1();

  xTaskCreate(blink_task, "led0", configMINIMAL_STACK_SIZE, (void *)&led0, PRIORITY_LOW, NULL);
  xTaskCreate(blink_task, "led1", configMINIMAL_STACK_SIZE, (void *)&led1, PRIORITY_LOW, NULL);
#else
  const bool run_1000hz = true;
  const size_t stack_size_bytes = 2048 / sizeof(void *); // RTOS stack size is in terms of 32-bits for ARM M4 32-bit CPU
  periodic_scheduler__initialize(stack_size_bytes, !run_1000hz); // Assuming we do not need the high rate 1000Hz task
  UNUSED(blink_task);
#endif
}

static void create_uart_task(void) {
  // It is advised to either run the uart_task, or the SJ2 command-line (CLI), but not both
  // Change '#if (0)' to '#if (1)' and vice versa to try it out
#if (0)
  // printf() takes more stack space, size this tasks' stack higher
  xTaskCreate(uart_task, "uart", (512U * 8) / sizeof(void *), NULL, PRIORITY_LOW, NULL);
#else
  sj2_cli__init();
  UNUSED(uart_task); // uart_task is un-used in if we are doing cli init()
#endif
}

static void blink_task(void *params) {
  const gpio_s led = *((gpio_s *)params); // Parameter was input while calling xTaskCreate()

  // Warning: This task starts with very minimal stack, so do not use printf() API here to avoid stack overflow
  while (true) {
    gpio__toggle(led);
    vTaskDelay(500);
  }
}

// This sends periodic messages over printf() which uses system_calls.c to send them to UART0
static void uart_task(void *params) {
  TickType_t previous_tick = 0;
  TickType_t ticks = 0;

  while (true) {
    // This loop will repeat at precise task delay, even if the logic below takes variable amount of ticks
    vTaskDelayUntil(&previous_tick, 2000);

    /* Calls to fprintf(stderr, ...) uses polled UART driver, so this entire output will be fully
     * sent out before this function returns. See system_calls.c for actual implementation.
     *
     * Use this style print for:
     *  - Interrupts because you cannot use printf() inside an ISR
     *    This is because regular printf() leads down to xQueueSend() that might block
     *    but you cannot block inside an ISR hence the system might crash
     *  - During debugging in case system crashes before all output of printf() is sent
     */
    ticks = xTaskGetTickCount();
    fprintf(stderr, "%u: This is a polled version of printf used for debugging ... finished in", (unsigned)ticks);
    fprintf(stderr, " %lu ticks\n", (xTaskGetTickCount() - ticks));

    /* This deposits data to an outgoing queue and doesn't block the CPU
     * Data will be sent later, but this function would return earlier
     */
    ticks = xTaskGetTickCount();
    printf("This is a more efficient printf ... finished in");
    printf(" %lu ticks\n\n", (xTaskGetTickCount() - ticks));
  }
}
