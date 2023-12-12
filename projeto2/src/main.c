#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h> /* Required for  I2C */
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys_clock.h>

#include "../include/rtdb.h"

#define STACKSIZE 1024
#define THREAD0_PRIORITY 7
#define TASK1_PERIOD 1000
#define SLEEP_TIME_MS 100
#define MAX_LEDS 4
#define UPDATE_PERIOD_MS 1000

void callback_btn0(const struct device *dev, struct gpio_callback *cb,
                   gpio_port_pins_t pins);
void callback_btn1(const struct device *dev, struct gpio_callback *cb,
                   gpio_port_pins_t pins);
void callback_btn2(const struct device *dev, struct gpio_callback *cb,
                   gpio_port_pins_t pins);
void callback_btn3(const struct device *dev, struct gpio_callback *cb,
                   gpio_port_pins_t pins);

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
static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

/* LED2_NODE is the devicetree node identifier for the "led0" alias. */
#define LED1_NODE DT_ALIAS(led1)
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);

/* LED3_NODE is the devicetree node identifier for the "led0" alias. */
#define LED2_NODE DT_ALIAS(led2)
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(LED2_NODE, gpios);

/* LED4_NODE is the devicetree node identifier for the "led0" alias. */
#define LED3_NODE DT_ALIAS(led3)
static const struct gpio_dt_spec led3 = GPIO_DT_SPEC_GET(LED3_NODE, gpios);

static const struct gpio_dt_spec leds_gpio[4] = {led, led1, led2, led3};

#define TC74_CMD_RTR 0x00  /* Read temperature command */
#define TC74_CMD_RWCR 0x01 /* Read/write configuration register */

/* I2C device vars and defines */
#define I2C0_NID DT_NODELABEL(tc74sensor)
static const struct i2c_dt_spec dev_i2c = I2C_DT_SPEC_GET(I2C0_NID);

// static struct k_thread_stack *Task1_stack_area;
K_THREAD_STACK_DEFINE(sync_io_thread_stack_area, STACKSIZE);
K_THREAD_STACK_DEFINE(temp_sampling_stack_area, STACKSIZE);

void sync_io_thread(void *, void *, void *);
void read_temp_samples(void *, void *, void *);

k_tid_t thds_ids[4];
struct k_thread thds[4];

void main(void) {
  int ret;

  if (!device_is_ready(led.port)) {
    return;
  }
  /* STEP 4 - Verify that the device is ready for use */
  if (!device_is_ready(led1.port)) {
    return;
  }

  if (!device_is_ready(led2.port)) {
    return;
  }
  /* STEP 4 - Verify that the device is ready for use */
  if (!device_is_ready(led3.port)) {
    return;
  }

  if (!device_is_ready(button.port)) {
    return;
  }
  /* STEP 4 - Verify that the device is ready for use */
  if (!device_is_ready(button2.port)) {
    return;
  }
  /* STEP 4 - Verify that the device is ready for use */
  if (!device_is_ready(button3.port)) {
    return;
  } /* STEP 4 - Verify that the device is ready for use */
  if (!device_is_ready(button4.port)) {
    return;
  }

  ret = gpio_pin_configure_dt(&led0, GPIO_OUTPUT_ACTIVE);
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

  if (!device_is_ready(dev_i2c.bus)) {
    printk("I2C bus %s is not ready!\n\r", dev_i2c.bus->name);
  } else {
    printk("I2C bus %s, device address %x ready\n\r", dev_i2c.bus->name,
           dev_i2c.addr);
  }
  /* Write (command RTR) to set the read address to temperature */
  /* Only necessary if a config done before (not the case), but let's stay in
   * the safe side */
  ret = i2c_write_dt(&dev_i2c, TC74_CMD_RTR, 1);
  if (ret != 0) {
    printk("Failed to write to I2C device at address %x, register %x \n\r",
           dev_i2c.addr, TC74_CMD_RTR);
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
  /* STEP 5 - Configure the pin connected to the button to void */
  ret = gpio_pin_interrupt_configure_dt(&button4, GPIO_INT_EDGE_TO_ACTIVE);
  if (ret < 0) {
    return;
  }

  struct gpio_callback gpio_callback_button4;
  gpio_init_callback(&gpio_callback_button4, callback_btn3, BIT(button4.pin));

  gpio_add_callback(button4.port, &gpio_callback_button4);
  struct k_thread sync_task_tid_data;

  thds_ids[0] = k_thread_create(
      &thds[0], sync_io_thread_stack_area,
      K_THREAD_STACK_SIZEOF(sync_io_thread_stack_area),
      sync_io_thread,      /* Pointer to code, i.e. the function name */
      NULL, NULL, NULL,    /* Three optional arguments */
      THREAD0_PRIORITY, 0, /* Thread options. Arch dependent */
      K_NO_WAIT);

  thds_ids[1] = k_thread_create(
      &thds[1], temp_sampling_stack_area,
      K_THREAD_STACK_SIZEOF(temp_sampling_stack_area),
      read_temp_samples,   /* Pointer to code, i.e. the function name */
      NULL, NULL, NULL,    /* Three optional arguments */
      THREAD0_PRIORITY, 0, /* Thread options. Arch dependent */
      K_NO_WAIT);

  int j = 0;
  int k = 0;

  while (1) {
    for (int i = 0; i < 2; i++) {
      k_thread_join(&thds[i], K_MSEC(1000));
    }
  }

  // start the threads

  return 0;
}

void callback_btn0(const struct device *dev, struct gpio_callback *cb,
                   const gpio_port_pins_t pins) {
  toggle_btn(0);
  // set_led(0, !get_led(0));
}
void callback_btn1(const struct device *dev, struct gpio_callback *cb,
                   const gpio_port_pins_t pins) {
  toggle_btn(1);
  // set_led(1, !get_led(1));
}
void callback_btn2(const struct device *dev, struct gpio_callback *cb,
                   const gpio_port_pins_t pins) {
  toggle_btn(2);
  // set_led(2, !get_led(2));
}
void callback_btn3(const struct device *dev, struct gpio_callback *cb,
                   const gpio_port_pins_t pins) {
  toggle_btn(3);
  // set_led(3, !get_led(3));
}
void sync_io_thread(void *, void *, void *) {
  int leds_values[4] = {0, 0, 0, 0};
  int i = 0;
  while (1) {
    get_leds(leds_values);
    for (i = 0; i < MAX_LEDS; i++) {
      gpio_pin_set_dt(&(leds_gpio[i]), leds_values[i]);
      // printk("Setting led %d -> %d\n", i, leds_values[i]);
    }

    k_msleep(1000);
  }
}

// TODO fix i2c
void read_temp_samples(void *, void *, void *) {
  uint8_t temp = 0; /* Temperature value (raw read from sensor)*/
  int ret;
  while (1) {
    /* Read temperature register */
    ret = i2c_read_dt(&dev_i2c, &temp, sizeof(temp));
    if (ret != 0) {
      printk("Failed to read from I2C device at address %x, register  at Reg. "
             "%x %d --- %d\n\r",
             dev_i2c.addr, TC74_CMD_RTR, ret, -EIO);
    }

    printk("Last temperature reading is %d \n\r", temp);
    add_temp(temp);

    /* Pause  */
    k_msleep(UPDATE_PERIOD_MS);
  }
}