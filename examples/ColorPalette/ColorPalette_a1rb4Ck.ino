#include <FastLED.h>
#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
    #warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

//#define DEBUG

#define LED_PIN     9
#define NUM_LEDS    150
#define BRIGHTNESS  200  // must be <256, initialy was 64
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];

#define UPDATES_PER_SECOND 60  // 90 is fast, 120 is too fast
// 156 FPS with UPDATES_PER_SECOND 90 or 120

// All great ideas came from Mark: https://gist.github.com/kriegsman
// Based on ColorPalette and :
// FastLED "100-lines-of-code" demo reel, showing just a few 
// of the kinds of animation patterns you can quickly and easily 
// compose using FastLED.  
//
// This example also shows one easy way to define multiple 
// animations patterns and have them automatically rotate.
//
// -Mark Kriegsman, December 2014
// 
// based also on:
// ColorWavesWithPalettes
// Animated shifting color waves, with several cross-fading color palettes.
// by Mark Kriegsman, August 2015
//
// Color palettes courtesy of cpt-city and its contributors:
//   http://soliton.vm.bytemark.co.uk/pub/cpt-city/
//
// Color palettes converted for FastLED using "PaletteKnife" v1:
//   http://fastled.io/tools/paletteknife/
//

// This example shows several ways to set up and use 'palettes' of colors
// with FastLED.
//
// These compact palettes provide an easy way to re-colorize your
// animation on the fly, quickly, easily, and with low overhead.
//
// USING palettes is MUCH simpler in practice than in theory, so first just
// run this sketch, and watch the pretty lights as you then read through
// the code.  Although this sketch has eight (or more) different color schemes,
// the entire sketch compiles down to about 6.5K on AVR.
//
// FastLED provides a few pre-configured color palettes, and makes it
// extremely easy to make up your own color schemes with palettes.
//
// Some notes on the more abstract 'theory and practice' of
// FastLED compact palettes are at the bottom of this file.



//uint8_t gHue = 0; // rotating "base color" used by many of the patterns
int boolGlitter = 0; //boolean to add glitter to the current palette

CRGBPalette16 currentPalette;
CRGBPalette16 targetPalette;
TBlendType currentBlending;  // NOBLEND or LINEARBLEND

extern CRGBPalette16 myRedWhiteBluePalette;
extern const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM;
extern CRGBPalette16 myWeddingPalette;
extern const TProgmemPalette16 myWeddingPalette_p PROGMEM;


void setup() {
    delay( 3000 ); // power-up safety delay
    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip ).setDither(BRIGHTNESS);
    FastLED.setBrightness(  BRIGHTNESS );
    // You can change the brightness on the fly, i.e. with a potentiometer.

    // Trick from http://tuline.com/some-fastled-notes/:
    set_max_power_in_volts_and_milliamps(5, 2000);
    // This is used by the power management functionality, set at 5V 500mA.

    // RainbowColors_p, RainbowStripeColors_p, OceanColors_p, CloudColors_p,
    // LavaColors_p, ForestColors_p, and PartyColors_p
    currentPalette = RainbowColors_p;
    //currentPalette = CRGBPalette16(CRGB::Black);
    targetPalette = RainbowColors_p; // Used for smooth transitioning.
    currentBlending = LINEARBLEND;


    // Optional randomization
    random16_set_seed(4832); // Awesome randomizer
    random16_add_entropy(analogRead(2));
    int ranstart = random16();

    #ifdef DEBUG
        Serial.begin(57600); // Get the serial port running for debugging
    #endif
}


void loop()
{
    // do some periodic updates
    //gHue++; // slowly cycle the "base color" through the rainbow
    
    ChangePalettePeriodically();
    
    // Crossfade current palette slowly toward the target palette
    //
    // Each time that nblendPaletteTowardPalette is called, small changes
    // are made to currentPalette to bring it closer to matching targetPalette.
    // You can control how many changes are made in each call:
    //   - the default of 24 is a good balance
    //   - meaningful values are 1-48.  1=veeeeeeeery slow, 48=quickest
    //   - "0" means do not change the currentPalette at all; freeze
    // uint8_t maxChanges = 24; 
    // nblendPaletteTowardPalette( currentPalette, targetPalette, maxChanges);

    // Add a 15 seconds glitter every 2 minutes
    EVERY_N_MILLISECONDS(120000) {  // 60000 for a minute
      boolGlitter =1;
    }
    EVERY_N_MILLISECONDS(135000) {
      boolGlitter = 0;
    }
    
    EVERY_N_MILLISECONDS(100) {
        uint8_t maxChanges = 24;
        // AWESOME palette blending:
        nblendPaletteTowardPalette(currentPalette, targetPalette, maxChanges);
    }

    EVERY_N_MILLISECONDS( 1000 / UPDATES_PER_SECOND ) {
        static uint8_t startIndex = 0;
        startIndex = startIndex + 1; /* motion speed */
    
        FillLEDsFromPaletteColors( startIndex);

        // If you want to ensure you don’t overload your battery,
        // you might want to use power managed display. So, instead of:
        // FastLED.show();
        // Use the following to show the LED’s in loop():
        show_at_max_brightness_for_power();
        #ifdef DEBUG
            // Display frames per second on the serial monitor:
            Serial.println(LEDS.getFPS());
        #endif
    }
    // FastLED based non-blocking delay to update/display:
    // similar to EVERY_N_MILLISECONDS( 1000 / UPDATES_PER_SECOND ) {
    // FastLED.delay(1000 / UPDATES_PER_SECOND);
}

void FillLEDsFromPaletteColors( uint8_t colorIndex)
{
    uint8_t brightness = 255;
    
    for( int i = 0; i < NUM_LEDS; i++) {
        leds[i] = ColorFromPalette( currentPalette, colorIndex, brightness, currentBlending);
        colorIndex += 3;
    }
    if (boolGlitter==1) {addGlitter(80);}
}

// void rainbow() 
// {
//     // FastLED's built-in rainbow generator
//     fill_rainbow( leds, NUM_LEDS, gHue, 7);
// }
// void rainbowWithGlitter() 
// {
//     // built-in FastLED rainbow, plus some random sparkly glitter
//     rainbow();
//     addGlitter(80);
// }

void addGlitter( fract8 chanceOfGlitter) 
{
    if( random8() < chanceOfGlitter) {
        leds[ random16(NUM_LEDS) ] += CRGB::White;
    }
}

// void confetti() 
// {
//     // random colored speckles that blink in and fade smoothly
//     fadeToBlackBy( leds, NUM_LEDS, 10);
//     int pos = random16(NUM_LEDS);
//     leds[pos] += CHSV( gHue + random8(64), 200, 255);
// }

// void sinelon()
// {
//     // a colored dot sweeping back and forth, with fading trails
//     fadeToBlackBy( leds, NUM_LEDS, 20);
//     int pos = beatsin16(13,0,NUM_LEDS);
//     leds[pos] += CHSV( gHue, 255, 192);
// }

// void bpm()
// {
//     // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
//     uint8_t BeatsPerMinute = 62;
//     CRGBPalette16 palette = PartyColors_p;
//     uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
//     for( int i = 0; i < NUM_LEDS; i++) { //9948
//         leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
//     }
// }

// void juggle() {
//     // eight colored dots, weaving in and out of sync with each other
//     fadeToBlackBy( leds, NUM_LEDS, 20);
//     byte dothue = 0;
//     for( int i = 0; i < 8; i++) {
//         leds[beatsin16(i+7,0,NUM_LEDS)] |= CHSV(dothue, 200, 255);
//         dothue += 32;
//     }
// }


// There are several different palettes of colors demonstrated here.
//
// FastLED provides several 'preset' palettes: RainbowColors_p, RainbowStripeColors_p,
// OceanColors_p, CloudColors_p, LavaColors_p, ForestColors_p, and PartyColors_p.
//
// Additionally, you can manually define your own color palettes, or you can write
// code that creates color palettes on the fly.  All are shown here.



void ChangePalettePeriodically()
{
    uint8_t secondHand = (millis() / 1000) % 120;
    // default: % 60
    // Change % 60 to a different value to change duration of the loop.
    static uint8_t lastSecond = 99;
    // Static variable, means it’s only defined once. Our debounce variable.
    
    if( lastSecond != secondHand) {
        // Debounce to make sure we’re not repeating an assignment.
        lastSecond = secondHand;
//        if( secondHand ==  0)  { currentPalette = RainbowColors_p;         currentBlending = LINEARBLEND; }//top
//        //if( secondHand == 10)  { currentPalette = RainbowStripeColors_p;   currentBlending = NOBLEND;  }//non
//        //if( secondHand == 20)  { currentPalette = RainbowStripeColors_p;   currentBlending = LINEARBLEND; }//bof
//        //if( secondHand == 40)  { SetupPurpleAndGreenPalette();             currentBlending = LINEARBLEND; }//bof
//        if( secondHand == 10)  { SetupTotallyRandomPalette();              currentBlending = LINEARBLEND; }//top
//        //if( secondHand == 30)  { SetupBlackAndWhiteStripedPalette();       currentBlending = NOBLEND; }//non
//        if( secondHand == 20)  { SetupBlackAndWhiteStripedPalette();       currentBlending = LINEARBLEND; }//top,  noir blanc
//        if( secondHand == 30)  { currentPalette = CloudColors_p;           currentBlending = LINEARBLEND; }//top, bleu
//        if( secondHand == 40)  { currentPalette = PartyColors_p;           currentBlending = LINEARBLEND; }//top
//        //if( secondHand == 50)  { currentPalette = myRedWhiteBluePalette_p; currentBlending = NOBLEND;  }//blubl nc rouge bof
//        if( secondHand == 50)  { currentPalette = myRedWhiteBluePalette_p; currentBlending = LINEARBLEND; }//bleu blanc rouge top

        //Reorganisation:
        // TODO: transitions-DONE!
        // TODO: fire-NO
        //CRGB p = CHSV( HUE_PURPLE, 255, 255);
        //CRGB g = CHSV( HUE_GREEN, 255, 255);
        //CRGB b = CRGB::Black;
        //CRGB w = CRGB::White;
        if( secondHand ==  0)  { targetPalette = RainbowColors_p; boolGlitter = 0; currentBlending = LINEARBLEND;}//top
        //if( secondHand == 10)  { targetPalette = CRGBPalette16( g,g,b,b, p,p,b,b, g,g,b,b, p,p,b,b); currentBlending = LINEARBLEND;}
        //if( secondHand == 20)  { targetPalette = CRGBPalette16( b,b,b,w, b,b,b,w, b,b,b,w, b,b,b,w); currentBlending = LINEARBLEND;}        
        //if( secondHand ==  10)  { rainbow();                        currentBlending = LINEARBLEND;}//top
        //if( secondHand ==  10)  { boolGlitter = 1;                    currentBlending = LINEARBLEND;}//top
        //if( secondHand ==  30)  { confetti();                        currentBlending = LINEARBLEND;}//top
        //if( secondHand ==  40)  { sinelon();                        currentBlending = LINEARBLEND;}//top
        //if( secondHand ==  50)  { bpm();                        currentBlending = LINEARBLEND;}//top
        //if( secondHand ==  60)  { juggle();                        currentBlending = LINEARBLEND;}//top
        if( secondHand == 20)  { targetPalette = ForestColors_p;     currentBlending = LINEARBLEND;}//top, vert
        if( secondHand == 30)  { targetPalette = OceanColors_p;          currentBlending = LINEARBLEND;}//top, bleue
        if( secondHand == 40)  { targetPalette = CloudColors_p;           currentBlending = LINEARBLEND;}//top, bleu
        if( secondHand ==  50)  { targetPalette = ForestColors_p;        currentBlending = LINEARBLEND;}//top
        if( secondHand == 60)  { SetupBlackAndWhiteStripedPalette();     currentBlending = LINEARBLEND;}//top,  noir blanc
        //if( secondHand == 80)  { targetPalette = myRedWhiteBluePalette_p; currentBlending = LINEARBLEND;}//bleu blanc rouge top
        if( secondHand == 70)  { targetPalette = myWeddingPalette_p; currentBlending = LINEARBLEND;}//bleu blanc rouge top
        if( secondHand == 80)  { targetPalette = LavaColors_p;           currentBlending = LINEARBLEND;}//top, rouge
        if( secondHand == 100)  { targetPalette = PartyColors_p;          currentBlending = LINEARBLEND;}//top
        if( secondHand == 110)  { SetupTotallyRandomPalette();              currentBlending = LINEARBLEND;}//top
    }
}

// This function fills the palette with totally random colors.
void SetupTotallyRandomPalette()
{
    for( int i = 0; i < 16; i++) {
        //currentPalette[i] = CHSV( random8(), 255, random8());
        targetPalette[i] = CHSV( random8(), random8(), random8());
    }
}

// This function sets up a palette of black and white stripes,
// using code.  Since the palette is effectively an array of
// sixteen CRGB colors, the various fill_* functions can be used
// to set them up.
void SetupBlackAndWhiteStripedPalette()
{
    // 'black out' all 16 palette entries...
    fill_solid( targetPalette, 16, CRGB::Black);
    // and set every fourth one to white.
    // I've added gray to smooth the effect:
    targetPalette[0] = CRGB::White;
    //currentPalette[1] = CRGB::Gray;
    //currentPalette[2] = CRGB::Black;
    //currentPalette[3] = CRGB::Gray;
    targetPalette[4] = CRGB::White;
    //currentPalette[5] = CRGB::Gray;
    //currentPalette[6] = CRGB::Black;
     //currentPalette[7] = CRGB::Gray;
    targetPalette[8] = CRGB::White;
    //currentPalette[9] = CRGB::Gray;
    //currentPalette[10] = CRGB::Black;
     //currentPalette[11] = CRGB::Gray;
    targetPalette[12] = CRGB::White;
    //currentPalette[13] = CRGB::Gray;
    //currentPalette[14] = CRGB::Black;
    //currentPalette[15] = CRGB::Gray;
    //currentPalette[16] = CRGB::White;
    
}

// This function sets up a palette of purple and green stripes.
void SetupPurpleAndGreenPalette()
{
    CRGB purple = CHSV( HUE_PURPLE, 255, 255);
    CRGB green  = CHSV( HUE_GREEN, 255, 255);
    CRGB black  = CRGB::Black;
    
    targetPalette = CRGBPalette16(
                                   green,  green,  black,  black,
                                   purple, purple, black,  black,
                                   green,  green,  black,  black,
                                   purple, purple, black,  black );
}


// This example shows how to set up a static color palette
// which is stored in PROGMEM (flash), which is almost always more
// plentiful than RAM.  A static PROGMEM palette like this
// takes up 64 bytes of flash.
const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM =
{
    CRGB::Red,
    CRGB::Gray,  // 'white' is too bright compared to red and blue
    CRGB::Blue,
    //CRGB::Black,
    
    CRGB::Red,
    CRGB::Gray,
    CRGB::Blue,
    //CRGB::Black,

    CRGB::Red,
    CRGB::Gray,
    CRGB::Blue,
    CRGB::Red,
    CRGB::Gray,
    CRGB::Blue,
    
    CRGB::Red,
    CRGB::Gray,
    CRGB::Blue,
    CRGB::Pink,
    //CRGB::Black,
    //CRGB::Black
};

const TProgmemPalette16 myWeddingPalette_p PROGMEM =
{
    CRGB::Blue,
    CRGB::Blue,
    CRGB::Blue,
    CRGB::Green,
    CRGB::Green,
    CRGB::Green,
    CRGB::Yellow,
    CRGB::Yellow,
    CRGB::Gray,  // 'white' is too bright compared to red and blue
    CRGB::Pink,
    CRGB::Pink,
    CRGB::Pink,
    CRGB::Red,
    CRGB::Purple,
    CRGB::Purple,
    CRGB::Purple
};



// Additionl notes on FastLED compact palettes:
//
// Normally, in computer graphics, the palette (or "color lookup table")
// has 256 entries, each containing a specific 24-bit RGB color.  You can then
// index into the color palette using a simple 8-bit (one byte) value.
// A 256-entry color palette takes up 768 bytes of RAM, which on Arduino
// is quite possibly "too many" bytes.
//
// FastLED does offer traditional 256-element palettes, for setups that
// can afford the 768-byte cost in RAM.
//
// However, FastLED also offers a compact alternative.  FastLED offers
// palettes that store 16 distinct entries, but can be accessed AS IF
// they actually have 256 entries; this is accomplished by interpolating
// between the 16 explicit entries to create fifteen intermediate palette
// entries between each pair.
//
// So for example, if you set the first two explicit entries of a compact 
// palette to Green (0,255,0) and Blue (0,0,255), and then retrieved 
// the first sixteen entries from the virtual palette (of 256), you'd get
// Green, followed by a smooth gradient from green-to-blue, and then Blue.
