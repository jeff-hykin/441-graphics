name: Jeff Hykin

Run time/controls:
    - I spent extra time to enable the x,y,z buttons to be held down to smoothly rotate objects (instead of having to tap over and over again)
    - Press shift to reverse the direction of the rotation
    - Use the arrow keys to translate the x and z axis

Code layout:
    - Where is the modular/generalized/abstract code?
        classes.h
    - Wheres the custom implemention-specific code?
        int main()
        
    I bundled up the shaders and many global variables to make the code easier to manage.
    The KeyMapperClass was created to allow dynamic/runtime binding and unbinding of keys and to allow the hold-down functionality;