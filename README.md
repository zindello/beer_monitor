beer_monitor
============

A quick, hacky, monitor for brewing beer

This sketch watches the temperature of both the Ice Chamber and the Wort Chamber in my version "A Son of a Fermentation Chiller". When the temperature threshold is reached in the Wort chamber, it switches on the fan to pump the cold air from the Ice Chamber to the Wort Chamber. It also has an overtemp alarm for the Wort Chamber (Incase the fan fails) and an overtemp alarm for the Ice Chamber (To tell you that the ice needs to be changed). It also allows you to silence the alarm. The alarms will reset if the thresholds are breached, and then the temp drops back below the threshold. 

This sketch leverages an Arduino Duemilanove and the following peripherals:

- 2 FreeTronics temperature sensors
- 1 FreeTronics Sound/Buzzer module
- 1 FreeTronics Header Shield
- 1 FreeTronics LCD Keypad Shield
- 1 N Type Fet
- 1 100mm 12V Case Fan
