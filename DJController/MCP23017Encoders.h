/*
  MCP23017Encoders.h - multiplex 8 interrupt driven rotary encoders on an MCP23017 chip over I2C
   
  Created by Curiosity Workshop / Michael Gerlicher, December 2020.

  The use of this library is at your own risk and does not include warantee for any purpose.  
  Please don't trust it with anything that can cause damage to ANYTHING including
  persons, property, the destruction of Earth or other celestial bodies.

  This library is dependent upon the adafruit MCP23017 library and it must be installed and included.

  To report problems, download updates and examples, suggest enhancements or get technical support, please visit my patreon page:
     
     www.patreon.com/curiosityworkshop
     www.github.com/curiosity-workshop


     Initial version released January 23 2021
     
*/


#ifndef MCP23017Encoders_h
#define MCP23017Encoders_h

#include "Adafruit_MCP23X17.h"

// Register bits
#define MCP_INT_MIRROR false  // Mirror inta to intb.
#define MCP_INT_ODR    false // Open drain.
#define MCP_ENCODERS 8          // how many encoders are attached to the MCP23017

class MCP23017Encoders
{
  public:

    MCP23017Encoders(int intPin);               // Constructor, intPin is the arduino interrupt pin that the MCP23017 is attached to

    void begin(void);                           // Sets up the MCP23017 chip
    int  read(int encoder);                     // Return current value of encoder (0-7)
    void write(int encoder, int value);         // write encoder with value  (int encoder, int value)
    void setAccel(int encoder, float value);    // set acceleration factor (currently under construction)
  private:
    
   
};


static void MCP23017EncoderISR(void);


#endif
