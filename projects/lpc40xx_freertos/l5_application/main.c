#include <stdio.h>
#include <stdlib.h>

#include "FreeRTOS.h"
#include "lpc40xx.h"
#include "task.h"

#include "adc.h"
#include "board_io.h"
#include "common_macros.h"
#include "gpio.h"
#include "gpio_lab.h"
#include "periodic_scheduler.h"
#include "pwm1.h"
#include "queue.h"
#include "semphr.h"
#include "sj2_cli.h"
#include "ssp_lab.h"
#include "uart_lab.h"

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
void pin29_isr(void); // Lab 3
void pin30_isr(void);
void pwm_task(void *p); // Lab 4
void adc_task(void *p);

static SemaphoreHandle_t switch_press_indication;
static SemaphoreHandle_t switch_pressed_signal;
static QueueHandle_t adc_to_pwm_task_queue;

#define outOfTheBox 0
#define Lab2 0
#define Lab3 0
#define Lab4 0
#define Lab5 0
#define Lab6 1

#if Lab5
static SemaphoreHandle_t spi_bus_mutex;
typedef struct {
  uint8_t manufacturer_id;
  uint8_t device_id_1;
  uint8_t device_id_2;
  uint8_t extended_device_id;
} adesto_flash_id_s;
gpio_s chipSelect;
gpio_s dummySelect;
void task_one();
void task_two();
void adesto_cs(void) { // LPC_GPIO1->CLR |= (1 << 10);
  gpio__reset(chipSelect);
  gpio__reset(dummySelect);
}
void adesto_ds(void) { // LPC_GPIO1->SET |= (1 << 10);
  gpio__set(chipSelect);
  gpio__set(dummySelect);
}
void spi_task(void *p);
#endif

#if Lab6
void uart_read_task(void *p);
void uart_write_task(void *p);
void board_1_sender_task(void *p);
void board_2_receiver_task(void *p);
#endif

int main(void) { // main function for project
  puts("Starting RTOS");

#if outOfTheBox
  create_blinky_tasks();
  create_uart_task();
#else

  // TODO: Use uart_lab__init() function and initialize UART2 or UART3 (your choice)
  // TODO: Pin Configure IO pins to perform UART2/UART3 function
  uart_lab__init(UART_2, 96, 115200);
  uart__enable_receive_interrupt(UART_2);
  uart_lab__init(UART_3, 96, 115200);
  uart__enable_receive_interrupt(UART_3);

  xTaskCreate(board_2_receiver_task, /*description*/ "uart_task", /*stack depth*/ 4096 / sizeof(void *),
              /*parameter*/ (void *)1,
              /*priority*/ 1, /*optional handle*/ NULL);
  xTaskCreate(board_1_sender_task, /*description*/ "uart_task2", /*stack depth*/ 4096 / sizeof(void *),
              /*parameter*/ (void *)1,
              /*priority*/ 2, /*optional handle*/ NULL);
#endif
  vTaskStartScheduler(); // This function never returns unless RTOS scheduler runs out of memory and fails
  return 0;
}

#if Lab6
void uart_read_task(void *p) {
  while (1) {
    // TODO: Use uart_lab__polled_get() function and printf the received value
    char testValue = 0;
    uart_lab__polled_get(UART_2, &testValue);
    fprintf(stderr, "%c", testValue);
    vTaskDelay(500);
  }
}

void uart_write_task(void *p) {
  char testString[] = "Hello world\n\n";
  int strlength = sizeof(testString) / sizeof(char);
  int i = 0;
  fprintf(stderr, "%s", testString);
  while (1) {
    // TODO: Use uart_lab__polled_put() function and send a value
    uart_lab__polled_put(UART_3, testString[i % strlength]);
    ++i;
    vTaskDelay(500);
  }
}
// This task is done for you, but you should understand what this code is doing
void board_1_sender_task(void *p) {
  char number_as_string[] = "Hello World";

  while (true) {
    //        const int number = rand();
    //        sprintf(number_as_string, "%i", number);

    // Send one char at a time to the other board including terminating NULL char
    for (int i = 0; i <= strlen(number_as_string); i++) {
      uart_lab__polled_put(UART_3, number_as_string[i]);
      printf("Sent: %c\n", number_as_string[i]);
    }

    //        printf("Sent: %i over UART to the other board\n", number);
    vTaskDelay(3000);
  }
}

void board_2_receiver_task(void *p) {
  char number_as_string[16] = {0};
  int counter = 0;

  while (true) {
    char byte = 0;
    uart_lab__get_char_from_queue(&byte, portMAX_DELAY);
    printf("Received: %c\n", byte);

    // This is the last char, so print the number
    if ('\0' == byte) {
      number_as_string[counter] = '\0';
      counter = 0;
      printf("Received this data from the other board: %s\n", number_as_string);
    }
    // We have not yet received the NULL '\0' char, so buffer the data
    else {
      // TODO: Store data to number_as_string[] array one char at a time
      number_as_string[counter] = byte;
      ++counter;
    }
  }
}
#endif

#if Lab5
/*
   const uint32_t spi_clock_mhz = 10;
  chipSelect = gpio__construct_as_output(GPIO__PORT_1, 10);
  dummySelect = gpio__construct_as_output(GPIO__PORT_1, 14);
  gpio__set(chipSelect);
  gpio__set(dummySelect);

  ssp2__init(spi_clock_mhz);
  spi_bus_mutex = xSemaphoreCreateMutex();
  xTaskCreate(spi_task, /*description*/ "spi_task", /*stack depth*/ 4096 / sizeof(void *), /*parameter*/ (void *)1,
/*priority*/ 1, /*optional handle*/ NULL);
xTaskCreate(spi_task, /*description*/ "spi_task2", /*stack depth*/ 4096 / sizeof(void *), /*parameter*/ (void *)1,
            /*priority*/ 1, /*optional handle*/ NULL);
* / adesto_flash_id_s adesto_read_signature(void) {
  adesto_flash_id_s data = {0};

  adesto_cs();
  {
    uint8_t opcode = ssp2__swap_byte(0x9F);
    data.manufacturer_id = ssp2__swap_byte(0xa);
    data.device_id_1 = ssp2__swap_byte(0xb);
    data.device_id_2 = ssp2__swap_byte(0xc);
    data.extended_device_id = ssp2__swap_byte(0xd);
  }
  adesto_ds();

  return data;
}

void spi_task(void *p) {

  while (1) {
    if (xSemaphoreTake(spi_bus_mutex, portMAX_DELAY)) {
      // Use Guarded Resource
      adesto_flash_id_s id = adesto_read_signature();
      fprintf(stderr,
              "manufacturer_id: %x\n"
              "device_id_1: %x\n"
              "device_id_2: %x\n"
              "extended_device_id: %x\n\n",
              id.manufacturer_id, id.device_id_1, id.device_id_2, id.extended_device_id);
      if (id.manufacturer_id != 0x1F) {
        fprintf(stderr, "Manufacturer ID read failure\n");
        vTaskSuspend(NULL); // Kill this task
      }
      // Give Semaphore back:
      xSemaphoreGive(spi_bus_mutex);
    }
  }
}
#endif

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

#if Lab4
/*
  adc_to_pwm_task_queue = xQueueCreate(1, sizeof(int));

  xTaskCreate(pwm_task, /*description*/ "pwm_task", /*stack depth*/ 4096 / sizeof(void *), /*parameter*/ (void *)1,
              /*priority*/ 2, /*optional handle*/ NULL);
xTaskCreate(adc_task, /*description*/ "adc_task", /*stack depth*/ 4096 / sizeof(void *), /*parameter*/ (void *)1,
            /*priority*/ 1, /*optional handle*/ NULL);
* / void pwm_task(void *p) {
  pwm1__init_single_edge(1000);

  // Locate a GPIO pin that a PWM channel will control
  // NOTE You can use gpio__construct_with_function() API from gpio.h

  gpio_s pin = gpio__construct_with_function(GPIO__PORT_2, /*Pin*/ 0, GPIO__FUNCTION_1);

  // We only need to set PWM configuration once, and the HW will drive
  // the GPIO at 1000Hz, and control set its duty cycle to 50%
  pwm1__set_duty_cycle(PWM1__2_0, 50);

  // Continue to vary the duty cycle in the loop
  uint8_t percent = 0;
  int adc_reading = 0;
  while (1) {

    if (xQueueReceive(adc_to_pwm_task_queue, &adc_reading, 100)) {
      percent = (adc_reading * 100) / 0xfff;
      pwm1__set_duty_cycle(PWM1__2_0, percent);
    }
    //    vTaskDelay(100);
  }
}

void adc_task(void *p) {
  adc__initialize();

  // TODO This is the function you need to add to adc.h
  // You can configure burst mode for just the channel you are using
  adc__enable_burst_mode();

  // Configure a pin, such as P1.31 with FUNC 011 to route this pin as ADC channel 5
  // You can use gpio__construct_with_function() API from gpio.h
  gpio_s pin = gpio__construct_with_function(GPIO__PORT_0, /*Pin*/ 25, GPIO__FUNCTION_1);
  LPC_IOCON->P0_25 &= ~(1 << 7);

  int adc_reading = 0; // Note that this 'adc_reading' is not the same variable as the one from adc_task
  while (1) {
    // Get the ADC reading using a new routine you created to read an ADC burst reading
    const uint16_t adc_value = adc__get_channel_reading_with_burst_mode(ADC__CHANNEL_2);
    float adc_voltage = (float)adc_value / 4095 * 3.3;
    fprintf(stderr, "ADC Voltage: %f\n", adc_voltage);
    // Implement code to send potentiometer value on the queue
    // a) read ADC input to 'int adc_reading'
    adc_reading = adc_value;
    // b) Send to queue: xQueueSend(adc_to_pwm_task_queue, &adc_reading, 0);
    xQueueSend(adc_to_pwm_task_queue, &adc_reading, 0);
    vTaskDelay(100);
  }
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
