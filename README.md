# esp32-spwm-gptimer
achieve SPWM (sinusoidal PWM)  by the ESP32 microcontroller
generate 50hz SPWM wave

This example shows how to use the gptimer to generate a SPWM signal using the `LOW SPEED` mode.

Connect the GPIO to an oscilloscope to see the generated signal:
|ledc channel| GPIO  |
|:----------:|:-----:|
| Channel 0  | GPIO4 |
| Channel 1  | GPIO5 |

The example uses fixed PWM frequency of 15 kHz.The gptimer executes 1000000/CAR times per second.
