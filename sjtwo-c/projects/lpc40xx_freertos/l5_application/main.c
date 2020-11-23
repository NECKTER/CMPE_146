#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

#include "board_io.h"
#include "common_macros.h"
#include "periodic_scheduler.h"
#include "queue.h"
#include "sj2_cli.h"

#include "audio_driver.h"
#include "lpc40xx.h"
#include "lpc_peripherals.h"
// 'static' to make these functions 'private' to this file
static void create_blinky_tasks(void);
static void create_uart_task(void);
static void blink_task(void *params);
static void uart_task(void *params);

static QueueHandle_t switch_queue;

typedef enum { switch__off, switch__on } switch_e;

switch_e get_switch_input_from_switch3() {
  switch_e a;
  if (LPC_GPIO0->PIN & (1 << 29)) {
    a = switch__on;
  } else {
    a = switch__off;
  }
  return a;
}

// TODO: Create this task at PRIORITY_LOW
void producer(void *p) {
  // fprintf(stderr, "producer\n");
  while (1) {
    // This xQueueSend() will internally switch context to "consumer" task because it is higher priority than this
    // "producer" task Then, when the consumer task sleeps, we will resume out of xQueueSend()and go over to the next
    // line

    // TODO: Get some input value from your board
    LPC_IOCON->P0_29 &= ~(0x7);
    LPC_GPIO0->CLR |= (0x1 << 29);
    const switch_e switch_value = get_switch_input_from_switch3();

    // TODO: Print a message before xQueueSend()
    // Note: Use printf() and not fprintf(stderr, ...) because stderr is a polling printf
    fprintf(stderr, "before xqueuesend\n");
    // printf("send");
    xQueueSend(switch_queue, &switch_value, 0);
    // TODO: Print a message after xQueueSend()
    fprintf(stderr, "after xqueuesend\n");
    // fprintf(stderr, "------------------producer done--------------");
    vTaskDelay(1000);
  }
}

// TODO: Create this task at PRIORITY_HIGH
void consumer(void *p) {
  // fprintf(stderr, "consumer\n");
  switch_e switch_value;
  while (1) {
    // TODO: Print a message before xQueueReceive()
    fprintf(stderr, "before xqueuereceive\n");
    xQueueReceive(switch_queue, &switch_value, portMAX_DELAY);
    // TODO: Print a message after xQueueReceive()
    if (switch_value == switch__on) {
      fprintf(stderr, "after xqueuereceive(high)\n");
    } else {
      fprintf(stderr, "after xqueuereceive(low)\n");
    }
    // fprintf(stderr, "------------------consumer done--------------");
    // printf("after xqueuereceive: ");
  }
}

void producer_consumer_assignment() {
  // TODO: Create your tasks
  // xTaskCreate(producer, ...);
  // xTaskCreate(consumer, ...);
  TaskHandle_t prod;
  TaskHandle_t cons;
  xTaskCreate(producer, "producer", 1024, NULL, 1, &prod);
  xTaskCreate(consumer, "consumer", 1024, NULL, 2, &cons);
  // TODO Queue handle is not valid until you create it
  switch_queue =
      xQueueCreate(1, sizeof(switch_e)); // Choose depth of item being our enum (1 should be okay for this example)
}

#include "acceleration.h"
#include "event_groups.h"
#include "ff.h"
#include <string.h>
int file_count = 0;
void write_file_using_fatfs_pi(acceleration__axis_data_s receive_value) {
  delay__ms(2);
  // file_count++;
  const char *filename = "file_plot.txt";
  FIL file; // File handle
  UINT bytes_written = 0;
  FRESULT result = f_open(&file, filename, (FA_WRITE | FA_OPEN_APPEND));

  if (FR_OK == result) {
    char string[64];
    // sprintf(string, "Value,%i\n", 123);
    sprintf(string, "x: %d, y: %d, z: %d\n", receive_value.x, receive_value.y, receive_value.z);
    // fprintf(stderr, "printed\n");
    if (FR_OK == f_write(&file, string, strlen(string), &bytes_written)) {
    } else {
      printf("ERROR: Failed to write data to file\n");
    }
    f_close(&file);

  } else {
    printf("ERROR: Failed to open: %s\n", filename);
  }
}

void write_file_using_fatfs_pi_error(int err) {
  delay__ms(2);
  // file_count++;
  const char *filename = "error.txt";
  FIL file; // File handle
  UINT bytes_written = 0;
  FRESULT result = f_open(&file, filename, (FA_WRITE | FA_OPEN_APPEND));

  if (FR_OK == result) {
    char string[64];
    // sprintf(string, "Value,%i\n", 123);
    // sprintf(string, "x: %d, y: %d, z: %d\n", receive_value.x, receive_value.y, receive_value.z);
    if (err == 1) {
      sprintf(string, "Both tasks didn't finish\n");
    } else if (err == 2) {
      sprintf(string, "Consumer task didn't finish\n");
    }

    // fprintf(stderr, "printed\n");
    if (FR_OK == f_write(&file, string, strlen(string), &bytes_written)) {
    } else {
      printf("ERROR: Failed to write data to file\n");
    }
    f_close(&file);

  } else {
    printf("ERROR: Failed to open: %s\n", filename);
  }
}

static QueueHandle_t watchdog_queue;
static EventGroupHandle_t xCreateGroup;
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
    // This xQueueSend() will internally switch context to "consumer" task because it is higher priority than this
    // "producer" task Then, when the consumer task sleeps, we will resume out of xQueueSend()and go over to the next
    // line

    // TODO: Get some input value from your board
    acceleration__axis_data_s send_value = get_send_value();

    // TODO: Print a message before xQueueSend()
    // Note: Use printf() and not fprintf(stderr, ...) because stderr is a polling printf
    // fprintf(stderr, "before xqueuesend\n");
    // printf("send");
    xQueueSend(watchdog_queue, &send_value, 0);
    // TODO: Print a message after xQueueSend()
    // fprintf(stderr, "after xqueuesend\n");
    // fprintf(stderr, "------------------producer done--------------");
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
      uint32_t time = xTaskGetTickCount();
      fprintf(stderr, "x: %d, y: %d, z: %d, time: %d\n", receive_value.x, receive_value.y, receive_value.z, time);
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
      fprintf(stderr, "Successfully completed both tasks\n");
    } else {
      // fprintf(stderr, "Producer task unfinished\n");
      int err;
      if (check & (1 << 1)) {
        fprintf(stderr, "Consumer task unfinished\n");
        err = 1;
        write_file_using_fatfs_pi_error(err);
      } else if (check & (1 << 2)) {
        fprintf(stderr, "Producer task unfinished\n");
        err = 2;
        write_file_using_fatfs_pi_error(err);
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

#include "i2c_slave_init.h"
#include "uart_lab.h"

void i2c_part_3_a() {
  i2c2__slave_init(0xFE);
  return -1;
}

extern QueueHandle_t song_name_queue;
static QueueHandle_t mp3_file_queue;
static QueueHandle_t lcd_song_name;
const int NUM_OF_SONGS = 2;
typedef char songname_t[32];
typedef char song_data_t[512];
bool pause_music_interrupt = false;
// songname_t song_array[2];
char song_array[2][32];
int song_count = 0;
bool new_song_interrupt = false;
uint16_t adc_value;
// static void get_song_name_task(void *p) {
//   songname_t s_name = {0};
//   strncpy(s_name, *p, sizeof(s_name)); // How do I pass in the string value from the CLI?
//   // That value will be the file name, then pass the song
//   if (xQueueSend(song_name_queue, &s_name, 0)) {
//     puts("Songname on queue");
//   } else {
//     puts("Songname failed to queue");
//   }
//   vTaskSuspend(NULL);
// }

static void read_file(const char *filename) {
  puts("Read and stored file name");
  FIL file;
  UINT bytes_written = 0;
  new_song_interrupt = false;
  FRESULT result = f_open(&file, filename, (FA_READ | FA_OPEN_EXISTING));
  if (FR_OK == result) {
    song_data_t buffer = {};
    UINT bytes_to_read = 512;
    UINT bytes_done_reading = 1;
    while (bytes_done_reading > 0) {
      if (new_song_interrupt) {
        break;
      }
      if (!pause_music_interrupt) {
        FRESULT rd = f_read(&file, buffer, bytes_to_read, &bytes_done_reading);
        xQueueSend(mp3_file_queue, buffer, portMAX_DELAY);
      } else {
        bytes_done_reading = 1;
      }
      // FRESULT rd = f_read(&file, buffer, bytes_to_read, &bytes_done_reading);
      // xQueueSend(mp3_file_queue, buffer, portMAX_DELAY);

      // if (FR_OK == rd) {
      //   printf("%d bytes\n", bytes_done_reading);
      // }
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
      // print_hex(s_data);
      play_data(&s_data, 512);
    }
  }
}
TaskHandle_t read_handle;
TaskHandle_t play_handle;
void milestone_1_main() {
  song_name_queue = xQueueCreate(1, sizeof(songname_t));
  mp3_file_queue = xQueueCreate(2, sizeof(song_data_t));

  xTaskCreate(mp3_file_reader_task, "reader", 512, NULL, 2, &read_handle);
  xTaskCreate(mp3_data_player_task, "player", 512, NULL, 1, &play_handle);
  // xTaskCreate(get_song_name_task, "Get Song Name", 1, NULL, PRIORITY_MEDIUM, &get_name);
}

// #include "LCD_display.h"
#include "adc.h"
#include "gpio.h"
#include "gpio_isr.h"
#include "ssp2.h"

void spi_task(void *p) {
  reset();
  ssp2__initialize(24);
  // gpio__set(xdcs_pin);
  while (1) {
    // write_decoder();
    sci_write(0x0B, 0x050E);
    uint16_t p = sci_read(0x0B);
    // uint8_t p = 5;
    vTaskDelay(100);
    fprintf(stderr, "Version: %04x\n", p);
  }
}
void milestone_2_main() {
  gpio__construct_with_function(GPIO__PORT_1, 4, GPIO__FUNCTION_4); // mosi
  gpio__construct_with_function(GPIO__PORT_1, 1, GPIO__FUNCTION_4); // miso
  gpio__construct_with_function(GPIO__PORT_1, 0, GPIO__FUNCTION_4); // sck
  // reset();
  ssp2__initialize(24);
  sine_test(2, 3000);
  begin();
  // xTaskCreate(spi_task, "task1", (512U * 4) / sizeof(void *), NULL, 1, NULL);
}

void pin29_resume() {
  fprintf(stderr, "29\n");
  pause_music_interrupt = false;
  // xTaskResumeFromISR(read_handle);
}

void pin30_pause() {
  fprintf(stderr, "30\n");
  pause_music_interrupt = true;
  // xTaskSuspendFromISR(read_handle);
}

void pin29_up() {
  fprintf(stderr, "up");
  new_song_interrupt = true;
  song_count++;
  if (song_count >= NUM_OF_SONGS) {
    song_count = 0;
  }
  xQueueSendFromISR(song_name_queue, song_array[song_count], NULL);
}

void pin29_volume() {
  fprintf(stderr, "vol");
  if (adc_value >= 2000) {
    set_volume(0x70, 0x3F);
  } else {
    set_volume(0x18, 0x18);
  }
}

void pin30_down() {
  fprintf(stderr, "down");
  new_song_interrupt = true;
  song_count--;
  if (song_count < 0) {
    song_count = NUM_OF_SONGS - 1;
  }
  xQueueSendFromISR(song_name_queue, song_array[song_count], NULL);
  // xQueueSendFromISR(lcd_song_name, song_array[song_count], NULL);
}

void gpio0_isr(void) { gpio0__interrupt_dispatcher(); }

void interrupt_setup() {
  // song_array[0] = "song_one.mp3";
  strcpy(song_array[0], "song_one.mp3");
  // song_array[1] = "song_two.mp3";
  strcpy(song_array[1], "song_two.mp3");
  gpio0__attach_interrupt(29, GPIO_INTR__RISING_EDGE, pin29_volume);
  gpio0__attach_interrupt(30, GPIO_INTR__RISING_EDGE, pin30_down);
  lpc_peripheral__enable_interrupt(LPC_PERIPHERAL__GPIO, gpio0_isr, "dispatcher");
  NVIC_EnableIRQ(GPIO_IRQn);
}

void adc_task(void *p) {
  adc__initialize();
  adc__enable_burst_mode(ADC__CHANNEL_5);
  LPC_IOCON->P1_31 &= ~(1 << 7);
  adc_value = 10;
  int currVal = 0;
  while (1) {
    adc_value = adc__get_channel_reading_with_burst_mode();
    // fprintf(stderr, "res:%d", adc_value);
    if (adc_value - currVal > 50 || adc_value - currVal < -50) {
      currVal = adc_value;
      if (adc_value < 500) {
        set_volume(0x00, 0x00);
      } else if (adc_value < 1000) {
        set_volume(0x0F, 0x0F);
      } else if (adc_value < 1500) {
        set_volume(0x30, 0x30);
      } else if (adc_value < 2000) {
        set_volume(0x4F, 0x4F);
      } else if (adc_value < 2500) {
        set_volume(0x5F, 0x5F);
      } else if (adc_value < 3000) {
        set_volume(0x6F, 0x6F);
      } else if (adc_value < 3500) {
        set_volume(0xBF, 0xBF);
      } else {
        set_volume(0xFE, 0xFE);
      }
    }

    vTaskDelay(2000);
  }
}
static gpio_s test_gp;

void adc_setup() { xTaskCreate(adc_task, "adc_task", (512U * 4) / sizeof(void *), NULL, 3, NULL); }

int main(void) {
  create_blinky_tasks();
  create_uart_task();
  pin_config();
  milestone_2_main();
  milestone_1_main();
  interrupt_setup();
  adc_setup();

  puts("Starting RTOS");

  vTaskStartScheduler(); // This function never returns unless RTOS scheduler runs out of memory and fails

  return 0;
}

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
