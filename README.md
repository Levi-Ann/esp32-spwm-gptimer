# esp32-spwm-timer
achieve SPWM (sinusoidal PWM)  by the ESP32 microcontroller

This example shows how to use the gptimer to generate a SPWM signal using the `LOW SPEED` mode.

Connect the GPIO to an oscilloscope to see the generated signal:
|ledc channel| GPIO  |
|:----------:|:-----:|
| Channel 0  | GPIO4 |
| Channel 1  | GPIO5 |

The example uses fixed PWM frequency of 15 kHz.The gptimer executes 1000000/CAR times per second.
