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
#include "i2c.h"
#include "periodic_scheduler.h"
#include "pwm1.h"
#include "queue.h"
#include "semphr.h"
#include "sj2_cli.h"
#include "ssp_lab.h"
#include "uart_lab.h"

#include "gpio_isr.h"
#include "lpc_peripherals.h"

#include "acceleration.h"
#include "event_groups.h"
#include "ff.h"
#include <string.h>

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
#define Lab6 0
#define Lab7 0
#define WatchDog 0
#define i2c 0
#define mp3 1

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

#if Lab7
static QueueHandle_t switch_queue;

typedef enum { switch__off, switch__on } switch_e;

void producer(void *p);
void consumer(void *p);
#endif

#if WatchDog
static QueueHandle_t watchdog_queue;
static EventGroupHandle_t xCreateGroup;
void watchdog_main();
#endif

#if i2c
/*
   // pp. 636, 648 i2C
#define i2c1 1
#if i2c1
  LPC_IOCON->P0_0 &= ~0x41f;      // i2c_1
  LPC_IOCON->P0_1 &= ~0x41f;      // i2c_1
  LPC_IOCON->P0_0 |= 0x3;         // i2c_1
  LPC_IOCON->P0_0 |= (0x0 << 3);  // i2c_1
  LPC_IOCON->P0_0 |= (0x1 << 10); // i2c_1
  LPC_IOCON->P0_1 |= 0x3;         // i2c_1
  LPC_IOCON->P0_1 |= (0x3 << 3);  // i2c_1
  LPC_IOCON->P0_1 |= (0x1 << 10); // i2c_1
  i2c__initialize_slave(I2C__1);
#else
  LPC_IOCON->P1_30 &= ~0x7; // i2c_1
  LPC_IOCON->P1_31 &= ~0x7; // i2c_1
  LPC_IOCON->P1_30 |= 0x4;  // i2c_1
  LPC_IOCON->P1_31 |= 0x4;  // i2c_1
  i2c__initialize_slave(I2C__0);
#endif

  puts("I2c_1 Slave init");
  i2c__write_single(I2C__2, 0x70, 0, 0x77);
  uint8_t read = i2c__read_single(I2C__2, 0x70, 0);
  printf("Read: 0x%02X from 0x70\n", read);
  printf("Checking address: 0x%02X\n", 112);
  if (i2c__detect(I2C__2, 112)) {
    printf("I2C slave detected at address: 0x%02X\n", 112);
  }
 * */
static volatile uint8_t slave_memory[256];
bool i2c_slave_callback__read_memory(uint8_t memory_index, uint8_t *memory) {
  // TODO: Read the data from slave_memory[memory_index] to *memory pointer
  // TODO: return true if all is well (memory index is within bounds)
  bool status = true;
  // LPC_I2C2->ADRO0 |= memory_index;
  *memory = slave_memory[memory_index];
  if (LPC_I2C2->STAT == 0x78 || LPC_I2C2->STAT == 0xB0) {
    status = false;
  }
  return status;
}

bool i2c_slave_callback__write_memory(uint8_t memory_index, uint8_t memory_value) {
  // TODO: Write the memory_value at slave_memory[memory_index]
  // TODO: return true if memory_index is within bounds
  bool status = true;
  slave_memory[memory_index] = memory_value;
  if (LPC_I2C2->STAT == 0x78 || LPC_I2C2->STAT == 0xB0) {
    status = false;
  }
  return status;
}
#endif

#if mp3
extern QueueHandle_t song_name_queue;
static QueueHandle_t mp3_file_queue;
typedef char songname_t[32];
typedef char song_data_t[512];
static void read_file(const char *filename) {
  puts("Read and stored file name");
  FIL file;
  UINT bytes_written = 0;
  FRESULT result = f_open(&file, filename, (FA_READ | FA_OPEN_EXISTING));
  if (FR_OK == result) {
    song_data_t buffer = {};
    UINT bytes_to_read = 512;
    UINT bytes_done_reading = 1;
    while (bytes_done_reading > 0) {
      FRESULT rd = f_read(&file, buffer, bytes_to_read, &bytes_done_reading);
      xQueueSend(mp3_file_queue, buffer, portMAX_DELAY);
      if (FR_OK == rd) {
        printf("Read %d bytes\n", bytes_done_reading);
      }
    }
    f_close(&file); // not sure when to close file
  } else {
    puts("Unavailable song");
  }
}
static void mp3_file_reader_task(void *p) {
  songname_t s_name = {};
  while (1) {
    if (xQueueReceive(song_name_queue, &s_name, 3000)) {
      read_file(s_name);
    } else {
      puts("Queue did not receive item");
    }
  }
}
static void mp3_decoder_send_block(song_data_t s_data) {
  for (size_t index = 0; index < sizeof(song_data_t); index++) {
    vTaskDelay(3);
    putchar(s_data[index]);
  }
}
static void print_hex(song_data_t s_data) {
  for (size_t index = 0; index < sizeof(song_data_t); index++) {
    vTaskDelay(1);
    printf("%02x", s_data[index]);
  }
}
static void mp3_data_player_task(void *p) {
  song_data_t s_data = {};
  while (1) {
    memset(&s_data[0], 0, sizeof(song_data_t));
    if (xQueueReceive(mp3_file_queue, &s_data[0], portMAX_DELAY)) {
      // mp3_decoder_send_block(s_data);
      print_hex(s_data);
    }
  }
}
void milestone_1_main() {
  song_name_queue = xQueueCreate(1, sizeof(songname_t));
  mp3_file_queue = xQueueCreate(2, sizeof(song_data_t));
  // TaskHandle_t get_name;
  xTaskCreate(mp3_file_reader_task, "reader", 512, NULL, PRIORITY_MEDIUM, NULL);
  xTaskCreate(mp3_data_player_task, "player", 512, NULL, PRIORITY_HIGH, NULL);
  // xTaskCreate(get_song_name_task, "Get Song Name", 1, NULL, PRIORITY_MEDIUM,&get_name);
}
#endif

int main(void) { // main function for project
  puts("Starting RTOS");
  create_uart_task();
#if outOfTheBox
  create_blinky_tasks();
#else
  milestone_1_main();

  puts("Main done");
#endif
  vTaskStartScheduler(); // This function never returns unless RTOS scheduler runs out of memory and fails
  return 0;
}

#if WatchDog
/*WatchDog Group Lab*/

void write_file_using_fatfs_pi(acceleration__axis_data_s receive_value) {
  const char *filename = "WatchDog_Data.csv";
  FIL file; // File handle
  UINT bytes_written = 0;

  // Open for the first time
  FRESULT result = f_open(&file, filename, (FA_WRITE | FA_CREATE_NEW));
  if (FR_OK == result) {
    char string[64] = "";
    // sprintf(string, "Value,%i\n", 123);
    sprintf(string, "x,y,z\n");
    if (FR_OK == f_write(&file, string, strlen(string), &bytes_written)) {
    } else {
      printf("ERROR: Failed to write data to file\n");
    }
    sprintf(string, "%d,%d,%d\n", receive_value.x, receive_value.y, receive_value.z);
    if (FR_OK == f_write(&file, string, strlen(string), &bytes_written)) {
    } else {
      printf("ERROR: Failed to write data to file\n");
    }
    f_close(&file);

  } else {
    result = f_open(&file, filename, (FA_WRITE | FA_OPEN_APPEND));
    if (FR_OK == result) {
      char string[64];
      // sprintf(string, "Value,%i\n", 123);
      sprintf(string, "%d,%d,%d\n", receive_value.x, receive_value.y, receive_value.z);
      // fprintf(stderr, "printed\n");
      if (FR_OK == f_write(&file, string, strlen(string), &bytes_written)) {
      } else {
        printf("ERROR: Failed to write data to file\n");
      }
      f_close(&file);

    } else {
      printf("ERROR %d: Failed to open: %s\n", result, filename);
    }
  }
}

acceleration__axis_data_s get_send_value() {
  acceleration__axis_data_s avg;
  int xVal = 0;
  int yVal = 0;
  int zVal = 0;
  for (int i = 0; i < 100; i++) {
    xVal += acceleration__get_data().x;
    yVal += acceleration__get_data().y;
    zVal += acceleration__get_data().z;
    delay__ms(1);
  }
  avg.x = xVal / 100;
  avg.y = yVal / 100;
  avg.z = zVal / 100;
  return avg;
}

void producer_of_sensor(void *p) {
  // fprintf(stderr, "producer\n");
  while (1) {
    acceleration__axis_data_s send_value = get_send_value();
    xQueueSend(watchdog_queue, &send_value, 0);
    xEventGroupSetBits(xCreateGroup, (1 << 1));
    vTaskDelay(1000);
  }
}

// TODO: Create this task at PRIORITY_HIGH
void consumer_of_sensor(void *p) {
  acceleration__axis_data_s receive_value;
  while (1) {
    // TODO: Print a message before xQueueReceive()
    // fprintf(stderr, "before xqueuereceive\n");
    if (xQueueReceive(watchdog_queue, &receive_value, portMAX_DELAY)) {
      //      fprintf(stderr, "x: %d, y: %d, z: %d\n", receive_value.x, receive_value.y, receive_value.z);
      write_file_using_fatfs_pi(receive_value);
    }
    xEventGroupSetBits(xCreateGroup, (1 << 2));
  }
}

void watchdog_task(void *params) {
  while (1) {
    vTaskDelay(1000);
    EventBits_t check = xEventGroupWaitBits(xCreateGroup, (1 << 1) | (1 << 2), pdTRUE, pdFALSE, 200);
    if (((check & (1 << 1)) != 0) && ((check & (1 << 2)) != 0)) {
      // Was able to complete both tasks. Be Humble
      //      fprintf(stderr, "Successfully completed both tasks\n");
    } else {
      // fprintf(stderr, "Producer task unfinished\n");
      if (check & (1 << 1)) {
        fprintf(stderr, "Consumer task unfinished\n");
      } else if (check & (1 << 2)) {
        fprintf(stderr, "Producer task unfinished\n");
      } else {
        fprintf(stderr, "Both tasks didnt finish\n");
      }
    }
  }
}
void watchdog_main() {
  if (acceleration__init()) {
    fprintf(stderr, "Acceleration Initialized\n");
  }
  TaskHandle_t prod_watchdog;
  TaskHandle_t cons_watchdog;
  TaskHandle_t watchdog;
  xTaskCreate(producer_of_sensor, "producer", 1024, NULL, 2, &prod_watchdog);
  xTaskCreate(consumer_of_sensor, "consumer", 1024, NULL, 2, &cons_watchdog);
  xTaskCreate(watchdog_task, "watchdog", 1024, NULL, 3, &watchdog);
  // TODO Queue handle is not valid until you create it
  watchdog_queue = xQueueCreate(
      1, sizeof(acceleration__axis_data_s)); // Choose depth of item being our enum (1 should be okay for this example
  xCreateGroup = xEventGroupCreate();
}
#endif

#if Lab7
/*

  xTaskCreate(producer, /*description*/ "producer", /*stack depth*/ 4096 / sizeof(void *),
/*parameter*/ (void *)1,
/*priority*/ 1, /*optional handle*/ NULL);
xTaskCreate(consumer, /*description*/ "consumer", /*stack depth*/ 4096 / sizeof(void *),
            /*parameter*/ (void *)1,
            /*priority*/ 2, /*optional handle*/ NULL);

// TODO Queue handle is not valid until you create it
switch_queue =
    xQueueCreate(1, sizeof(switch_e)); // Choose depth of item being our enum (1 should be okay for this example)

** /
    // TODO: Create this task at PRIORITY_LOW
    void producer(void *p) {
  //  // SW3 p0.29
  LPC_IOCON->P0_29 &= ~(0x3 << 3);
  LPC_IOCON->P0_29 |= (1 << 3); // pull down
  LPC_GPIO0->DIR &= ~(1 << 29); // set as input

  while (1) {
    // This xQueueSend() will internally switch context to "consumer" task because it is higher priority than this
    // "producer" task Then, when the consumer task sleeps, we will resume out of xQueueSend()and go over to the next
    // line

    // TODO: Get some input value from your board
    const switch_e switch_value = (LPC_GPIO0->PIN & (1 << 29)) >> 29; // value of switch

    // TODO: Print a message before xQueueSend()
    // Note: Use printf() and not fprintf(stderr, ...) because stderr is a polling printf
    printf("sending switch_value: %d\n", switch_value);
    xQueueSend(switch_queue, &switch_value, 0);
    // TODO: Print a message after xQueueSend()
    printf("Returned to producer task\n");

    vTaskDelay(1000);
  }
}

// TODO: Create this task at PRIORITY_HIGH
void consumer(void *p) {
  switch_e switch_value;
  while (1) {
    // TODO: Print a message before xQueueReceive()
    printf("waiting to consume a value from the queue\n");
    xQueueReceive(switch_queue, &switch_value, portMAX_DELAY);
    // TODO: Print a message after xQueueReceive()
    printf("received switch_value: %d\n", switch_value);
  }
}
#endif

#if Lab6
/*
   // TODO: Use uart_lab__init() function and initialize UART2 or UART3 (your choice)
  // TODO: Pin Configure IO pins to perform UART2/UART3 function
  uart_lab__init(UART_2, 96, 115200);
  uart__enable_receive_interrupt(UART_2);
  uart_lab__init(UART_3, 96, 115200);
  uart__enable_receive_interrupt(UART_3);
   xTaskCreate(producer, /*description*/ "uart_task", /*stack depth*/ 4096 / sizeof(void *),
              /*parameter*/ (void *)1,
              /*priority*/ 1, /*optional handle*/ NULL);
xTaskCreate(consumer, /*description*/ "uart_task2", /*stack depth*/ 4096 / sizeof(void *),
            /*parameter*/ (void *)1,
            /*priority*/ 2, /*optional handle*/ NULL);
** / void uart_read_task(void *p) {
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
      uart_lab__polled_put(UART_2, number_as_string[i]);
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
