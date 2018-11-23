
# Navputter
#### Firmware for a USB keymouse + virtual serial nmea. breakout designed for OpenCPN.



         .-'           Watch out for me!
    '--./ /     _.---.
    '-,  (__..-`       \
       \          .     |
        `,.__.   ,__.--/
          '._/_.'___.-`
            /  /
           :__/

Navputter is firmware that works with USB compatible micro controllers plugged into  Windows, Mac, and Linux without additional drivers. Pug in the USB and it works as a mouse, a keyboare, and a serial device. The serial can be used to parse nmea sentences and drive custom digital i/o or to program the eeprom keymappings. Tis project is just an easy shell to implement simple opencpn hardware idea.

### Hardware
Hardware: this project is currently set up for specific hardware. For easiest results start with identical harware. The only required hardware is a USB compatibe uC and enbough free pins to support the keypad or buttons. Currenty I have: 

   uC: Teensy++ 2.0. This is a at90usb1286.
   4x4 keypad ( multiplexed 8 pin type )
   simple radial encoder 
   simple push buttons (2)
   LEDs (2)
  
### port mappings:
 
    Navputter currently has a 4x4 keypad ( PORTF ), 
    An uTFT_ST7735 display ( PORTC 0-3 )
    A radial encoder ( PIND0,1 )
    misc buttons ( PIND2, 3 )
    2 LEDS ( PIND4, PIND5 )

Everything is configured in navputter.h, except for the display which is configured inside uTFT_ST7735.h 


### Serial Interface:

 Navputter creates a virtual serial interface as well. You can enter commands
 for configuring the eeprom or send NMEA sequences and have them parsed.

### Command Interface:

To use the command interface connect to the virtual serial device with a teriminal program such as minicom. Ubuntu e.g.: minicom -D /dev/ttyACM0

#### Mouse moves are specified with the chars:
    'U' - mouse move up
    'D' - mouse move down
    'L' - mouse move left
    'R' - mouse move right 
    'l' - mouse left click
    'r' - mouse right click

#### The following siple serial commands are supported:

  help  - show some help
  dump  - dump out all the eeprom data

  maps  - set a slow key mapping. Example: 
  
    maps 1 1 5  
  This sets row 1, col 1 to key sequence 5. You can also set mouse moves instead such as:
  
    maps 1 1 U

 mapf  - sets a fast key mapping. Same syntax as maps above:
 
    mapf 1 1 U

 mapm -  set a mouse mapping. Same syntax as mapf above:
 
    mapm 1 1 U

 seq - set a key sequence. Each key sequence can have up to 4 key presses, each keypress can have any number of key modifiers such as alt, shift, etc. All keys need to be entered numerically. Refer to keycodes.txt for the values for each key and scancode. For example:
 
       seq 1 0x55 0, 0x56 0
 
 The above example sets key sequence 1 to be asterix then minus: '*-'


  save - this command save the current running key config to eeprom. Be 
   careful to test your key mapping before saving.

#### NMEA via serial

This code is there just to aid future expansion. I am not doing anyting with the NMEA except parsing it and displaying some of the data on the display.********



#### Building:

Navputter is currently set up to build as a demo of LUFA. To build navputter first get and build lufa:

    git clone https://github.com/abcminiuser/lufa lufa
    cd lufa
    make

Now pull Navputter into the correct location:

    cd Demos/Device/ClassDriver
    git clone https://github.com/capnkeith/navputter navputter
    cd navputter
    make
    
Now program your AVR

    make avrdude
 
 
#### Debugging:

if DEBUG is defined you can use the dbgprint which is printf for the lcd, or your can fprintf( gfp, ... ) to write out th serial adapter if you are debugging nmea you will be glad for the lcd. 
 
#### Key Mappings:

Navputter is fully eeprom customizable, but the first time after a fresh 
program, or if you write 0xff to eeprom location 0, the eeprom will be reset
to factory default which corresponds rougly to this function map below

   
    toggle      |   up      | anchor    |routstart
    left        |markboat   | right     |routend 
    zim         | down      | zout      |routenext 
    daynight    |track      |follow     |drop cursor
  
  slow key set maps like this:
  
    INT        | alt+up    | A         |crrl+r 
    alt+lt     | ctrl+o    | alt+rt    | esc 
    alt+ +     | alt+down  | alt + -   | ctrl+n
    f5         |  ?        | F2        | ctrl+m     
  
  fast key set maps like this:

 
    INT         | up        | A         |  ctrl+r  
    lt          | ctrl+o    | rt        | esc   
     +          | down      | -         | ctrl+n  
    f5          |  ?        |F2         | ctrl+m     
  
mouse map is like this:

     INT       | mouse up  | A         | ctrl_r
     mouseleft | ctrl+o    |mousert    | esc 
     alt+ +    | mouse down| alt + -   | ctrl+n
     f5        | ?         | f2        | ctrl_+m
 
