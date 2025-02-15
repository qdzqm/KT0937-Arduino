# KT0937-Arduino
In this project, the KT0937-D8 chip can be controlled to achieve basic FM and MW band reception, and 2-Wire I2C can be used to initialize the chip, as well as realize the band switching and current frequency reading.

I2C uses the default Pins of Arduino, for example, for the Arduino Pro Mini, which are A4 and A5.

In order to reduce I2C interference on the radio, I would like to have I2C communication only when switching frequencies or switching FM/MW, and not at other times. Although it is possible to achieve this by the INT interrupt of Pin-7, I have adopted the method of reading the CH voltage to achieve a similar function. If you're going to use a button to switch frequencies, you can use something similar, I'm currently using a potentiometer to control the frequency.

You can also use a rotary encoder to control the frequency by controlling the GPIO output voltage of the MCU.

This project is just a minimal implementation of a radio based on the KT0937-D8 chip. In fact, you can delete a few lines of code for the register settings and the program will still function and maintain the basic functionality of the radio. As an amateur coder, mistakes are inevitable.
