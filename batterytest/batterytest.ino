#include <Arduino.h>
#include "driver/adc.h"

// Test for ADC battery test signal

void setup() {
  Serial.begin(115200);

  int adc1_result = adc1_get_raw(ADC1_CHANNEL_0);
  int adc2_result = 0;
  adc2_get_raw(ADC2_CHANNEL_0,ADC_WIDTH_BIT_9,&adc2_result);
  Serial.print("\nADC1: ");
  Serial.println(adc1_result);
  Serial.print("ADC2: ");
  Serial.println(adc2_result);
  Serial.println("\nPress EN on the board to test again.");
}

void loop() { }