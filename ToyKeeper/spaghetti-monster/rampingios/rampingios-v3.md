RampingIOS V3 Manual

This Markdown-formatted manual was contributed by phil_g under a 
Creative Commons CC0 waiver:
  http://aperiodic.net/phil/archives/Geekery/rampingios-v3.html
  https://creativecommons.org/publicdomain/zero/1.0/


<figure style="float: right">
  <a href="https://bazaar.launchpad.net/~toykeeper/flashlight-firmware/trunk/download/head:/rampingiosv3ui.png-20180807025443-zdamv4ixtu49o7hm-1/rampingiosv3-ui.png">
    <!-- img width="256em" src="https://bazaar.launchpad.net/~toykeeper/flashlight-firmware/trunk/download/head:/rampingiosv3.svg-20180807025420-q28902kbav01123w-1/rampingiosv3.svg" -->
    <img width="256em" src="https://bazaar.launchpad.net/~toykeeper/flashlight-firmware/trunk/download/head:/rampingiosv3ui.png-20180807025443-zdamv4ixtu49o7hm-1/rampingiosv3-ui.png">
  </a>
  <figcaption>RampingIOS V3 UI diagram</figcaption>
</figure>
    
The Emisar [D4S][emisar-d4s] flashlights use a firmware named RampingIOS
V3.  (The Emisar [D4][emisar-d4], [D1][emisar-d1], and [D1S][emisar-d1s]
all use [RampingIOS V2][rampingios-v2].)  There's not really a manual; the
only thing we get is the diagram on the right.  It's reasonably
comprehensive, but there's a fair amount of detail it merely summarizes,
so I thought a textual manual would be nice.

  [emisar-d4]: https://intl-outdoor.com/emisar-d4-high-power-led-flashlight-p-921.html
  [emisar-d1]: https://intl-outdoor.com/emisar-d1-mini-thrower-p-922.html
  [emisar-d1s]: https://intl-outdoor.com/emisar-d1s-thrower-p-926.html
  [emisar-d4s]: https://intl-outdoor.com/emisar-d4s-26650-high-power-led-flashlight-p-932.html
  [rampingios-v2]: http://aperiodic.net/phil/archives/Geekery/rampingios-v2.html

The Emisar D4S only works when the head and tailcap are tightened fully.
You can physically lock it out--prevent it from turning on
accidentally--by simply loosening the tailcap a small amount.  A quarter
turn will do it.

Emisar lights are known for their ramping interfaces.  Rather than have a
small number of distinct brightness levels, they can vary their brightness
anywhere between their lowest and highest levels, like a light on a
dimmer.  The D4S is in ramping mode by default, but it also has a stepped
mode that can be configured to be closer to how non-ramping lights work.

Each mode--ramping and stepped--can have differently-configured brightness
floors and ceilings.

The driver for the D4S has two different chipsets.  At low brightness
levels, a fairly-efficient but low-power chipset (called a *7135*) is
used.  These lowest brightness levels are called the "*regulated levels*".
Each regulated level will always be the same brightness regardless of how
much charge the battery has.  Above a particular brightness level, the
light switches over to a less-efficient but high-power chipset (called a
*FET*).  These levels are called "*direct-drive*".  The brightness of the
direct-drive levels is directly related to the battery's charge level; the
more charged the battery, the brighter the levels.  The light is at its
most efficient, in terms of power used for every lumen generated, at the
brightest regulated level.  When the light is first powered by tightening
the tailcap, it will default to this level.

At higher brightness levels, the light's LEDs generate a lot of heat.  If
the light exceeds its configured maximum temperature, it will begin
dimming itself automatically until the temperature drops below the allowed
maximum.

The D4S has a set of cyan-colored auxiliary LEDs that can be on when the
main LEDs are off.  You can configure the behavior of the aux LEDs.

#### Basic Usage

The default mode for the light is ramping mode.  Triple-pressing the
button (**3 clicks**) while the light is on will toggle between ramping
and stepped mode.

While the light is off, press and release the button (**1 click**) to turn
it on.  It will turn on at the last-used brightness level.  (This is
called "*mode memory*".)  Immediately after loosening and tightening the
tailcap (or after changing the battery), the memorized level will be the
light's max regulated level.

When the light is on, 1 click will turn it off.  The current brightness
level will be memorized for future use.  There's a fraction of a second
delay between pressing the button and the light actually turning off.
That's because of the way the light processes input; it's waiting to make
sure you're only going to press the button once (since multiple presses
will trigger other actions).

When the light is on, holding the button down will brighten the light.  In
ramping mode, the brightness will increase gradually ("*ramping up*").  In
stepped mode, the light will jump through increasing brightness levels.
If you press, release, and then hold the button, it will begin dimming.
In ramping mode, the brightness will decrease gradually ("*ramping
down*").  In stepped mode, the light will jump through decreasing
brightness levels.  While the light is changing, if you release the button
and immediately hold it again, the direction (dimming or brightening) will
switch.

In ramping mode, while the light is ramping, it'll briefly blink off and
on again at two different brightness levels: the maximum regulated level
and the brightness ceiling.

While the light is off, double-pressing the button (**2 clicks**) will
immediately jump to the brightness ceiling.

While the light is on, **2 clicks** will jump to the maximum brightness
level, regardless of the configured brightness ceiling.  Another two
clicks will go back to the previous brightness level.

While the light is off, if you hold the button the light will turn on at
its lowest level.  If you continue holding the button, the light will
begin brightening from there.

##### Configuration Menus

The light has several different configuration modes.  Each of those modes
works more or less the same way.  The mode will have a series of menu
items that it will go through.  For each menu item, the light will first
blink a number of times corresponding to the item number (first, second,
etc.)  After that, the light will begin fluttering on and off fairly
quickly.  While the light is fluttering, you can click the button a number
of times; the light will count the number of button presses and use that
number as its new configuration for that menu item.  After a short period
of time, the fluttering will stop and the light will move on to the next
menu item.  After the light has gone through all of the menu items, it
will return to whatever mode it was in before entering the configuration
mode.

If you don't press the button during a particular menu item's fluttering,
that item will remain unchanged.

##### Configuring the Basic Modes

While the light is on, **4 clicks** will enter ramping or stepped
configuration mode, depending on which mode the light was in before the 4
clicks.

For ramping mode, there are two menu options:

 1. Brightness floor (default 1/150)
 2. Brightness ceiling (default 150/150)

During the floor configuration, press the button equal to the number of
ramping levels (out of 150) at which the floor should be.  To set the
lowest possible floor, click the button once.

The ceiling is configured similarly, but you press the button equal to the
number of steps away from maximum brightness.  To set the highest possible
ceiling (at max brightness), click the button once.

For stepped mode, there are three menu options:

 1. Brightness floor (default 20/150)
 2. Brightness ceiling (default 120/150)
 3. Number of steps (default 7)

#### Other Modes

The other modes largely involve multiple clicks from off.  Most of them
are not generally needed for everyday use, but they supplement the light's
basic operations.

##### BattCheck/TempCheck Modes

From off, **3 clicks** will enter "BattCheck" mode, which blinks out the
current battery voltage.  First it blinks the number of volts, then it
pauses, then it blinks out the tenths of volts.  Thus, if the battery were
at 3.5 volts, the light would blink three times, pause, then five times.
For zeroes, it gives a very short blink.

A fully-charged lithium-ion battery is 4.2 volts.  The light considers 2.8
volts to be an empty battery and won't turn on if the battery is at or
below 2.8 volts.

The voltage sequence will continue blinking until you turn off the light
with a single click.

While the light is in BattCheck mode, **2 clicks** will enter TempCheck
mode.  Instead of blinking out the battery voltage, the light will start
blinking out its current temperature in degrees Celsius, first the tens
digit then the units digit.  Like BattCheck mode, the light will continue
blinking out the temperature until you turn it off with a single click.

While the light is in TempCheck mode, **4 clicks** will enter thermal
configuration mode.  See the thermal configuration mode documentation
below for how that works.

##### Tactical Mode

From off, **4 clicks** will enter "tactical" or "momentary" mode.  The
light will flash once to show that it's entered the mode.  The auxiliary
LEDs will turn off (if they were on).  In tactical mode, the light will
turn on at its memorized brightness for as long as the button is being
held down.  It will turn off as soon as the button is released.

There's no button press combination that will exit tactical mode.  To exit
it, you will have to partially unscrew and retighten the tailcap.

##### Lockout Mode

From off, **6 clicks** will enter lockout mode.  The light will flash
twice to show that it's entered the mode.  There's a separate aux LED mode
for lockout mode, so you can tell whether the light is in lockout or not.

In lockout mode, pressing the button will turn on the light at its lowest
brightness ("*moonlight mode*") for as long as the button is held down.

Another 6 clicks will exit lockout mode.  The light will flash twice to
show that it's left the mode.

While in lockout mode, **3 clicks** will cycle through the various
settings for the aux LEDs in lockout mode.  The four modes are, in order:
low, high, blink (on high), and off.  The default mode is blink.

Remember that loosening the tailcap a quarter turn will also lock out the
light.  Using the 6 clicks is called "*electronic lockout*", while turning
the tailcap is "*physical lockout*".

##### Aux LED Configuration

From off, **7 clicks** will cycle to the next aux LED mode.  The four
modes are, in order:  low, high, blink (on high), and off.  The default
mode is low.

##### Beacon Mode

From off, **8 clicks** will enter beacon mode.  In beacon mode, the light
will blink on and off every few seconds.

By default, the light will blink every two seconds.  To change the timing,
use **4 clicks** while in beacon mode.  The light will enter a one-item
menu.  During the flickering for input, press the button a number of times
equal to the number of seconds between blinks.

1 click will exit beacon mode.

##### Thermal Configuration Mode

From off, **10 clicks** will enter thermal configuration mode.

The menu items here are:

 1. Current temperature (every click is one degree Celsius)
 2. Temperature ceiling (every click is one degree *above 30??C*)
 
The "current temperature" item can be used to adjust the calibration of
the light's temperature sensor.  To use it, make sure the light has been
off long enough that all of its components have cooled (or warmed) to the
ambient temperature.  Check the ambient temperature using a thermometer
you trust.  Go to thermal configuration mode, and enter the current
temperature by clicking the button a number of times equal to the
temperature in degrees Celsius.  (If it's 22??C, click the button 22
times.)

You can check the default calibration by entering TempCheck mode from a
room-temperature light.  The D4Ss are supposed to go through a temperature
calibration at the factory, so hopefully most of them won't need manual
thermal calibration.

The temperature ceiling is simply the highest temperature the light should
be allowed to reach.  Once it hits its temperature ceiling, it will
progressively dim itself until the temperature stabilizes below the
ceiling.  Note that the number of clicks in that menu option is added to
*30* to reach the actual ceiling.  (Thus, you can't set a ceiling below
31??C.)  The maximum allowed ceiling is 70??C.

The default temperature ceiling is 45??C.
