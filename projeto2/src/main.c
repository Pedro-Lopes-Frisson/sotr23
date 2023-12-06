#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys_clock.h>

#include "./include/rtDatabase.h"

#define STACKSIZE 1024
#define THREAD0_PRIORITY 7
#define TASK1_PERIOD 1000
#define SLEEP_TIME_MS 100
void callback_btn0(const struct device *dev, struct gpio_callback *cb, gpio_port_pins_t pins);
void callback_btn1(const struct device *dev, struct gpio_callback *cb, gpio_port_pins_t pins);
void callback_btn2(const struct device *dev, struct gpio_callback *cb, gpio_port_pins_t pins);
void callback_btn3(const struct device *dev, struct gpio_callback *cb, gpio_port_pins_t pins);

void sync_io()

/* Button setup */
#define SW0_NODE DT_ALIAS(sw0)

static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(SW0_NODE, gpios);

#define SW1_NODE DT_ALIAS(sw1)

static const struct gpio_dt_spec button2 = GPIO_DT_SPEC_GET(SW1_NODE, gpios);

#define SW2_NODE DT_ALIAS(sw2)

static const struct gpio_dt_spec button3 = GPIO_DT_SPEC_GET(SW2_NODE, gpios);

#define SW3_NODE DT_ALIAS(sw3)

static const struct gpio_dt_spec button4 = GPIO_DT_SPEC_GET(SW3_NODE, gpios);

/* LED configuration */

/* LED0_NODE is the devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

/* LED2_NODE is the devicetree node identifier for the "led0" alias. */
#define LED1_NODE DT_ALIAS(led1)
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);

/* LED3_NODE is the devicetree node identifier for the "led0" alias. */
#define LED2_NODE DT_ALIAS(led2)
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(LED2_NODE, gpios);

/* LED4_NODE is the devicetree node identifier for the "led0" alias. */
#define LED3_NODE DT_ALIAS(led3)
static const struct gpio_dt_spec led3 = GPIO_DT_SPEC_GET(LED3_NODE, gpios);

// static struct k_thread_stack *Task1_stack_area;
K_THREAD_STACK_DEFINE(Task1_stack_area, STACKSIZE);
K_THREAD_STACK_DEFINE(Task2_stack_area, STACKSIZE);
K_THREAD_STACK_DEFINE(Task3_stack_area, STACKSIZE);
K_THREAD_STACK_DEFINE(Task4_stack_area, STACKSIZE);

int main(void) {

  int ret;

  if (!device_is_ready(led.port)) {
    return 1;
  }
  /* STEP 4 - Verify that the device is ready for use */
  if (!device_is_ready(led1.port)) {
    return 1;
  }

  if (!device_is_ready(led2.port)) {
    return 1;
  }
  /* STEP 4 - Verify that the device is ready for use */
  if (!device_is_ready(led3.port)) {
    return 1;
  }

  if (!device_is_ready(button.port)) {
    return 1;
  }
  /* STEP 4 - Verify that the device is ready for use */
  if (!device_is_ready(button2.port)) {
    return 1;
  }
  /* STEP 4 - Verify that the device is ready for use */
  if (!device_is_ready(button3.port)) {
    return 1;
  } /* STEP 4 - Verify that the device is ready for use */
  if (!device_is_ready(button4.port)) {
    return 1;
  }

  ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
  if (ret < 0) {
    return;
  }
  ret = gpio_pin_configure_dt(&led1, GPIO_OUTPUT_ACTIVE);
  if (ret < 0) {
    return;
  }
  ret = gpio_pin_configure_dt(&led2, GPIO_OUTPUT_ACTIVE);
  if (ret < 0) {
    return;
  }
  ret = gpio_pin_configure_dt(&led3, GPIO_OUTPUT_ACTIVE);
  if (ret < 0) {
    return;
  }

  gpio_pin_set_dt(&led, 0);
  gpio_pin_set_dt(&led1, 0);
  gpio_pin_set_dt(&led2, 0);
  gpio_pin_set_dt(&led3, 0);

  /* Button configuration */
  // Button 0
  ret = gpio_pin_configure_dt(&button, GPIO_INPUT);
  if (ret < 0) {
    return;
  }
  /* STEP 5 - Configure the pin connected to the button to be an input pin and
   * set its hardware specifications */
  ret = gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_TO_ACTIVE);
  if (ret < 0) {
    return;
  }

  struct gpio_callback gpio_callback_button;
  gpio_init_callback(&gpio_callback_button, callback_btn0, BIT(button.pin));

  gpio_add_callback(button.port, &gpio_callback_button);

  // Button 1
  ret = gpio_pin_configure_dt(&button2, GPIO_INPUT);
  if (ret < 0) {
    return;
  }
  /* STEP 5 - Configure the pin connected to the button to be an input pin and
   * set its hardware specifications */
  ret = gpio_pin_interrupt_configure_dt(&button2, GPIO_INT_EDGE_TO_ACTIVE);
  if (ret < 0) {
    return;
  }

  struct gpio_callback gpio_callback_button2;
  gpio_init_callback(&gpio_callback_button2, callback_btn1, BIT(button2.pin));

  gpio_add_callback(button2.port, &gpio_callback_button2);

  // Button 2
  ret = gpio_pin_configure_dt(&button3, GPIO_INPUT);
  if (ret < 0) {
    return;
  }
  /* STEP 5 - Configure the pin connected to the button to be an input pin and
   * set its hardware specifications */
  ret = gpio_pin_interrupt_configure_dt(&button3, GPIO_INT_EDGE_TO_ACTIVE);
  if (ret < 0) {
    return;
  }

  struct gpio_callback gpio_callback_button3;
  gpio_init_callback(&gpio_callback_button3, callback_btn2, BIT(button3.pin));

  gpio_add_callback(button3.port, &gpio_callback_button3);
  // Button 3
  ret = gpio_pin_configure_dt(&button4, GPIO_INPUT);
  if (ret < 0) {
    return;
  }
  /* STEP 5 - Configure the pin connected to the button to be an input pin and
   * set its hardware specifications */
  ret = gpio_pin_interrupt_configure_dt(&button4, GPIO_INT_EDGE_TO_ACTIVE);
  if (ret < 0) {
    return;
  }

  struct gpio_callback gpio_callback_button4;
  gpio_init_callback(&gpio_callback_button4, callback_btn3, BIT(button4.pin));

  gpio_add_callback(button4.port, &gpio_callback_button4);




  return 0;
}

void callback_btn0(void) { toggle_btn(0); }
void callback_btn1(void) { toggle_btn(1); }
void callback_btn2(void) { toggle_btn(2); }
void callback_btn3(void) { toggle_btn(3); }