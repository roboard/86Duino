#ifndef TLC_pin_functions_h
#define TLC_pin_functions_h

#define pulse_pin(port, pin)        digitalWrite(pin, HIGH); digitalWrite(pin, LOW)
#define set_pin(port, pin)          digitalWrite(pin, HIGH)
#define clear_pin(port, pin)        digitalWrite(pin, LOW)
#define output_pin(ddr, pin)        pinMode(pin, OUTPUT)
#define pullup_pin(ddr, port, pin)  pinMode(pin, INPUT_PULLUP)

#endif
