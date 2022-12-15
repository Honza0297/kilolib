#include "kilolib.h"
#define DEBUG
#include "debug.h"

// Constants for light following.
#define THRESH_LO 450
#define THRESH_HI 600
#define NUM_SAMPLES 10


// Constants for motion handling function.
#define STOP 0
#define FORWARD 1
#define LEFT 2
#define RIGHT 3
// Function to sample light.
int sample_light()
{
    // The ambient light sensor gives noisy readings. To mitigate this,
    // we take the average of 300 samples in quick succession.
    
    int samples_ok = 0;
    int sum = 0;

    while (samples_ok < NUM_SAMPLES)
    {
        int sample = get_ambientlight();
        
        // -1 indicates a failed sample, which should be discarded.
        if (sample != -1)
        {
            sum += sample;
            samples_ok++;
        }
        delay(5);
    }

    // Compute the average.
    return (int) ((float)sum)/NUM_SAMPLES;
}

void set_motion(int new_motion)
{
    static current_motion = STOP;
    // Only take an action if the motion is being changed.
    if (current_motion != new_motion)
    {
        current_motion = new_motion;
        
        if (current_motion == STOP)
        {
            set_motors(0, 0);
        }
        else if (current_motion == FORWARD)
        {
            spinup_motors();
            set_motors(kilo_straight_left, kilo_straight_right);
        }
        else if (current_motion == LEFT)
        {
            spinup_motors();
            set_motors(kilo_turn_left, 0);
        }
        else if (current_motion == RIGHT)
        {
            spinup_motors();
            set_motors(0, kilo_turn_right);
        }
    }
}

void setup()
{
    // This ensures that the robot starts moving.
    //set_motion(LEFT);
    set_color(RGB(1,1,1));
    delay(300);
    set_color(RGB(0,0,0));
    delay(300);
    set_color(RGB(1,1,1));
    delay(300);
    set_color(RGB(0,0,0));
    delay(300);
    
}

void loop()
{
    int current_light = sample_light();
    #ifdef DEBUG
    printf("%d\n",current_light);
    #endif 
    if (current_light == -1)
    {
        set_color(RGB(0,0,0));
    }
    else if (current_light >= 0 && current_light < 340)
    {
        set_color(RGB(1,0,0));
    }
     else if (current_light >= 341 && current_light < 690)
    {
        set_color(RGB(0,1,0));
    }
     else if (current_light >= 691)
    {
        set_color(RGB(0,0,1));
    }
   
}

int main()
{
    kilo_init();
    #ifdef DEBUG
    debug_init();
    #endif
    kilo_start(setup, loop);

    return 0;
}
