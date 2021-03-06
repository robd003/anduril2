/*
 * This is intended for use on flashlights with a clicky switch.
 * Ideally, a Nichia 219B running at 1900mA in a Convoy S-series host.
 * It's mostly based on JonnyC's STAR on-time firmware and ToyKeeper's
 * tail-light firmware.
 *
 * Original author: JonnyC
 * Modifications: ToyKeeper / Selene Scriven
 *
 * NANJG 105C Diagram
 *           ---
 *         -|   |- VCC
 *  Star 4 -|   |- Voltage ADC
 *  Star 3 -|   |- PWM
 *     GND -|   |- Star 2
 *           ---
 *
 * CPU speed is 4.8Mhz without the 8x divider when low fuse is 0x75
 *
 * define F_CPU 4800000  CPU: 4.8MHz  PWM: 9.4kHz       ####### use low fuse: 0x75  #######
 *                             /8     PWM: 1.176kHz     ####### use low fuse: 0x65  #######
 * define F_CPU 9600000  CPU: 9.6MHz  PWM: 19kHz        ####### use low fuse: 0x7a  #######
 *                             /8     PWM: 2.4kHz       ####### use low fuse: 0x6a  #######
 * 
 * Above PWM speeds are for phase-correct PWM.  This program uses Fast-PWM,
 * which when the CPU is 4.8MHz will be 18.75 kHz
 *
 * FUSES
 *      I use these fuse settings
 *      Low:  0x75
 *      High: 0xff
 *
 * STARS (not used)
 *
 * VOLTAGE
 *      Resistor values for voltage divider (reference BLF-VLD README for more info)
 *      Reference voltage can be anywhere from 1.0 to 1.2, so this cannot be all that accurate
 *
 *           VCC
 *            |
 *           Vd (~.25 v drop from protection diode)
 *            |
 *          1912 (R1 19,100 ohms)
 *            |
 *            |---- PB2 from MCU
 *            |
 *          4701 (R2 4,700 ohms)
 *            |
 *           GND
 *
 *      ADC = ((V_bat - V_diode) * R2   * 255) / ((R1    + R2  ) * V_ref)
 *      125 = ((3.0   - .25    ) * 4700 * 255) / ((19100 + 4700) * 1.1  )
 *      121 = ((2.9   - .25    ) * 4700 * 255) / ((19100 + 4700) * 1.1  )
 *
 *      Well 125 and 121 were too close, so it shut off right after lowering to low mode, so I went with
 *      130 and 120
 *
 *      To find out what value to use, plug in the target voltage (V) to this equation
 *          value = (V * 4700 * 255) / (23800 * 1.1)
 *
 */
#define F_CPU 4800000UL

/*
 * =========================================================================
 * Settings to modify per driver
 */

#define VOLTAGE_MON                 // Comment out to disable

// Lumen measurements used a Nichia 219B at 1900mA in a Convoy S7 host
#define MODE_MOON           6       // 6: 0.14 lm (6 through 9 may be useful levels)
#define MODE_LOW            14      // 14: 7.3 lm
#define MODE_MED            39      // 39: 42 lm
#define MODE_HIGH           120     // 120: 155 lm
#define MODE_HIGHER         255     // 255: 342 lm
// If you change these, you'll probably want to change the "modes" array below
#define SOLID_MODES         5       // How many non-blinky modes will we have?
#define DUAL_BEACON_MODES   5+4     // How many beacon modes will we have (with background light on)?
#define SINGLE_BEACON_MODES 5+4+1   // How many beacon modes will we have (without background light on)?
#define STROBE_MODES        5+4+1+3 // How many strobe modes?
// Note: don't use more than 32 modes, or it will interfere with the mechanism used for mode memory
#define TOTAL_MODES         STROBE_MODES

#define WDT_TIMEOUT         2       // Number of WTD ticks before mode is saved (.5 sec each)

#define ADC_LOW             130     // When do we start ramping
#define ADC_CRIT            120     // When do we shut the light off

/*
 * =========================================================================
 */

#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>
#include <avr/sleep.h>

#define STAR2_PIN   PB0
#define STAR3_PIN   PB4
#define STAR4_PIN   PB3
#define PWM_PIN     PB1
#define VOLTAGE_PIN PB2
#define ADC_CHANNEL 0x01    // MUX 01 corresponds with PB2
#define ADC_DIDR    ADC1D   // Digital input disable bit corresponding with PB2
#define ADC_PRSCL   0x06    // clk/64

#define PWM_LVL     OCR0B   // OCR0B is the output compare register for PB1

/*
 * global variables
 */

// Mode storage
uint8_t eepos = 0;
uint8_t eep[32];
// change to 1 if you want on-time mode memory instead of "short-cycle" memory
#define memory 0
//uint8_t memory = 0;

// Modes (hardcoded to save space)
static uint8_t modes[TOTAL_MODES] = { // high enough to handle all
    MODE_MOON, MODE_LOW, MODE_MED, MODE_HIGH, MODE_HIGHER, // regular solid modes
    MODE_MOON, MODE_LOW, MODE_MED, MODE_HIGH, // dual beacon modes
    MODE_HIGHER, // single beacon modes
    MODE_HIGHER, MODE_HIGHER, // strobe modes
};
volatile uint8_t mode_idx = 0;
// 1 or -1. Do we increase or decrease the idx when moving up to a higher mode?
// Is set by checking stars in the original STAR firmware, but that's removed to save space.
#define mode_dir 1
uint8_t mode_cnt = TOTAL_MODES;

uint8_t lowbatt_cnt = 0;

void store_mode_idx(uint8_t lvl) {  //central method for writing (with wear leveling)
    uint8_t oldpos=eepos;
    eepos=(eepos+1)&31;  //wear leveling, use next cell
    // Write the current mode
    EEARL=eepos;  EEDR=lvl; EECR=32+4; EECR=32+4+2;  //WRITE  //32:write only (no erase)  4:enable  2:go
    while(EECR & 2); //wait for completion
    // Erase the last mode
    EEARL=oldpos;           EECR=16+4; EECR=16+4+2;  //ERASE  //16:erase only (no write)  4:enable  2:go
}
inline void read_mode_idx() {
    eeprom_read_block(&eep, 0, 32);
    while((eep[eepos] == 0xff) && (eepos < 32)) eepos++;
    if (eepos < 32) mode_idx = eep[eepos];
    else eepos=0;
}

inline void next_mode() {
    mode_idx += mode_dir;
    if (mode_idx > (TOTAL_MODES - 1)) {
        // Wrap around
        mode_idx = 0;
    }
}

inline void WDT_on() {
    // Setup watchdog timer to only interrupt, not reset, every 500ms.
    cli();                          // Disable interrupts
    wdt_reset();                    // Reset the WDT
    WDTCR |= (1<<WDCE) | (1<<WDE);  // Start timed sequence
    WDTCR = (1<<WDTIE) | (1<<WDP2) | (1<<WDP0); // Enable interrupt every 500ms
    sei();                          // Enable interrupts
}

inline void WDT_off()
{
    cli();                          // Disable interrupts
    wdt_reset();                    // Reset the WDT
    MCUSR &= ~(1<<WDRF);            // Clear Watchdog reset flag
    WDTCR |= (1<<WDCE) | (1<<WDE);  // Start timed sequence
    WDTCR = 0x00;                   // Disable WDT
    sei();                          // Enable interrupts
}

inline void ADC_on() {
    ADMUX  = (1 << REFS0) | (1 << ADLAR) | ADC_CHANNEL; // 1.1v reference, left-adjust, ADC1/PB2
    DIDR0 |= (1 << ADC_DIDR);                           // disable digital input on ADC pin to reduce power consumption
    ADCSRA = (1 << ADEN ) | (1 << ADSC ) | ADC_PRSCL;   // enable, start, prescale
}

inline void ADC_off() {
    ADCSRA &= ~(1<<7); //ADC off
}

#ifdef VOLTAGE_MON
uint8_t low_voltage(uint8_t voltage_val) {
    // Start conversion
    ADCSRA |= (1 << ADSC);
    // Wait for completion
    while (ADCSRA & (1 << ADSC));
    // See if voltage is lower than what we were looking for
    if (ADCH < voltage_val) {
        // See if it's been low for a while
        if (++lowbatt_cnt > 8) {
            lowbatt_cnt = 0;
            return 1;
        }
    } else {
        lowbatt_cnt = 0;
    }
    return 0;
}
#endif

ISR(WDT_vect) {
    static uint8_t ticks = 0;
    if (ticks < 255) ticks++;

    if (ticks == WDT_TIMEOUT) {
#if memory
        store_mode_idx(mode_idx);
#else
        // Reset the mode to the start for next time
        store_mode_idx(0);
#endif
    }
}

int main(void)
{
    // All ports default to input, but turn pull-up resistors on for the stars
    // (not the ADC input!  Made that mistake already)
    // (stars not used)
    //PORTB = (1 << STAR2_PIN) | (1 << STAR3_PIN) | (1 << STAR4_PIN);

    // Set PWM pin to output
    DDRB = (1 << PWM_PIN);

    // Set timer to do PWM for correct output pin and set prescaler timing
    TCCR0A = 0x23; // phase corrected PWM is 0x21 for PB1, fast-PWM is 0x23
    TCCR0B = 0x01; // pre-scaler for timer (1 => 1, 2 => 8, 3 => 64...)

    // Turn features on or off as needed
    #ifdef VOLTAGE_MON
    ADC_on();
    #else
    ADC_off();
    #endif
    ACSR   |=  (1<<7); //AC off

    // Enable sleep mode set to Idle that will be triggered by the sleep_mode() command.
    // Will allow us to go idle between WDT interrupts
    set_sleep_mode(SLEEP_MODE_IDLE);

    // Determine what mode we should fire up
    // Read the last mode that was saved
    read_mode_idx();
    if (mode_idx&0x20) {  // maximum of 32 modes allowed, uses bit 5 as a mode memory timer
        // Indicates we did a short press last time, go to the next mode
        // Remove short press indicator first
        mode_idx &= 0x1f;
        next_mode(); // Will handle wrap arounds
    } else {
        // Didn't have a short press, keep the same mode
    }
    // Store mode with short press indicator
    store_mode_idx(mode_idx|0x20);

    WDT_on();

    // Now just fire up the mode
    PWM_LVL = modes[mode_idx];

    uint8_t i = 0;
    uint8_t j = 0;
    uint8_t strobe_len = 0;
#ifdef VOLTAGE_MON
    uint8_t hold_pwm;
#endif
    while(1) {
        if(mode_idx < SOLID_MODES) { // Just stay on at a given brightness
            sleep_mode();
        } else if (mode_idx < DUAL_BEACON_MODES) { // two-level fast strobe pulse at about 1 Hz
            for(i=0; i<4; i++) {
                PWM_LVL = modes[mode_idx+1];
                _delay_ms(5);
                PWM_LVL = modes[mode_idx];
                _delay_ms(50);
            }
            _delay_ms(530);
        } else if (mode_idx < SINGLE_BEACON_MODES) { // heartbeat flasher
            PWM_LVL = modes[SINGLE_BEACON_MODES-1];
            _delay_ms(1);
            PWM_LVL = 0;
            _delay_ms(149);
            PWM_LVL = modes[SINGLE_BEACON_MODES-1];
            _delay_ms(1);
            PWM_LVL = 0;
            _delay_ms(599);
        } else if (mode_idx == SINGLE_BEACON_MODES) { // strobe mode, ~10 Hz
            // 24 Hz:  1.0 / 10 * 1000 * 3/4 == 75 "ms" cycle
            PWM_LVL = modes[SOLID_MODES-1];
            _delay_ms(1);
            PWM_LVL = 0;
            _delay_ms(74);
        } else if (mode_idx == SINGLE_BEACON_MODES+1) { // strobe mode, ~24 Hz (like a movie)
            // 24 Hz:  1.0 / 24 * 1000 * 3/4 == 31.25 "ms" cycle
            PWM_LVL = modes[SOLID_MODES-1];
            _delay_ms(1);
            PWM_LVL = 0;
            _delay_ms(30);
        } else { // strobe mode, smoothly oscillating frequency ~7 Hz to ~18 Hz
            for(j=0; j<50; j++) {
                PWM_LVL = modes[SOLID_MODES-1];
                _delay_ms(1);
                PWM_LVL = 0;
                if (j<25) { strobe_len = j; }
                else { strobe_len = 50-j; }
                for(i=5; i<strobe_len+25; i++) {
                    _delay_ms(2);
                }
            }
        }
    #ifdef VOLTAGE_MON
        if (low_voltage(ADC_LOW)) {
            // We need to go to a lower level
            if (PWM_LVL <= modes[0]) {
                // Can't go any lower than the lowest mode
                // Wait until we hit the critical level before flashing 10
                // times and turning off
                while (!low_voltage(ADC_CRIT));
                for(i=0; i<20; i++) {
                    PWM_LVL = (i&1) ? 0 : modes[0];
                    _delay_ms(250);
                }
                // Turn off the light
                PWM_LVL = 0;
                // Disable WDT so it doesn't wake us up
                WDT_off();
                // Power down as many components as possible
                set_sleep_mode(SLEEP_MODE_PWR_DOWN);
                sleep_mode();
            } else {
                // Flash 3 times before lowering
                hold_pwm = PWM_LVL;
                for(i=0; i<6; i++) {
                    PWM_LVL = (i&1) ? 0 : hold_pwm;
                    _delay_ms(250);
                }
                // Lower the mode by half, but don't go below lowest level
                PWM_LVL = (PWM_LVL >> 1);
                if (PWM_LVL < modes[0]) {
                    PWM_LVL = modes[0];
                    mode_idx = 0;
                }
#if memory
                // See if we should change the current mode level if we've gone
                // under the current mode.
                if (PWM_LVL < modes[mode_idx]) {
                    // Lower our recorded mode
                    mode_idx--;
                }
#endif
            }
            // Wait 3 seconds before lowering the level again
            _delay_ms(2250);
        }
    #endif
    }
}
