#include "../src/include/rtDatabase.h"
#include <assert.h>
#include <stdio.h>

#define MAX_TEMPS 20
#define MAX_
int btns[4];
int leds[4];
double temps[20];
int temps_returned = 0;

int btns_suppossed[4] = {0, 0, 0, 0};
int leds_suppossed[4] = {0, 0, 0, 0};
double temps_suppossed[20] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                              0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int temps_returned_suppossed = 0;
  int i;
void assert_values();

int main() {

  openDb("db test");


  get_btns(&btns);
  get_leds(&leds);
  get_temps(&temps);
  assert_values();

  // test toggles
  printf("Test Toggles\n");
  toggle_btn(1);
  get_btns(&btns);

  btns_suppossed[1] = 1;
  assert_values();

  toggle_btn(1);
  get_btns(&btns);

  btns_suppossed[1] = 0;
  assert_values();

  // test set btn
  printf("Test set btn\n");
  {
    set_btn(1, 1);
    get_btns(&btns);

    btns_suppossed[1] = 1;
    assert_values();

    set_btn(1, 1);
    get_btns(&btns);

    btns_suppossed[1] = 1;
    assert_values();

    set_btn(0, 0);
    get_btns(&btns);

    btns_suppossed[0] = 0;
    assert_values();

    set_btn(0, 1);
    get_btns(&btns);

    btns_suppossed[0] = 1;
    assert_values();
  }

  // test set led
  printf("Test set led\n");
  {
    set_led(1, 1);
    get_leds(&leds);

    leds_suppossed[1] = 1;
    assert_values();
    printf("first\n");

    set_led(1, 1);
    get_leds(&leds);

    leds_suppossed[1] = 1;
    assert_values();
    printf("sec\n");

    set_led(0, 0);
    get_leds(&leds);

    leds_suppossed[0] = 0;
    assert_values();
    printf("th\n");

    set_led(0, 1);
    get_leds(&leds);

    leds_suppossed[0] = 1;
    assert_values();
    printf("fourth\n");
  }

  // test atomic sets
  printf("Test atomic sets\n");
    {
       btns[0] = 1;
       btns[1] = 0;
       btns[2] = 1;
       btns[3] = 0;

       btns_suppossed[0] = 1;
       btns_suppossed[1] = 0;
       btns_suppossed[2] = 1;
       btns_suppossed[3] = 0;
       set_btns(&btns);

        assert_values();

    }


  // test atomic sets leds
  printf("Test atomic sets leds\n");
    {
       leds[0] = 1;
       leds[1] = 0;
       leds[2] = 1;
       leds[3] = 0;

       leds_suppossed[0] = 1;
       leds_suppossed[1] = 0;
       leds_suppossed[2] = 1;
       leds_suppossed[3] = 0;
       set_leds(&leds);

        assert_values();
    }
    // test add temp
    printf("Test add_temp sets\n");
    {
        add_temp(21.3f);
        temps_returned = get_temps(&temps);
        temps_suppossed[0]=21.3f;
	temps_returned_suppossed = 1;
        assert_values();
    }

    // test add multiple temp
    printf("Test add_temp sets\n");
    {
        add_temp(21.3f);
        add_temp(22.3f);
        add_temp(23.3f);
        add_temp(24.3f);
        temps_returned = get_temps(&temps);
        temps_suppossed[0]=21.3f;
        temps_suppossed[1]=21.3f;
        temps_suppossed[2]=22.3f;
        temps_suppossed[3]=23.3f;
        temps_suppossed[4]=24.3f;
	temps_returned_suppossed = 5;
        assert_values();
    }


    // test add multiple temp
    printf("Test reset and add_temp sets\n");
    {
	reset_temps();
        add_temp(21.3f);
        add_temp(22.3f);
        add_temp(23.3f);
        add_temp(24.3f);
        temps_returned = get_temps(&temps);
        temps_suppossed[0]=21.3f;
        temps_suppossed[1]=22.3f;
        temps_suppossed[2]=23.3f;
        temps_suppossed[3]=24.3f;
	temps_returned_suppossed = 4;
        assert_values();
    }
}

void assert_values() {
  // assert initialized data
  for (i = 0; i < temps_returned_suppossed; i++) {
    if (i < 4) {
      assert(leds[i] == leds_suppossed[i]);
      assert(btns[i] == btns_suppossed[i]);
    }
      assert(temps[i] == temps_suppossed[i]);
  }
  assert(temps_returned == temps_returned_suppossed);
}
