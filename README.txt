******************************************************************************
* Navputer - like a putter for golf except for your sailboat.
******************************************************************************
*
* This work contains open source 3rd party libraries such as LUFA, minmea,
* and uTFT_ST7735 at least. Each of these has its own licenses, the most 
* restrictive of is LUFA. Please investigagte and respect each licnese 
* appropriately. Also if you like this work, please donate to LUFA instead, 
* LUFA is the real magic here.
*
******************************************************************************
*
* This file is an original work by Seth Keth offered for free with no licese 
* whatsoever or if you prefer you can stick a WTFPL v2 on it. 
*
******************************************************************************
*
*
*         .-'           Watch out for me!
*    '--./ /     _.---.
*    '-,  (__..-`       \
*       \          .     |
*        `,.__.   ,__.--/
*          '._/_.'___.-`
*            /  /
*            __/
*
******************************************************************************
*
* Hardware: this project is currently set up for specific hardware. For 
* easiest results start with identical harware. 
*
* Navputter assembled:
*
*   uC: Teensy++ 2.0. This is a at90usb1286.
*   4x4 keypad ( multiplexed 8 pin type )
*   simple radial encoder 
*   simple push buttons (2)
*   LEDs (2)
*
* For programming: 
*    USBASP programmer
*
******************************************************************************
*
* Wiring: use pullup resistors on the radial encoder/buttons, pull down 
* resistors on the keypad rows/cols. The LEDs are connected with annode to
* pin and cathode to GND. I like using the internal pull ups in the uC but
* I could not get this working for some reason. Possibly try it again.
*
******************************************************************************
* 
* port mappings:
* 
* Navputter currently has a 4x4 keypad ( PORTF ), 
* An uTFT_ST7735 display ( PORTC 0-3 )
* A radial encoder ( PIND0,1 )
* misc buttons ( PIND2, 3 )
* 2 LEDS ( PIND4, PIND5 )
*
* Everything is configured in navputter.h, except for the display which is
* configured inside uTFT_ST7735.h 
*
******************************************************************************
*
* Navputter operation:
*
* Navputter has 3 modes, selectable with the upper left button (default config). 
* These modes are 'slow key mode', 'fast key mode', and 'mouse mode'. If you
* have the LEDs connected they will light left, right, or both respectivley.
*
* In the 3 modes buttons are mapped to key sequences or mouse moves in the
* different maps. key_map => slow mode, key_map2 => fast mode, mouse_map =>
* mouse mode. 
*
*
******************************************************************************
*
* Serial Interface:
*
* Navputter creates a virtual serial interface as well. You can enter commands
* for configuring the eeprom or send NMEA sequences and have them parsed.
*
******************************************************************************
*
* Command Interfsce:
*
* To use the command interface connect to the virtual serial device with a
* teriminal program such as minicom. Ubuntu e.g.: minicom -D /dev/ttyACM0
*
*
* Mouse moves are specified with the chars:
*    'U' - mouse move up
*    'D' - mouse move down
*    'L' - mouse move left
*    'R' - mouse move right 
*    'l' - mouse left click
*    'r' - mouse right click
*
* The following siple commands are supported:
*
* help  - show some help
* dump  - dump out all the eeprom data
*
* maps  - set a slow key mapping. Example: 
*    maps 1 1 5  
* This sets row 1, col 1 to key sequence 5. You can also set mouse moves 
* instead such as:
*    maps 1 1 U
*
* mapf  - sets a fast key mapping. Same syntax as maps above:
*    mapf 1 1 U
*
* mapm -  set a mouse mapping. Same syntax as mapf above:
*    mapm 1 1 U
*
* seq - set a key sequence. Each key sequence can have up to 4 key presses,
*   each keypress can have any number of key modifiers such as alt, shift, etc.
*   All keys need to be entered numerically. Refer to keymap.txt for the values
*   for each key and scancode.
*   Example:
*       seq 1 0x55 0, 0x56 0
*   The above example sets key sequence 1 to be asterix then minus: '*-'
*
* save - this command save the current running key config to eeprom. Be 
*   careful to test your key mapping before saving.
*
******************************************************************************
*
* NMEA via serial
*
* This code is there just to aid future expansion. I am not doing anyting 
* with the NMEA except parsing it and displaying some of the data on the
* display. For instance if you send
*
******************************************************************************
*
* Building:
*
* Navputter is currently set up to build as a demo of LUFA. To build navputter
* first get and build lufa:
*
*    git clone https://github.com/abcminiuser/lufa lufa
*    cd lufa
*    make
*
* Now pull Navputter into the correct location:
*
*    cd Demos/Device/ClassDriver
*    git clone https://github.com/capnkeith/navputter navputter
*    cd navputter
*    make
*
* Now program your AVR
*
*    make avrdude
*
* 
* 
******************************************************************************
* 
* Debugging:
*   if DEBUG is defined you can use the dbgprint which is printf for 
*   the lcd, or your can fprintf( gfp, ... ) to write out th serial adapter
*   if you are debugging nmea you will be glad for the lcd. I think it is 
*   possible to use an actual debugger somehow...
* 
******************************************************************************
TODO:

* Currently key repeat is not implemented. 

******************************************************************************
*

Key Mappings:

Navputter is fully eeprom customizable, but the first time after a fresh 
program, or if you write 0xff to eeprom location 0, the eeprom will be reset
to factory default which corresponds rougly to this function map below

   ---------------------------------------------------
    toggle      |   up      | anchor    |routstart
   ---------------------------------------------------
    left        |markboat   | right     |routend 
   ---------------------------------------------------
    zim         | down      | zout      |routenext 
   ---------------------------------------------------
    daynight    |track      |follow     |drop cursor
   ---------------------------------------------------

  flow key set maps like this:
  
   ---------------------------------------------------
   |INT        | alt+up    | A         |crrl+r 
   ---------------------------------------------------
   |alt+lt     | ctrl+o    | alt+rt    | esc 
   ---------------------------------------------------
   |alt+ +     | alt+down  | alt + -   | ctrl+n
   ---------------------------------------------------
   | f5        |  ?        | F2        | ctrl+m     
   ---------------------------------------------------

  fast key set maps like this:


   ---------------------------------------------------
   |INT         | up        | A         |  ctrl+r 
   ---------------------------------------------------
   |lt          | ctrl+o    | rt        | esc  
   ---------------------------------------------------
   |  +         | down      | -         | ctrl+n  
   ---------------------------------------------------
   | f5         |  ?        |F2         | ctrl+m     
   ---------------------------------------------------

mouse map is like this:

   ---------------------------------------------------
   |INT         | mouse up  | A         | ctrl_r
   ---------------------------------------------------
   |mouseleft   | ctrl+o    |mousert    | esc 
   ---------------------------------------------------
   |  alt+ +    | mouse down| alt + -   | ctrl+n 
   ---------------------------------------------------
   | f5         | ?         | f2        | ctrl_+m
   ---------------------------------------------------

