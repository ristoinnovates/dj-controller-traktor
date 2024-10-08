#include <arduino.h>
#include "MCP23017Encoders.h"

int _MCP23017intPin;
Adafruit_MCP23X17 _mcp;

volatile struct mcpEncoder
{
    int tickValue;
    uint16_t previousBitMask;
    unsigned long msSinceLastRead;
    int currentValue;
    int previousValue; // Store previous value
    float accel;       // Accelerator value
    unsigned long lastDebounceTime; // Debouncing timer
} mcpEncoders[MCP_ENCODERS];

MCP23017Encoders::MCP23017Encoders(int intPin)
{
    _MCP23017intPin = intPin;
}

void MCP23017Encoders::begin()
{
    int i;

    _mcp.begin_I2C(); // Use default address 0

    pinMode(_MCP23017intPin, INPUT);

    _mcp.setupInterrupts(MCP_INT_MIRROR, MCP_INT_ODR, LOW); // The MCP output interrupt pin.
    for (i = 0; i < 16; i++)
    {
        _mcp.pinMode(i, INPUT_PULLUP);
        _mcp.setupInterruptPin(i, CHANGE);
    }

    _mcp.readGPIOAB(); // Initialize for interrupts.

    attachInterrupt(digitalPinToInterrupt(_MCP23017intPin), MCP23017EncoderISR, FALLING);

    for (i = 0; i < MCP_ENCODERS; i++)
    {
        mcpEncoders[i].accel = 1;
        mcpEncoders[i].msSinceLastRead = millis();
        mcpEncoders[i].previousValue = 0; // Initialize previous value
        mcpEncoders[i].lastDebounceTime = 0; // Initialize debounce timer
    }
}

int MCP23017Encoders::read(int encoder)
{
    if (encoder < 0 || encoder >= MCP_ENCODERS) return 0;

    unsigned long currentTime = millis();
    if (currentTime - mcpEncoders[encoder].lastDebounceTime > 50) // 50ms debounce delay
    {
        float w = abs(mcpEncoders[encoder].tickValue) / (currentTime - mcpEncoders[encoder].msSinceLastRead);

        if (w > 11) 
            mcpEncoders[encoder].currentValue += mcpEncoders[encoder].tickValue * mcpEncoders[encoder].accel;
        else  
            mcpEncoders[encoder].currentValue += mcpEncoders[encoder].tickValue;

        mcpEncoders[encoder].tickValue = 0;
        mcpEncoders[encoder].msSinceLastRead = currentTime;
        mcpEncoders[encoder].lastDebounceTime = currentTime;
    }

    if (mcpEncoders[encoder].currentValue != mcpEncoders[encoder].previousValue) // Check if value has changed
    {
        mcpEncoders[encoder].previousValue = mcpEncoders[encoder].currentValue; // Update previous value
        return mcpEncoders[encoder].currentValue;
    }

    return mcpEncoders[encoder].previousValue;
}

void MCP23017Encoders::write(int encoder, int value)
{
    if (encoder < 0 || encoder >= MCP_ENCODERS) return;

    mcpEncoders[encoder].currentValue = value;
    mcpEncoders[encoder].previousValue = value; // Update previous value when writing
}

void MCP23017Encoders::setAccel(int encoder, float value)
{
    if (encoder < 0 || encoder >= MCP_ENCODERS) return;

    mcpEncoders[encoder].accel = value;
}

/*
  Interrupt service
*/
static void MCP23017EncoderISR(void)
{
    uint8_t p;
    uint16_t v;

    noInterrupts();
    detachInterrupt(digitalPinToInterrupt(_MCP23017intPin));
    interrupts();

    p = _mcp.getLastInterruptPin();
    v = _mcp.readGPIOAB();

    for (int i = 0; i < 8; i++)
    {
        uint16_t mask = (0x03 << (i * 2));
        uint16_t currentBits = (v & mask) >> (i * 2);
        currentBits |= mcpEncoders[i].previousBitMask << 2;

        switch (currentBits)
        {
            case 0: case 5: case 10: case 15:
                break;

            case 1: case 7: case 8: case 14:
                mcpEncoders[i].tickValue++;
                break;

            case 2: case 4: case 11: case 13:
                mcpEncoders[i].tickValue--;
                break;
        }

        mcpEncoders[i].previousBitMask = v & mask;
    }

    EIFR = 0x01;
    attachInterrupt(digitalPinToInterrupt(_MCP23017intPin), MCP23017EncoderISR, FALLING);
}
