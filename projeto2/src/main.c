#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys_clock.h>

#include "./include/rtdb.h"

#define STACKSIZE 1024
#define THREAD0_PRIORITY 7
#define TASK1_PERIOD 1000
#define SLEEP_TIME_MS 100
#define MAX_LEDS 4

/* Button setup */
#define SW0_NODE DT_ALIAS(sw0)
static const struct gpio_dt_spec button0 = GPIO_DT_SPEC_GET(SW0_NODE, gpios);
#define SW1_NODE DT_ALIAS(sw1)
static const struct gpio_dt_spec button1 = GPIO_DT_SPEC_GET(SW1_NODE, gpios);
#define SW2_NODE DT_ALIAS(sw2)
static const struct gpio_dt_spec button2 = GPIO_DT_SPEC_GET(SW2_NODE, gpios);
#define SW3_NODE DT_ALIAS(sw3)
static const struct gpio_dt_spec button3 = GPIO_DT_SPEC_GET(SW3_NODE, gpios);

/* LED configuration */
#define LED0_NODE DT_ALIAS(led0)
static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
#define LED1_NODE DT_ALIAS(led1)
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
#define LED2_NODE DT_ALIAS(led2)
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(LED2_NODE, gpios);
#define LED3_NODE DT_ALIAS(led3)
static const struct gpio_dt_spec led3 = GPIO_DT_SPEC_GET(LED3_NODE, gpios);

static const struct gpio_dt_spec leds_gpio[4] = {led0, led1, led2, led3};

// static struct k_thread_stack *Task1_stack_area;
K_THREAD_STACK_DEFINE(sync_io_thread_stack_area, STACKSIZE);

void callback_btn0(
    const struct device *dev, struct gpio_callback *cb, gpio_port_pins_t pins);
void callback_btn1(
    const struct device *dev, struct gpio_callback *cb, gpio_port_pins_t pins);
void callback_btn2(
    const struct device *dev, struct gpio_callback *cb, gpio_port_pins_t pins);
void callback_btn3(
    const struct device *dev, struct gpio_callback *cb, gpio_port_pins_t pins);
void sync_io_thread(void *, void *, void *);
int check_devices();
int leds_initialization();
int btns_initialization();

void main(void)
{

  int ret;

  if (!check_devices())
  {
    return;
  }

  if (leds_initialization() < -1)
  {
    return;
  }

  if (btns_initialization() < -1)
  {
    return;
  }

  struct k_thread sync_task_tid_data;

  k_tid_t sync_task_tid = k_thread_create(
      &sync_task_tid_data, sync_io_thread_stack_area,
      K_THREAD_STACK_SIZEOF(sync_io_thread_stack_area),
      sync_io_thread,      /* Pointer to code, i.e. the function name */
      NULL, NULL, NULL,    /* Three optional arguments */
      THREAD0_PRIORITY, 0, /* Thread options. Arch dependent */
      K_NO_WAIT);

  int j = 0;
  int k = 0;
  while (1)
  {
    if (j % 4)
    {
      set_led(1, k);
    }
    k = (k + 1) % 2;
    k_msleep(SLEEP_TIME_MS);
  }

  return;
}

/* CHECK IF THE DEVICES ARE READY */
int check_devices()
{
  return device_is_ready(led0.port) && device_is_ready(led1.port) &&
         device_is_ready(led2.port) && device_is_ready(led3.port) &&
         device_is_ready(button.port) && device_is_ready(button2.port) &&
         device_is_ready(button3.port) && device_is_ready(button4.port);
}

/* INITIALIZE LEDS */
int leds_initialization()
{
  if (gpio_pin_configure_dt(&led0, GPIO_OUTPUT_ACTIVE) < 0)
  {
    return -1;
  }
  if (gpio_pin_configure_dt(&led1, GPIO_OUTPUT_ACTIVE) < 0)
  {
    return -1;
  }
  if (gpio_pin_configure_dt(&led2, GPIO_OUTPUT_ACTIVE) < 0)
  {
    return -1;
  }
  if (gpio_pin_configure_dt(&led3, GPIO_OUTPUT_ACTIVE) < 0)
  {
    return -1;
  }

  gpio_pin_set_dt(&led0, 0);
  gpio_pin_set_dt(&led1, 0);
  gpio_pin_set_dt(&led2, 0);
  gpio_pin_set_dt(&led3, 0);

  return 0;
}

/* INITIALIZE BUTTONS */
int btns_initialization()
{
  /* BUTTON 0 */
  ret = gpio_pin_configure_dt(&button0, GPIO_INPUT);
  if (ret < 0)
  {
    return -1;
  }
  ret = gpio_pin_interrupt_configure_dt(&button0, GPIO_INT_EDGE_TO_ACTIVE);
  if (ret < 0)
  {
    return -1;
  }
  struct gpio_callback gpio_callback_button0;
  gpio_init_callback(&gpio_callback_button0, callback_btn0, BIT(button.pin));
  gpio_add_callback(button0.port, &gpio_callback_button0);

  /* BUTTON1 */
  ret = gpio_pin_configure_dt(&button1, GPIO_INPUT);
  if (ret < 0)
  {
    return -1;
  }
  ret = gpio_pin_interrupt_configure_dt(&button1, GPIO_INT_EDGE_TO_ACTIVE);
  if (ret < 0)
  {
    return -1;
  }
  struct gpio_callback gpio_callback_button1;
  gpio_init_callback(&gpio_callback_button1, callback_btn1, BIT(button1.pin));
  gpio_add_callback(button1.port, &gpio_callback_button1);

  /* BUTTON2 */
  ret = gpio_pin_configure_dt(&button2, GPIO_INPUT);
  if (ret < 0)
  {
    return -1;
  }
  ret = gpio_pin_interrupt_configure_dt(&button2, GPIO_INT_EDGE_TO_ACTIVE);
  if (ret < 0)
  {
    return -1;
  }
  struct gpio_callback gpio_callback_button2;
  gpio_init_callback(&gpio_callback_button2, callback_btn2, BIT(button2.pin));
  gpio_add_callback(button2.port, &gpio_callback_button2);

  /* BUTTON3 */
  ret = gpio_pin_configure_dt(&button3, GPIO_INPUT);
  if (ret < 0)
  {
    return -1;
  }
  ret = gpio_pin_interrupt_configure_dt(&button3, GPIO_INT_EDGE_TO_ACTIVE);
  if (ret < 0)
  {
    return -1;
  }
  struct gpio_callback gpio_callback_button3;
  gpio_init_callback(&gpio_callback_button3, callback_btn3, BIT(button3.pin));
  gpio_add_callback(button3.port, &gpio_callback_button3);

  return 0;
}

/* BUTTON CALLBACKS */
void callback_btn0(
    const struct device *dev, struct gpio_callback *cb, const gpio_port_pins_t pins)
{
  toggle_btn(0);
}
void callback_btn1(
    const struct device *dev, struct gpio_callback *cb, const gpio_port_pins_t pins)
{
  toggle_btn(1);
}
void callback_btn2(
    const struct device *dev, struct gpio_callback *cb, const gpio_port_pins_t pins)
{
  toggle_btn(2);
}
void callback_btn3(
    const struct device *dev, struct gpio_callback *cb, const gpio_port_pins_t pins)
{
  toggle_btn(3);
}

void sync_io_thread(void *, void *, void *)
{
  int leds_values[4] = {0, 0, 0, 0};
  int i = 0;
  while (1)
  {
    get_leds(leds_values);
    for (i = 0; i < MAX_LEDS; i++)
    {
      gpio_pin_set_dt(&(leds_gpio[i]), leds_values[i]);
      printk("Setting led %d -> %d\n", i, leds_values[i]);
    }
    k_msleep(100);
  }
}