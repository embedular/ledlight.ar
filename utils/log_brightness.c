#include <stdio.h>
#include <math.h>


// https://diarmuid.ie/blog/pwm-exponential-led-fading-on-arduino-or-other-platforms
int main()
{
    // The number of Steps between the output being on and off
    const int pwmIntervals = 2048;
    
    // The R value in the graph equation
    double R;
    
    // Calculate the R variable (only needs to be done once at setup)
    R = (pwmIntervals * log10(2))/(log10(256));
    
    int brightness = 0;
    
    printf ("#include <stdint.h>\n\n");
    printf ("#define LOG_BRIGHTNESS_COUNT\t\t%iU\n\n", pwmIntervals);

    printf ("static const uint8_t s_logBrightness[LOG_BRIGHTNESS_COUNT] =\n");
    printf ("{\n");
    
    for (int interval = 0; interval < pwmIntervals; interval++)
    {
        // Calculate the required PWM value for this interval step
        brightness = pow (2, (interval / R)) - 0.1;
        // Set the LED output to the calculated brightness
        printf ("\t[%i] = %i,\n", interval, brightness);
    }
    
    printf ("}");

    return 0;
}
