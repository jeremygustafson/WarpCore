# ST:TNG Warp Core with RGB LEDs, Web GUI, Sound Effects, and Bluetooth

Source and instructions for [ElmoC's ST:TNG Warp Core](https://www.thingiverse.com/thing:1656741), with RGB LEDs, web-based control, sound effects, and bluetooth, as devised by [Boogle's make](https://www.thingiverse.com/make:589779).

Code and Printed Circuit Board (PCB) design by [Dan Clarke](https://github.com/danclarke). Instructions (and a sightly revised PCB) by [Jeremy Gustafson](https://github.com/jeremygustafson).

## Credit where credit is due
ElmoC designed the Warp Core Thing based on the [Warp Core Table Lamp created by ThePlanetMike](https://www.thingiverse.com/thing:327114).

Boogle a.k.a. Dan Clarke posted a [make](https://www.thingiverse.com/make:589779) that expanded on ElmoC's design with RGB LEDs and a web-based GUI, as well as sound effects and the ability to connect to a Bluetooth speaker. However, in his comments he noted could only offer the raw source code and PCB files, without any documentation. When I asked, Dan graciously shared his source and PCB with me, and he went far above and beyond in answering innumerable questions I sent his way while working on this project. So, to be clear: the whole electronics design and code belongs to Dan, I'm just the one documenting the process so others can also build their own RGB Warp Core.

And finally, I owe a lot of thanks to my Dad, who went to school for electrical engineering and has all the right tools and various "stuff", and who let me take over his soldering workstation for almost a week.


## Warnings

At the risk of sounding like a middle school teacher, I highly recommend reading through all the directions before starting.

**Warning:** This is an advanced project. You will need at least a rudimentary understanding of electronics and the command-line (whether you're running on Mac, Windows, or Linux). This build involves surface-mount soldering, a complex installation process of the ESP IDF, and potentially a lot of troubleshooting of the ESP code at compilation time. Dan mentioned that he needed to perform "unspeakable hackery to make it function," and that was also my experience. There is no GUI for any of this. I don't say that to scare anyone away, just to set expectations that you might/will spend a lot of time banging your head against a wall. But it's worth it.

In Dan's words: "My warp core is a bit over-complicated because I wanted it to be an appliance you could just plug in and use, then unplug when done. So it uses 2(!!) ESP32s to overcome some wireless limitations and not suffer from any corruption issues like a Raspberry Pi would if you just yanked the power." 

Dan also warns that the warp core uses "a fair bit of current so things can get warmer." He cautions that using too thin a wire or having a poor connection could result in a fire. So, use caution. Dan used 22 gauge solid-core wire, I used a combination of 22 gauge stranded and 20 gauge solid-core. (More details on that later in this README).

Even if you take advantage of JLCPCB's assembly service for most of the PCB components, you will still need to do some component soldering yourself. It's worth having all the right tools, they will make your life easier.

Last preface: I came into both this project and my previous Stargate build as an electronics novice, and so I've tried very hard to make these directions as complete as possible to make it accessible to others like me with almost no expertise. But there might be things I missed or forgot about. If you find a mistake or something is missing, please let me know so I can try to fix it!


## Purchasing Requirements (aka Shopping List)

You will need components from JLCPCB, LCSC, DigiKey, and Amazon (or another retailer). The following list does not include tools such as soldering iron, electric tape, or other miscellaneous supplies.

ElmoC includes a fantastic instructions PDF for assembling your warp core, and you should follow that for the majority of your build. However the following list **replaces** all electronics-y parts from ElmoC's instructions. You will still need the physical hardware components he lists, such as screws, nuts, washer, and 1/4" threaded rod.


**From JLCPCB**
- 1x custom Printed Circuit Board ("PCB"), described in immense detail below


**From LCSC**
- 2x ESP32-WROOM-32D [C473012](https://lcsc.com/product-detail/RF-Transceiver-ICs_Espressif-Systems-ESP32-WROOM-32D_C473012.html) (I purchased the 16MB versions in case I or someone else ever expands upon the code to include more features; however, in this project's current state the 4MB ESP32's will work just fine)
- 5x 470uF/16V capacitors [C42238](https://lcsc.com/product-detail/Solid-Polymer-Electrolytic-Capacitor_Honor-Elec-HS1C477M0811PC_C42238.html)
- Optional: 2x Header-Female-2.54_1x5 [C50950](https://lcsc.com/product-detail/Pin-Header-Female-Header_BOOMELE-Boom-Precision-Elec-C50950_C50950.html) (alternatively, solder the USB boards' pins directly to the PCB)
- Optional: 1x Header-Female-2.54_1x7 [C124418](https://lcsc.com/product-detail/Female-Header_Shenzhen-Cancome-Female-header-1-7P-2-54mm-Straight-line_C124418.html) (alternatively, solder the audio board's pins directly to the PCB)

If you use JLCPCB's SMT assembly service (described later), they will solder the following parts to the board for you, so you don't need to order them separately. However, if JLCPCB is out of stock for a part, you'll need to order that part from LCSC and solder yourself.

- 2x Texas Instruments TLV1117-33IDCYR LDO [C37359](https://lcsc.com/product-detail/Dropout-Regulators-LDO_Texas-Instruments-TLV1117-33IDCYR_C37359.html)
- 4x 10uF Tantalum Capacitor [C7171](https://lcsc.com/product-detail/Tantalum-Capacitors_AVX-TAJA106K016RNJ_C7171.html)
- 2x FT232RL-REEL [C8690](https://lcsc.com/product-detail/USB-ICs_FTDI-FT232RL-REEL_C8690.html)
- 8x 100nF capacitor [C49678](https://lcsc.com/product-detail/Multilayer-Ceramic-Capacitors-MLCC-SMD-SMT_YAGEO-CC0805KRX7R9BB104_C49678.html)
- 2x 4.7uF capacitor [C1779](https://lcsc.com/product-detail/Multilayer-Ceramic-Capacitors-MLCC-SMD-SMT_Samsung-Electro-Mechanics-CL21A475KAQNNNE_C1779.html)
- 2x Red SMD LED [C84256](https://lcsc.com/product-detail/Light-Emitting-Diodes-LED_Foshan-NationStar-Optoelectronics-NCD0805R1_C84256.html)
- 2x Green SMD LED [C72043](https://lcsc.com/product-detail/Light-Emitting-Diodes-LED_Everlight-Elec-19-217-GHC-YR1S2-3T_C72043.html)
- 4x 1k resistor [C17513](https://lcsc.com/product-detail/Chip-Resistor-Surface-Mount_UNI-ROYAL-Uniroyal-Elec-0805W8F1001T5E_C17513.html)
- 2x SP0503BAHTG SOT [C7074](https://lcsc.com/product-detail/TVS_Littelfuse-SP0503BAHTG_C7074.html)
- 2x UMH3N SOT [C62892](https://lcsc.com/product-detail/Digital-Transistors_Changjiang-Electronics-Tech-CJ-UMH3N_C62892.html)
- 2x 10uF capacitor [C380331](https://lcsc.com/product-detail/Multilayer-Ceramic-Capacitors-MLCC-SMD-SMT_CCTC-TCC0805X5R106K100FT_C380331.html)
- 2x 1uF capacitor [C28323](https://lcsc.com/product-detail/Multilayer-Ceramic-Capacitors-MLCC-SMD-SMT_Samsung-Electro-Mechanics-CL21B105KBFNNNE_C28323.html)
- 13x 10k resistor [C17414](https://lcsc.com/product-detail/Chip-Resistor-Surface-Mount_UNI-ROYAL-Uniroyal-Elec-0805W8F1002T5E_C17414.html)
- 5x 220 resistor [C17557](https://lcsc.com/product-detail/Chip-Resistor-Surface-Mount_UNI-ROYAL-Uniroyal-Elec-0805W8F2200T5E_C17557.html)
- 1x Flash module W25Q128JVSIQTR [C97521](https://lcsc.com/product-detail/FLASH_Winbond-Elec-W25Q128JVSIQ_C97521.html)
- 5x BSS138 SOT [C78284](https://lcsc.com/product-detail/MOSFET_Changjiang-Electronics-Tech-CJ-BSS138_C78284.html)


**From DigiKey**
- 1x [Adafruit Max98357A Class D Amp breakout board](https://www.digikey.com/product-detail/en/adafruit-industries-llc/3006/1528-1696-ND/6058477)
- 2x [Adafruit USB Micro-B breakout board](https://www.digikey.com/products/en?mpart=1833&v=1528)
- 1x [Adafruit 4 Ohms / 3W / 40mm diameter speaker](https://www.digikey.com/products/en?mpart=3968&v=1528)
- 1x [2 Pin Molex connector](https://www.digikey.com/product-detail/en/molex/0022232021/900-0022232021-ND/26667)
- 1x [2 Pin Molex housing](https://www.digikey.com/product-detail/en/molex/0022013027/900-0022013027-ND/26431)
- 5x [3 Pin Molex connector](https://www.digikey.com/product-detail/en/molex/0022232031/WM4201-ND/26669)
- 5x [3 Pin Molex housing](https://www.digikey.com/product-detail/en/molex/0022013037/900-0022013037-ND/26433)
- 17x [Crimp Connectors Female](https://www.digikey.com/product-detail/en/molex/0008500114/WM1114-ND/26475)


**From Amazon**
- 1x [RGB LED 16.4ft light strip](https://smile.amazon.com/gp/product/B01CDTEG1O/)
- 2x USB A-Male to Micro-B cables, I bought this [3-pack](https://smile.amazon.com/gp/product/B072J1BSV6/) so I'd have a spare
- Optional: 1x [20 gauge solid-core wire](https://smile.amazon.com/gp/product/B083DNGSPV/) (see my notes later in this document on how I used different wires)
- Optional: 1x [22 gauge stranded wire](https://smile.amazon.com/gp/product/B00DPQMKBS/) (see my notes later in this document on how I used different wires)
- 1x [5V/10A power supply](https://smile.amazon.com/gp/product/B07Q26YG61/) (you can use a different power supply, but it **must** be 5V; the LEDs draw a maximum of 60 mA each, and with the RGB strip linked above there are 7 LEDs per ring, and in my warp core I had 20 rings plus the 14-LED center ring. 60 x (7 x 20 + 14) = 9.2 Amps to drive all LEDs at full 100% brightness; you'll probably never do this, but best to have a power supply that can handle it just in case you do)
- Optional: 1x [ESP32 Development Board](https://smile.amazon.com/gp/product/B0811LGWY2/), for testing while you wait on your PCB from JLCPCB. Highly recommended.
- Optional: 1x [Arduino Nano with headers](https://smile.amazon.com/LAFVIN-Board-ATmega328P-Micro-Controller-Arduino/dp/B07G99NNXL/), for testing each section of light rings after soldering, prior to final warp core assembly. Highly recommended. (You will also need a breadboard and jumper wires.)


### PCB
You will need to order a custom Printed Circuit Board ("PCB") from JLCPCB or another PCB service.

1. Open this link: https://easyeda.com/editor#id=54b11f66f61e4816807124ef3484aed0 (I don't want to start a religious war, but you must use a supported browser to access EasyEDA; I used Chrome)

2. The active tab should be "Warp Core PCB" and you should see a circuit board in the middle of your window. If you instead see an electronics diagram, look in the upper-left-ish corner of the EasyEDA window and double-click the "Warp Core PCB" item underneath the "Warp Core with USB headers" folder icon.

3. In the top menu bar, click the folder icon with a "G" and an arrow (it's to the right of the "BOM" button); the hover text will say "Generate PCB Fabrication File(Gerber)".

4. When prompted, choose "Yes, Check DRC". There should be no errors. If there are, that means a design rule has changed since I put this together; send me a note and I'll try to fix it.

5. Choose a PCB quantity, the minimum order is 5 boards. You can leave the rest of the settings default. In the lower right, click "Order at JLCPCB"

6. In the JLCPCB order, you can leave everything default and click "Save to Cart". Or, for a few extra monetary units (about $70 when I ordered), you can have JLCPCB do most of the soldering work for you. Turn on "SMT Assembly", choose "Assemble top side," choose your quantity you want assembled, and click Confirm.

6.1. To generate a BOM file, open the "Warp Core PCB" EasyEDA project and click the "BOM" button, then "Export BOM". Upload that file to the JLCPCB prompt.

6.2. To generate a CPL file, open the "Warp Core PCB" EasyEDA project, hover over the file folder icon in the upper left, and choose "Export Pick and Place File...". Upload that file to the JLCPCB prompt.

6.3. JLCPCB won't add the headers, large capacitors, or ESP32s, you'll need to solder those yourself but they are fairly "large" compared to the surface mount stuff.

6.4. I've spent time replacing some of the chips on the PCB with equivalent JLCPCB "basic parts" in order to minimize the "Extended components fee," and I think I got it down as low as I could. Feel free to play with it more, but just know I've already made an effort. When I ordered, I had four parts that were considered "extended parts."

In the electronics list above, I've broken it into separate lists of parts that are/aren't available for JLCPCB to solder, so you know what not to order in case you choose to go this route.

7. After ordering the PCB, you can get discounted shipping for your additional parts from LCSC at checkout, as long as you're signed in on the same account.


## Soldering the PCB

When the PCB arrives, you will need to solder components onto it.

This is a bit of an aside but in the interest of making this project more accessible, I think it's relevant:

Unless you are a skilled solderer (I am not), I **highly** recommend paying the extra $70-or-so to have JLCPCB assemble most of the board for you. The pins on the FT232 in particular are EXTREMELY SMALL, and that also happens to be an expensive part ($3/chip, and there are two of them on the board). Dan told me he went through several FT232s before he was able to solder them correctly. For me, my first attempt on this project was soldering similarly-sized tiny pins for a micro-USB port onto Dan's original PCB design; after a couple days of effort, I'd ruined/ripped/burnt the pads on several PCBs, and then after I thought I'd finally soldered the ports correctly, I plugged it in and cried as I watched the magic smoke escape (fortunately no fire extinguishers were required). My point is: while I can be a bit of a Scrooge when it comes to spending my allowance money, in this case I think it is well worth paying the professionals and saving hours/days of frustration. Or, as my wife put it to me: how much is your time worth?

Assuming you have JLCPCB assemble the boards for you, you will still need to solder on the following components:

- 2x ESP32 - my Dad suggested to me to start in each of the corners to somewhat "lock" the chip in place, before soldering the sides
- 5x 470uF capacitors
- wires leading from PWR, and LEDs 1-5 - use 20 AWG solid core for the 5V lines, and then either the solid core or stranded for the ground and data connections
- pins onto the audio board and 2x USB boards (each board ships with it's own pins, so you don't have to buy those separately)
- either solder headers for the audio and USB boards onto the PCB and then insert the boards by their pins, or solder the USB and audio breakout boards' pins directly into the PCB.

Regarding that last note: on my PCB redesign, it turns out the right-most USB board would physically collide with the audio breakout board, so you will **need** to use headers on at least one of those two. Personally, I used headers for both USB boards and the audio breakout, and then used short jumper wires so the boards could move around more flexibly. Not only did this solve my breakout board collision problem, but also makes me less worried about the USB cables accidentally yanking one of the USB boards off the PCB. And, should I ever want/need to replace one of the boards, it's a lot easier to do so than if they were permanently soldered onto the PCB.

Alternatively, if you are an expert solderer, you can order Dan's original PCB and solder the micro USB ports directly onto it: https://easyeda.com/boogleoogle/Warp_Core-f6d4b07ba5d5411b9c9e35cec9619701 . If you are not an expert solderer, you should not choose this option.


## 3D Printing
Refer to ElmoC's excellent PDF guide for a list of what you need to print, with these few changes:

1. Skip printing Knob.stl, as the USB cords to the PCB will come out that hole instead of having a potentiometer there.

2. Use Dan's LightShelf.stl included in this download, which is designed for the LED light strips, instead of ElmoC's (which is designed for standard blue LEDs)

3. Dan's PCB design includes an additional set of light rings (for a total of four light ring sections, plus the central chamber). If you want a shorter warp core, you can skip this additional section, though you might need to edit the code a little bit. If you choose to keep the fourth light section (as I've done), you will need to 3D print an **additional**:
- LightRing.stl x5
- LightShelf.stl x5
- Upright.stl x9
- LightRingSupportUnit.stl x3
- SupportRingToRingUpper.stl x1
- SupportRingToMI.stl x1


## Soldering and Assembling the LED rings

Similar to ElmoC's guide, you will need to solder together five light rings into one light ring section, and repeat that process to create either three or four light ring sections, depending on your preference.

If you order the same LED strip I linked above, then each light ring will use seven LEDs, and the central chamber will have 14 LEDs. You can use a scissors to cut the LED strip into these lengths.

Each individual LED strip requires 5V power, ground, and a Data In. In each set of five strips, the Data In (DI) comes from the PCB into the first strip, then daisy-chain the Data Out (DO) from the first strip into the Data In of the second strip, and so on. The fifth strip will not need a Data Out connection.

It doesn't matter which end of the LED strip you connect 5V and ground, as power flows both ways. If you accidentally rip off a pad while soldering, you can even connect the 5V or ground to any of the other pads on that same strip. The only connections that **must** be at the start or end of the strips are the DI and DO. If you rip off one of those pads, you will need to cut a new strip for that ring.

Soldering the LED ring light strips was challenging for me, due to the soldered wires frequently ripping the pads off the light strip ends. This was largely caused by my attempt to use the less-flexible solid core wire on all four connections on each strip. At some point I realized this heavier-duty wire wasn't necessary, and switched to using the 22-gauge stranded wires for each of the LED strip connections, which worked much better.

The 5V and G wires I cut 1-2" and then soldered to a central wire for each, for which I used the solid core so it could safely conduct more electricity. I built up this central wire in 1-2" sections as I added each light ring on top of each other. I used 3-4" wires for the DO->DI connections, so in case I needed to pry apart the light rings later on, I'd have some wiggle room. This happened more times than I'd care to admit, after testing an assembled section of five rings and finding out there was a loose connection in the middle somewhere.

I also used electrical tape over each connection to the 5V and G central wires, since my center 1/4" pole was conductive. Finally, I wrapped electrical tape over each end of each light strip prior to adhering the strip onto the light shelf, in order to help hold the wires/solder points in place when I bent the light strip to fit into the shelf's slot.

I've included close-up photos of all these steps on my Thingiverse post.

One final "gotcha" I discovered is that when soldering the central wire connections, heat easily traveled back to the light strip solder pads and loosened those solder connections. That's likely what caused my frequent need to disassemble/reassemble. If you have a metal clip that you can clip onto the wires as a heat sink, that would probably help.


### Testing with an Arduino

I highly recommend testing each completed set of light rings with an Arduino after soldering, but prior to assembling the grey supports and end-caps around the rings. As I mentioned, because I'm an amateur solderer I had to disassemble each set of rings multiple times to re-solder some of the strand-to-strand connections that had come loose. Had I not tested with the Arduino and waited until I'd fully assembled the warp core (or even each section) to power up the lights, this would have been a major bummer.

I recommend reading this guide:
https://learn.adafruit.com/adafruit-neopixel-uberguide/the-magic-of-neopixels

Follow these steps for wiring:
https://learn.adafruit.com/adafruit-neopixel-uberguide/basic-connections

Follow these steps to install the NeoPixel library:
https://learn.adafruit.com/adafruit-neopixel-uberguide/arduino-library-installation

Follow these steps to flash a basic strand test to your Arduino (I used the strandtest_wheel example sketch: File > Examples > AdaFruit NeoPixel > strandtest_wheel)
https://learn.adafruit.com/adafruit-neopixel-uberguide/arduino-library-use

When connecting the strands, always plug in ground first. Typically, I plugged things in in this order, which might have been overly cautious:

- Arduino USB power
- light rings ground wire
- light rings data wire
- light rings 5V wire
- 5V power adapter to breadboard

Only after all my light rings were lit up did I start assembling the grey support pillars around the light ring, plugging in frequently if I thought I might have jostled a wire loose.


## Assembling the 3D model

Follow ElmoC's guide, with the following caveats:

- Run the two USB cables from the PCB down through what would be the potentiometer knob hole in the base. **Be sure to mark which USB cable goes to which ESP32!** (ie, "Lights" and "Audio").

- For the main power wires, I used 20-gauge solid core wires and ran them straight out the power socket hole in the base, and screwed them into to the power adapters's included plug-in connector.


## Brief overview of the ESP projects

This section contains background information only, you do not need to execute any of these commands at this time.

There are two project directories, one for each of the two ESP32 chips. The `warp_core` project is the main code that hosts the web GUI and controls the lighting. The `warp_core_audio` project controls the audio, including Bluetooth connections. Once powered on, the warp core will continuously scan for Bluetooth speakers in discover mode, and connect when it finds one. If the Bluetooth speaker is turned off, the ESP32 will resume playing sounds to the warp core's internal speaker (though the quality here is not great).

Inside each of the two projects is a directory for the ESP's SPIFFS image (SPI Flash File System). These are located at `/components/spiffs_image/image/`. Any time that you modify files inside this directory, you will need to recreate the SPIFFS image (which gets built into `/build/spiffs_image.img`) and then re-flash the SPIFFS image to the ESP32. Both of these steps can be accomplished from a single `make flashfs`, which uses the commands from your project's `/components/spiffs_image/Makefile.projbuild` file. You shouldn't ever need to tweak that file, I'm just mentioning it for reference.

If you modify any program files in `/main` or elsewhere in the project, you will need to re-compile and re-flash the program to the ESP32. There are a few different incantations you might use:

- `make flash` will compile and flash the program.
- `make flash monitor` will compile and flash the program, and then monitor the ESP32's serial console. Useful for debugging issues and when first setting up.
- `make flash monitor -j32` will do the same as above, except the `-j32` option tells the compiler to run 32 processes at once, rather than a single-threaded compile. This will result in a much faster compile time. This is generally the command I will reference in the rest of the README.
- `make monitor` will not compile or flash anything, just connect to the serial console. Useful for debugging.
- `make flashfs monitor` will build and flash the SPIFFS image and then monitor the serial console. Useful for debugging when your only changes are in the SPIFFS image not the primary code.

Occasionally and unpredictably there seemed to be a race condition where compiling with `-j32` would fail the first time, but succeed on a second run. If you don't see an obvious error, I'd suggest just keep running `make flash monitor` or `make flash monitor -j32` until an error becomes obvious.

Lastly, just a note that to flash the PCB's ESP chips, you must have the 5V power plugged in, in addition to the appropriate USB cable (either lights or audio). To flash a stand-alone Amazon boards, power is supplied via USB so no additional power adapter is required.


## Preparing to program the ESP32 chips

Setting up the ESP IDF (Espressif IoT Development Framework) is non-trivial. Once I finally had mine working, Dan's advice to me was, "That's awesome! Now don't change the build environment! It's mega fickle and wants you to fail because it's a monster."

I’ll be the first to admit, my setup (outlined in detail below) feels very kludge-y. There is probably a much better/smoother/easier way of setting this up. But, after many hours spent troubleshooting and Googling, I’ve documented in meticulous detail what worked for me. I've also included a Troubleshooting document alongside this README that has detailed notes about over a dozen specific errors I encountered and how to resolve them.

Note: my setup is on a Mac running macOS 10.12.6. You'll probably have to do some of your own platform-specific troubleshooting while installing the ESP IDF.

Dan's code is based on older versions of both ESP IDF and esp32_digital_led_lib. I haven't taken time to update his code for the newer releases; if someone else wants to give that a try, please fork this project and share your results!

Lastly, you might notice all my file editing commands below use `vi`. If you're not familiar with `vi`, then I'd suggest using `nano` or `pico` instead.


1. Follow these LEGACY instructions for installing ESP-IDF version 3.3.2:
https://docs.espressif.com/projects/esp-idf/en/v3.3.2/get-started/index.html

It’s easy (I think) to miss steps on this page, because it frequently links to directions on other pages. Be sure not to skip the steps under “Setup Toolchain” and “Setup Path to ESP-IDF”.

When setting up the Toolchain, I at first had an issue running `sudo easy_install pip`, but I’m not able to replicate the issue now. It’s possible I had to install pip using [Brew](https://brew.sh).

Under “Install the Required Python Packages”, if your default python is still version 2.x, you will need to explicitly call “python3” instead of just “python” on the command lines. For instance, instead of:

```
python -m pip install --user -r $IDF_PATH/requirements.txt
```

Do:

```
python3 -m pip install --user -r $IDF_PATH/requirements.txt
```

If you’re on a Mac and need to update your Python to v3, see this page for help: https://opensource.com/article/19/5/python-3-default-mac

I recommend attempting the ESP-IDF `hello_world` example, to make sure your build environment is working. This is where I used one of those optional ESP32 boards from Amazon, while I was waiting for my PCB and chips to arrive from China.


2. Copy the downloaded warp core projects to your esp build folder, something like:

```
cp -a ~/Downloads/Warp-Core-master/warp_core ~/Downloads/Warp-Core-master/warp_core_audio ~/esp/
```

3. Download extra dependencies:

```
cd ~/esp/warp_core/components/
git clone --recursive https://github.com/espressif/arduino-esp32.git
cd ~/esp/warp_core/components/arduino-esp32/
git checkout 4d98cea085d619bed7026b37071bd8402a485d95
```
(As of this writing, use the July 18, 2020 revision, commit `4d98cea085d619bed7026b37071bd8402a485d95`)

Remove a troublesome folder that causes compile issues (I don’t understand why, but this works around it!)
```
rm -r ~/esp/warp_core/components/arduino-esp32/libraries/AzureIoT/*
```

Edit this file:
```
vi ~/esp/esp-idf/components/protocomm/include/transports/protocomm_ble.h
(line 17)
// #include <esp_gap_ble_api.h> // KLUDGE: comment out this line
#include "../../../arduino-esp32/tools/sdk/include/bt/esp_gap_ble_api.h" // KLUDGE: add this line
```

Manually add this fix I found from https://github.com/espressif/arduino-esp32/issues/3760 (which itself was linked from https://github.com/espressif/esp32-arduino-lib-builder/issues/10) :
```
vi ~/esp/warp_core/components/arduino-esp32/libraries/WiFi/src/ETH.cpp
(line 196)
    //    ip_addr_t dns_ip = dns_getserver(dns_no); // KLUDGE: comment out this line
    //    return IPAddress(dns_ip.u_addr.ip4.addr); // KLUDGE: comment out this line
    const ip_addr_t * dns_ip = dns_getserver(dns_no); // KLUDGE: add this line
    return IPAddress(dns_ip->u_addr.ip4.addr); // KLUDGE: add this line
```

Make the same change in this file, too:
```
vi ~/esp/warp_core/components/arduino-esp32/libraries/WiFi/src/WiFiSTA.cpp
(line 491)
    //    ip_addr_t dns_ip = dns_getserver(dns_no); // KLUDGE: comment out this line
    //    return IPAddress(dns_ip.u_addr.ip4.addr); // KLUDGE: comment out this line
    const ip_addr_t * dns_ip = dns_getserver(dns_no); // KLUDGE: add this line
    return IPAddress(dns_ip->u_addr.ip4.addr); // KLUDGE: add this line
```

Download source for `mkspiffs`:
```
cd ~/esp/
git clone --recursive https://github.com/igrr/mkspiffs
cd ~/esp/mkspiffs
git checkout 112d65368043f01dd86813cb84e41b1159a34f6b
```
(As of this writing, use the March 7, 2018 revision, commit `112d65368043f01dd86813cb84e41b1159a34f6b`)

Build `mkspiffs` and copy the executable to your `warp_core` and `warp_core_audio` projects:

```
cd ~/esp/mkspiffs
make BUILD_CONFIG_NAME="-arduino-esp32" CPPFLAGS="-DSPIFFS_OBJ_META_LEN=4"
cp -p mkspiffs ~/esp/warp_core/
cp -p mkspiffs ~/esp/warp_core_audio/
```

Finally, copy the hacked/kludged files to the warp_core_audio project, so you don't need to re-download and manually edit them a second time:
```
cp -a ~/esp/warp_core/components/arduino-esp32 ~/esp/warp_core_audio/components/
```

## Compile and flash the warp_core project

First, download two more libraries:
```
cd ~/esp/warp_core/components/arduino-esp32/libraries
git clone --recursive https://github.com/me-no-dev/AsyncTCP
cd ~/esp/warp_core/components/arduino-esp32/libraries/AsyncTCP
git checkout ca8ac5f919d02bea07b474531981ddbfd64de97c
```
(As of this writing, use the October 17, 2019 revision, commit `ca8ac5f919d02bea07b474531981ddbfd64de97c`)

```
cd ~/esp/warp_core/components/arduino-esp32/libraries/
git clone --recursive https://github.com/me-no-dev/ESPAsyncWebServer
cd ~/esp/warp_core/components/arduino-esp32/libraries/ESPAsyncWebServer
git checkout f13685ee97675be2ac9502d177d3024ebc49c1e0
```
(As of this writing, use the October 17, 2019 revision, commit `f13685ee97675be2ac9502d177d3024ebc49c1e0`)


I've modified the warp core's web-based GUI to load local copies of bootstrap and axios, so that a live internet connection isn't required for the warp core interface to work. If for some reason you'd like to change this, just switch these comment lines in `~/esp/warp_core/components/spiffs_image/image/web/index.htm` :

```
<!--<link rel="stylesheet" href="https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css" integrity="sha384-BVYiiSIFeK1dGmJRAkycuHAHRg32OmUcww7on3RYdg4Va+PmSTsz/K68vbdEjh4u" crossorigin="anonymous" />-->
		<link rel="stylesheet" href="css/bootstrap.min.css" />
```
and
```
<!--<script src="https://unpkg.com/axios/dist/axios.min.js"></script>-->
        <script src="js/axios.min.js"></script>
```

Download the aforementioned javascript and css files:
(If necessary, install `wget` first: `brew install wget`)
```
cd ~/esp/warp_core/components/spiffs_image/image/web/js
wget https://unpkg.com/axios@0.19.2/dist/axios.min.js
wget https://unpkg.com/axios@0.19.2/dist/axios.min.map

cd ~/esp/warp_core/components/spiffs_image/image/web/css
wget https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css
wget https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css.map
```

### Set up the warp_core project in ESP IDF
```
cd ~/esp/warp_core
```

Make sure you’ve loaded the ESP environment variables:
```
export PATH=$HOME/esp/xtensa-esp32-elf/bin:$PATH
export IDF_PATH=~/esp/esp-idf
```

Run `make menuconfig`, and arrow down to `Serial flasher config`. Modify the `Default serial port` to your platform specific value, as determined from https://docs.espressif.com/projects/esp-idf/en/v3.3.2/get-started/establish-serial-connection.html

In my case, my Amazon board showed up as `/dev/tty.SLAB_USBtoUART`, but my actual PCB chips showed up as `/dev/tty.usbserial-AR0JGNRF` and `/dev/tty.usbserial-AR0JGNRG`. (you will need to modify this value each time you are flashing a different board; ie, when you flash your Amazon sample board vs when you flash the real PCB)

In the `Serial flasher config` menu, also double check that `Default baud rate` is set to `921600 baud`.


### Customize your network settings

Edit your Config.h and input your local settings:
```
vi ~/esp/warp_core/main/Config.h
#define DEVICE_IP			"192.168.1.35"
#define DEVICE_GW			"192.168.1.1"
#define DEVICE_NETMASK		"255.255.255.0"
#define DEVICE_DNS			"192.168.1.1"
#define DEVICE_DNS2			"8.8.8.8"
#define AP_TARGET_SSID		"X"
#define AP_TARGET_PASSWORD	"X"
```

Note that `AP_TARGET_SSID` and `AP_TARGET_PASSWORD` are your wireless network ID and password.


### Flash the ESP32

As far as I can tell, these must all be done as separate steps:
```
make erase_flash
make partition_table # and then run the command given
make flashfs
make flash monitor -j32
```

If you are prompted with: `Autostart Arduino setup and loop on boot (AUTOSTART_ARDUINO) [N/y/?] (NEW)`, answer the default answers for all the questions.

If you run into compile or flashing errors, consult the Troubleshooting document included alongside this README.

If all goes well, the compile will run for a few minutes and then you should start seeing serial console output from the ESP32. If you've already connected your PCB and light rings, the light rings should start pulsing.

Look for this line in your serial console output:
```
I (2809) wifi:connected with [YOUR SSID HERE]…
```

Probably self-obvious but that means your warp core successfully connected to your wifi. If instead you see:
```
I (2313) WiFi: SYSTEM_EVENT_STA_DISCONNECTED
```

That means your ESP isn’t connected to your wifi. Check your settings in `Config.h` and `make flash monitor -j32` again.

At this point, you should be able to connect to your warp core via a web browser by entering the IP address from your Config.h. As you press buttons on the web interface, you will see the commands logged in the serial console output.

If the web interface doesn't load, look for any errors in the console and consult the Troubleshooting document.


## Compile and flash the warp_core_audio project

### Set up the warp_core_audio project in ESP IDF
```
cd ~/esp/warp_core_audio
```

Make sure you’ve loaded the ESP environment variables:
```
export PATH=$HOME/esp/xtensa-esp32-elf/bin:$PATH
export IDF_PATH=~/esp/esp-idf
```

Run `make menuconfig`, and arrow down to `Serial flasher config`. Modify the `Default serial port`, putting in your audio ESP32's path.

In the `Serial flasher config` menu, also double check that `Default baud rate` is set to `921600 baud`.

### Flash the ESP32

```
make erase_flash
make partition_table # and then run the command given
make flashfs
make flash monitor -j32
```

If for some reason 

If you see any of the following errors, consult the Troubleshooting document:
```
SPIFFS: mount failed, -10025
Wav: File too small (riffLen)
Audio: wavFile is NULL. Error code: 2 : No such file or directory
BT-Audio: initBtAudio enable controller failed: 258,
```


## Congratulations
If you've made it this far, you are a real trooper! This is a complex project and you should feel proud of yourself for completing it!

If you are interested in adding features or attempting to port the code to newer releases of the ESP IDF, have at it, and please share your results!

A few ideas I had for future improvements include:
- holiday-themed lighting effects (for instance, alternating rows of green-red for Christmas, or flag-colors for patriotic holidays)
- setting a hostname and using DHCP instead of a static IP
- fixing what might be a bug where setting a custom color disables the sound effects
- upgrading code to work with the current ESP IDF
- upgrading code to work with the newer esp32_digital_led_lib