# DHT11Tiny

ATtiny variant of [DHT11Light](https://github.com/chocotov1/DHT11Light). The ATtiny specific part lies in the use of exteral interrupt and timer1 settings. Can be ported to UNO.

## 1 MHz doesn't work
This version began in an effort to get a good reading with the chip running at 1 MHz. After trying a lot of different approaches, it seems that it's simply not possible. I added a remark in the code.

## New ideas
I did implemented some new ideas here though:
- Read in all the DHT11 bits and shift them in one by one. I reckoned that it saves a few bytes of flash because i wouldn't need to check the start long sequence.
- This version uses external interrupt (PCINT0) only on the rising edge instead of tracking the pin state.

## No space saving

Sadly, there didn't seem to be any space savings compared to my first version.

DHT11Light:
```
Sketch uses 986 bytes (12%) of program storage space. Maximum is 8192 bytes.
Global variables use 14 bytes (2%) of dynamic memory, leaving 498 bytes for local variables. Maximum is 512 bytes.
```

DHT11Tiny
```
Sketch uses 1034 bytes (12%) of program storage space. Maximum is 8192 bytes.
Global variables use 16 bytes (3%) of dynamic memory, leaving 496 bytes for local variables. Maximum is 512 bytes.
```
