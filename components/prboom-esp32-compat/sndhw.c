// Sound hardware completely disabled.
// Stub implementation to remove FreeRTOS queue/semaphore usage.

#include <stdint.h>

// Called during sound init
void sndhw_init(int rate, void (*callback)(void))
{
    // Sound disabled
}

// Lock (was using semaphore before)
void sndhw_lock(void)
{
    // Do nothing
}

// Unlock
void sndhw_unlock(void)
{
    // Do nothing
}

// Start playback
void sndhw_start(void)
{
    // Do nothing
}

// Stop playback
void sndhw_stop(void)
{
    // Do nothing
}

// Submit audio buffer
void sndhw_submit(void *data, int len)
{
    // Do nothing
}
