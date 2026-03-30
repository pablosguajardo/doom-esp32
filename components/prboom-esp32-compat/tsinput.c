// Touch input completely disabled for now.
// Stub implementation to remove esp_lcd_touch dependency.

#include <stdint.h>

// Called during initialization (now does nothing)
void tsJsInputInit(void)
{
    // Touch disabled
}

// Called by old joystick layer (returns neutral state)
int tsJsInputGet(void)
{
    return 0xFFFF; // No buttons pressed
}
