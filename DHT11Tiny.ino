
// DHT11 info:
// https://www.electronicwings.com/sensors-modules/dht11
// http://www.ocfreaks.com/basics-interfacing-dht11-dht22-humidity-temperature-sensor-mcu/

//
// ATtiny85: Reads in DHT11 sensor data by using external interrupt on only the rising edge of the signal: the high time + following low time is measured (timer1).
//
// All pulses are shifted into the 5 byte dht_data array. This might seem cumbersome, but this way it becomes unnecessary to evaluate the start of the
// signal: In the end the 40 data bits will in the right place. The checksum verifies the value (doesn't work with all 0 values).
//
// This doesn't work at 1 mhz. I've tried leaner approaches, different prescalers, PLL timer clock, bare minimum (post) processing etc.
// At 1 mhz the measured pulses would look good at a glance, but at least one erroneous pulse would always be in there.
//

//#define DEBUG;

byte dht11_pin = 2;

//volatile uint16_t ovf_counter;

#ifdef DEBUG
// only for debugging: DHT11: 40 data bits + leading bits / noise
const byte dht11_pulse_bits = 45;

volatile byte pulses[dht11_pulse_bits];
volatile byte shifted_bits;
volatile byte current_bit;
volatile uint16_t isr_counter;
#endif DEBUG

const byte dht11_bytes = 5;
// humidity: dht11_data[0], temperatur: dht11_data[2]
volatile byte dht11_data[dht11_bytes];

void setup(){
  set_dht11_pin_input();

  GIMSK = 1<<INT0;                 // external interrupt (pin 5): INT0_vect
  //MCUCR = 1<<ISC00;              // any change
  MCUCR = (1<<ISC01) | (1<<ISC00); // only rising
  //MCUCR = 1<<ISC01;              // only falling

//  noInterrupts();
  TCCR1 = 0;  // reset TCCR1-Register

  //TCCR1 &= 0xF0; // turn off timer clock (not tested)
  // see datasheet for all prescaler settings
  TCCR1 |= (1<<CS12) | (1<<CS10); // prescaler: 16

  // external 64 mhz timer clock: only tested it, it's not needed here
  //PLLCSR = 1<<PCKE | 1<<PLLE; 
  //PLLCSR |= 1<<LSM;               // low speed mode: 32 mhz
  //while(!(PLLCSR & (1<<PLOCK)));  // wait for stable PLL clock

  //TIMSK |= (1<<TOIE1);  // overflow: TIMER1_OVF_vect: overflowing of the timer indicates that no new data is receiced on the pin 
//  interrupts();

  Serial.begin(9600);
  Serial.println("DHT11 attiny interrupt version");
}

void set_dht11_pin_input(){
  // it seems to work without any form of pullup (4 pin DHT11 version)
  //pinMode(dht11_pin, INPUT);
  // use INPUT_PULLUP nevertheless
  //pinMode(dht11_pin, INPUT_PULLUP);
  DDRB &= ~(1<<dht11_pin);
  //digitalWrite(dht11_pin, HIGH);
  PORTB |= 1<<dht11_pin;
}

// timer overflow
//ISR(TIMER1_OVF_vect) {
//  ovf_counter++;
//}

ISR(INT0_vect) {
   // shift in the bit based on the timer value
   shift_in_dht11_bit(TCNT1);
   // reset the timer
   TCNT1 = 0;

   #ifdef DEBUG
      isr_counter++;
   #endif
}

bool dht11_get_reading(){
   Serial.print("reading dht11.. ");

   dht11_start();

   #ifdef DEBUG
      current_bit = 0;
   #endif DEBUG
   
   //ovf_counter = 0;
   //while (ovf_counter < 50);
   delay(4);

   // data should now be in place

   // checksum
   if (((dht11_data[0] + dht11_data[1] + dht11_data[2] + dht11_data[3]) & 0xFF) == dht11_data[4]){
      return true;
   }

   return false;
}

bool shift_in_dht11_bit(byte pulse_length){

   #ifdef DEBUG
      pulses[current_bit] = pulse_length;
      current_bit++;
   #endif
  
   bool new_bit = 0;
   
   if (pulse_length > 50){
      new_bit = 1;
   }

   // shift all the bits in the dht11_data array by one
   for (byte shift_byte = 0; shift_byte < dht11_bytes; shift_byte++){

      dht11_data[shift_byte] <<= 1;

      // the first 4 bytes get the first bit of the next byte
      if (shift_byte < 4 && dht11_data[shift_byte+1] & 0x80){
         dht11_data[shift_byte]++;
      } else if (shift_byte == 4){
         // the last byte gets the new bit
         if (new_bit){
            dht11_data[shift_byte]++;
            #ifdef DEBUG
                shifted_bits++;
            #endif
         }
      }
   }
}

void dht11_start(){
   // wake up dht11: pull low 18 ms 
   //pinMode(dht11_pin, OUTPUT);
   DDRB |= 1<<dht11_pin;
   //digitalWrite(dht11_pin, 0);
   PORTB &= ~(1<<dht11_pin);
   
   delay(18);
   // signal rises quicker by driving high, but it also works without doing this:
   //digitalWrite(dht11_pin, 1); 
   
   set_dht11_pin_input();
}

#ifdef DEBUG
void print_pulses(){
   for (byte i = 0; i < dht11_pulse_bits; i++){
      Serial.print(i+1);
      Serial.print(": ");
      Serial.print(pulses[i]);
      Serial.println();
   }
}
#endif

#ifdef DEBUG
void reset_pulses(){
   for (byte i = 0; i < dht11_pulse_bits; i++){
      pulses[i] = 0;
   }
}
#endif

#ifdef DEBUG
void print_debugging_info(){
   Serial.print("isr_counter: ");
   Serial.println(isr_counter);
   isr_counter = 0;
   //Serial.print("ovf_counter: ");
   //Serial.println(ovf_counter);
   Serial.print("shifted_bits: ");
   Serial.println(shifted_bits);
   shifted_bits = 0;

   Serial.print("dht11_data[1]: ");
   Serial.println(dht11_data[1]);
   Serial.print("dht11_data[3]: ");
   Serial.println(dht11_data[3]);
   Serial.print("dht11_data[4]: ");
   Serial.print(dht11_data[4]);
   Serial.println(" (checksum)");

   print_pulses();

   reset_pulses();
}
#endif

void loop(){

   if (dht11_get_reading()){
      Serial.print("success: humidity: ");
      Serial.print(dht11_data[0]);
      Serial.print(", temperatur: ");
      Serial.println(dht11_data[2]);    
   } else{
      Serial.println("failed");
   }

   #ifdef DEBUG
      print_debugging_info();
   #endif
   delay(5000);   
}
