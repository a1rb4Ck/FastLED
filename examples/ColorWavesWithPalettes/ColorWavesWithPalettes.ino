#include "FastLED.h"

#if FASTLED_VERSION < 3001000
    #error "Requires FastLED 3.1 or later; check github for latest code."
#endif

//#define DEBUG

#define DATA_PIN    9//3
// #define CLK_PIN   4
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define NUM_LEDS    300  // 150 or 60
#define BRIGHTNESS  200  // must be <256, initialy==255

CRGB leds[NUM_LEDS];

#define UPDATES_PER_SECOND 60  // 50 perfect, 90 fast, 120 too fast
// 156 FPS with UPDATES_PER_SECOND 90 or 120
// initialy: 20 FPS
// LEDS.getFPS() shows 177 FPS!


// ColorWavesWithPalettes
// Animated shifting color waves, with several cross-fading color palettes.
// by Mark Kriegsman, August 2015
// https://gist.github.com/kriegsman/8281905786e8b2632aeb
//
// Color palettes courtesy of cpt-city and its contributors:
//   http://soliton.vm.bytemark.co.uk/pub/cpt-city/
//
// Color palettes converted for FastLED using "PaletteKnife" v1:
//   http://fastled.io/tools/paletteknife/
//


// ten seconds per color palette makes a good demo
// 20-120 is better for deployment
#define SECONDS_PER_PALETTE 60

#define SECONDS_PER_GLITTER 120
int boolGlitter = 0;  // boolean to add glitter to the current palette

void setup() {
    delay(3000); // 3 second delay for recovery

    // tell FastLED about the LED strip configuration
    FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS)
    //.setCorrection(TypicalLEDStrip) // cpt-city palettes have different color balance
    .setDither(BRIGHTNESS < 255);

    // set master brightness control
    FastLED.setBrightness(BRIGHTNESS);

    // hinty tricks from http://tuline.com/some-fastled-notes/:
    set_max_power_in_volts_and_milliamps(5, 2000); // This is used by the power management functionality and is currently set at 5V, 2000mA.

    #ifdef DEBUG
        Serial.begin(57600); // Get the serial port running for debugging
    #endif
}

// Forward declarations of an array of cpt-city gradient palettes, and 
// a count of how many there are.  The actual color palette definitions
// are at the bottom of this file.
extern const TProgmemRGBGradientPalettePtr gGradientPalettes[];
extern const uint8_t gGradientPaletteCount;

// Current palette number from the 'playlist' of color palettes
uint8_t gCurrentPaletteNumber = 0;

CRGBPalette16 gCurrentPalette( CRGB::Black);
// CRGBPalette16 gCurrentPalette( gGradientPalettes[0]);
CRGBPalette16 gTargetPalette( gGradientPalettes[0] );


void loop()
{
    EVERY_N_SECONDS( SECONDS_PER_PALETTE ) {
    gCurrentPaletteNumber = addmod8( gCurrentPaletteNumber, 1, gGradientPaletteCount);
    gTargetPalette = gGradientPalettes[ gCurrentPaletteNumber ];
    #ifdef DEBUG
        gCurrentPalette = gGradientPalettes[ gCurrentPaletteNumber ];
        Serial.println(gCurrentPaletteNumber);
    #endif
    }

    //Add a 15 seconds glitter every 2 minutes
    EVERY_N_SECONDS( SECONDS_PER_GLITTER ) {  // 60000 for a minute
        boolGlitter =1;
    }
        EVERY_N_SECONDS( SECONDS_PER_GLITTER + 10 ) {  // +15
        boolGlitter = 0;
    }

    EVERY_N_MILLISECONDS(40) {  // initialy 40, other 100
        // Crossfade current palette slowly toward the target palette
        //
        // Each time that nblendPaletteTowardPalette is called, small changes
        // are made to currentPalette to bring it closer to matching targetPalette.
        // You can control how many changes are made in each call:
        //   - the default of 24 is a good balance
        //   - meaningful values are 1-48.  1=veeeeeeeery slow, 48=quickest
        //   - "0" means do not change the currentPalette at all; freeze
        uint8_t maxChanges = 48; //24 //initialy 16
        nblendPaletteTowardPalette( gCurrentPalette, gTargetPalette, maxChanges);
    }
  
      colorwaves( leds, NUM_LEDS, gCurrentPalette);
      // palettetest( leds, NUM_LEDS, gCurrentPalette);

      // If you want to ensure you don’t overload your battery,
      // you might want to use power managed display. So, instead of:
      // FastLED.show();
      // Use the following to show the LED’s in loop():
      show_at_max_brightness_for_power();
      FastLED.delay(1000 / UPDATES_PER_SECOND);  //initialy==20FPS

      //#ifdef DEBUG
      //Serial.println(LEDS.getFPS()); // Display frames per second on the serial monitor.
      //#endif
}

// This function draws color waves with an ever-changing,
// widely-varying set of parameters, using a color palette.
void colorwaves( CRGB* ledarray, uint16_t numleds, CRGBPalette16& palette) 
{
    static uint16_t sPseudotime = 0;
    static uint16_t sLastMillis = 0;
    static uint16_t sHue16 = 0;

    uint8_t sat8 = beatsin88( 87, 220, 250);
    uint8_t brightdepth = beatsin88( 341, 96, 224);
    uint16_t brightnessthetainc16 = beatsin88( 203, (25 * 256), (40 * 256));
    uint8_t msmultiplier = beatsin88(147, 23, 60);

    uint16_t hue16 = sHue16;  // gHue * 256;
    uint16_t hueinc16 = beatsin88(113, 300, 1500);

    uint16_t ms = millis();
    uint16_t deltams = ms - sLastMillis ;
    sLastMillis  = ms;
    sPseudotime += deltams * msmultiplier;
    sHue16 += deltams * beatsin88( 400, 5,9);
    uint16_t brightnesstheta16 = sPseudotime;

    for( uint16_t i = 0 ; i < numleds; i++) {
        hue16 += hueinc16;
        uint8_t hue8 = hue16 / 256;
        uint16_t h16_128 = hue16 >> 7;
        if( h16_128 & 0x100) {
            hue8 = 255 - (h16_128 >> 1);
        } else {
            hue8 = h16_128 >> 1;
        }

        brightnesstheta16  += brightnessthetainc16;
        uint16_t b16 = sin16( brightnesstheta16  ) + 32768;

        uint16_t bri16 = (uint32_t)((uint32_t)b16 * (uint32_t)b16) / 65536;
        uint8_t bri8 = (uint32_t)(((uint32_t)bri16) * brightdepth) / 65536;
        bri8 += (255 - brightdepth);

        uint8_t index = hue8;
        //  index = triwave8( index);
        index = scale8( index, 240);

        CRGB newcolor = ColorFromPalette( palette, index, bri8);

        uint16_t pixelnumber = i;
        pixelnumber = (numleds-1) - pixelnumber;

        nblend( ledarray[pixelnumber], newcolor, 128);
    }
    if (boolGlitter==1) {addGlitter(80);}
}

// Alternate rendering function just scrolls the current palette 
// across the defined LED strip.
void palettetest( CRGB* ledarray, uint16_t numleds, const CRGBPalette16& gCurrentPalette)
{
    static uint8_t startindex = 0;
    startindex--;
    fill_palette( ledarray, numleds, startindex, (256 / NUM_LEDS) + 1, gCurrentPalette, 255, LINEARBLEND);
}


void addGlitter( fract8 chanceOfGlitter) 
{
    if( random8() < chanceOfGlitter) {
        leds[ random16(NUM_LEDS) ] += CRGB::White;
    }
}


// Gradient Color Palette definitions for 33 different cpt-city color palettes.
//    956 bytes of PROGMEM for all of the palettes together,
//   +618 bytes of PROGMEM for gradient palette code (AVR).
//  1,494 bytes total for all 34 color palettes and associated code.

// Gradient palette "ib_jul01_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/ing/xmas/tn/ib_jul01.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 16 bytes of program space.

DEFINE_GRADIENT_PALETTE( ib_jul01_gp ) {
    0, 194,  1,  1,
   94,   1, 29, 18,
  132,  57,131, 28,
  255, 113,  1,  1};

// Gradient palette "es_vintage_57_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/es/vintage/tn/es_vintage_57.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 20 bytes of program space.

DEFINE_GRADIENT_PALETTE( es_vintage_57_gp ) {
    0,   2,  1,  1,
   53,  18,  1,  0,
  104,  69, 29,  1,
  153, 167,135, 10,
  255,  46, 56,  4};

// Gradient palette "es_vintage_01_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/es/vintage/tn/es_vintage_01.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 32 bytes of program space.

DEFINE_GRADIENT_PALETTE( es_vintage_01_gp ) {
    0,   4,  1,  1,
   51,  16,  0,  1,
   76,  97,104,  3,
  101, 255,131, 19,
  127,  67,  9,  4,
  153,  16,  0,  1,
  229,   4,  1,  1,
  255,   4,  1,  1};

// Gradient palette "es_rivendell_15_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/es/rivendell/tn/es_rivendell_15.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 20 bytes of program space.

DEFINE_GRADIENT_PALETTE( es_rivendell_15_gp ) {
    0,   1, 14,  5,
  101,  16, 36, 14,
  165,  56, 68, 30,
  242, 150,156, 99,
  255, 150,156, 99};

// Gradient palette "rgi_15_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/ds/rgi/tn/rgi_15.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 36 bytes of program space.

DEFINE_GRADIENT_PALETTE( rgi_15_gp ) {
    0,   4,  1, 31,
   31,  55,  1, 16,
   63, 197,  3,  7,
   95,  59,  2, 17,
  127,   6,  2, 34,
  159,  39,  6, 33,
  191, 112, 13, 32,
  223,  56,  9, 35,
  255,  22,  6, 38};

// Gradient palette "retro2_16_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/ma/retro2/tn/retro2_16.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 8 bytes of program space.

DEFINE_GRADIENT_PALETTE( retro2_16_gp ) {
    0, 188,135,  1,
  255,  46,  7,  1};

// Gradient palette "Analogous_1_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/nd/red/tn/Analogous_1.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 20 bytes of program space.

DEFINE_GRADIENT_PALETTE( Analogous_1_gp ) {
    0,   3,  0,255,
   63,  23,  0,255,
  127,  67,  0,255,
  191, 142,  0, 45,
  255, 255,  0,  0};

// Gradient palette "es_pinksplash_08_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/es/pink_splash/tn/es_pinksplash_08.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 20 bytes of program space.

DEFINE_GRADIENT_PALETTE( es_pinksplash_08_gp ) {
    0, 126, 11,255,
  127, 197,  1, 22,
  175, 210,157,172,
  221, 157,  3,112,
  255, 157,  3,112};

// Gradient palette "es_pinksplash_07_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/es/pink_splash/tn/es_pinksplash_07.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 28 bytes of program space.

DEFINE_GRADIENT_PALETTE( es_pinksplash_07_gp ) {
    0, 229,  1,  1,
   61, 242,  4, 63,
  101, 255, 12,255,
  127, 249, 81,252,
  153, 255, 11,235,
  193, 244,  5, 68,
  255, 232,  1,  5};

// Gradient palette "Coral_reef_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/nd/other/tn/Coral_reef.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 24 bytes of program space.

DEFINE_GRADIENT_PALETTE( Coral_reef_gp ) {
    0,  40,199,197,
   50,  10,152,155,
   96,   1,111,120,
   96,  43,127,162,
  139,  10, 73,111,
  255,   1, 34, 71};

// Gradient palette "es_ocean_breeze_068_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/es/ocean_breeze/tn/es_ocean_breeze_068.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 24 bytes of program space.

DEFINE_GRADIENT_PALETTE( es_ocean_breeze_068_gp ) {
    0, 100,156,153,
   51,   1, 99,137,
  101,   1, 68, 84,
  104,  35,142,168,
  178,   0, 63,117,
  255,   1, 10, 10};

// Gradient palette "es_ocean_breeze_036_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/es/ocean_breeze/tn/es_ocean_breeze_036.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 16 bytes of program space.

DEFINE_GRADIENT_PALETTE( es_ocean_breeze_036_gp ) {
    0,   1,  6,  7,
   89,   1, 99,111,
  153, 144,209,255,
  255,   0, 73, 82};

// Gradient palette "departure_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/mjf/tn/departure.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 88 bytes of program space.

DEFINE_GRADIENT_PALETTE( departure_gp ) {
    0,   8,  3,  0,
   42,  23,  7,  0,
   63,  75, 38,  6,
   84, 169, 99, 38,
  106, 213,169,119,
  116, 255,255,255,
  138, 135,255,138,
  148,  22,255, 24,
  170,   0,255,  0,
  191,   0,136,  0,
  212,   0, 55,  0,
  255,   0, 55,  0};

// Gradient palette "es_landscape_64_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/es/landscape/tn/es_landscape_64.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 36 bytes of program space.

DEFINE_GRADIENT_PALETTE( es_landscape_64_gp ) {
    0,   0,  0,  0,
   37,   2, 25,  1,
   76,  15,115,  5,
  127,  79,213,  1,
  128, 126,211, 47,
  130, 188,209,247,
  153, 144,182,205,
  204,  59,117,250,
  255,   1, 37,192};

// Gradient palette "es_landscape_33_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/es/landscape/tn/es_landscape_33.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 24 bytes of program space.

DEFINE_GRADIENT_PALETTE( es_landscape_33_gp ) {
    0,   1,  5,  0,
   19,  32, 23,  1,
   38, 161, 55,  1,
   63, 229,144,  1,
   66,  39,142, 74,
  255,   1,  4,  1};

// Gradient palette "rainbowsherbet_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/ma/icecream/tn/rainbowsherbet.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 28 bytes of program space.

DEFINE_GRADIENT_PALETTE( rainbowsherbet_gp ) {
    0, 255, 33,  4,
   43, 255, 68, 25,
   86, 255,  7, 25,
  127, 255, 82,103,
  170, 255,255,242,
  209,  42,255, 22,
  255,  87,255, 65};

// Gradient palette "gr65_hult_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/hult/tn/gr65_hult.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 24 bytes of program space.

DEFINE_GRADIENT_PALETTE( gr65_hult_gp ) {
    0, 247,176,247,
   48, 255,136,255,
   89, 220, 29,226,
  160,   7, 82,178,
  216,   1,124,109,
  255,   1,124,109};

// Gradient palette "gr64_hult_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/hult/tn/gr64_hult.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 32 bytes of program space.

DEFINE_GRADIENT_PALETTE( gr64_hult_gp ) {
    0,   1,124,109,
   66,   1, 93, 79,
  104,  52, 65,  1,
  130, 115,127,  1,
  150,  52, 65,  1,
  201,   1, 86, 72,
  239,   0, 55, 45,
  255,   0, 55, 45};

// Gradient palette "GMT_drywet_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/gmt/tn/GMT_drywet.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 28 bytes of program space.

DEFINE_GRADIENT_PALETTE( GMT_drywet_gp ) {
    0,  47, 30,  2,
   42, 213,147, 24,
   84, 103,219, 52,
  127,   3,219,207,
  170,   1, 48,214,
  212,   1,  1,111,
  255,   1,  7, 33};

// Gradient palette "ib15_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/ing/general/tn/ib15.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 24 bytes of program space.

DEFINE_GRADIENT_PALETTE( ib15_gp ) {
    0, 113, 91,147,
   72, 157, 88, 78,
   89, 208, 85, 33,
  107, 255, 29, 11,
  141, 137, 31, 39,
  255,  59, 33, 89};

// Gradient palette "Fuschia_7_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/ds/fuschia/tn/Fuschia-7.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 20 bytes of program space.

DEFINE_GRADIENT_PALETTE( Fuschia_7_gp ) {
    0,  43,  3,153,
   63, 100,  4,103,
  127, 188,  5, 66,
  191, 161, 11,115,
  255, 135, 20,182};

// Gradient palette "es_emerald_dragon_08_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/es/emerald_dragon/tn/es_emerald_dragon_08.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 16 bytes of program space.

DEFINE_GRADIENT_PALETTE( es_emerald_dragon_08_gp ) {
    0,  97,255,  1,
  101,  47,133,  1,
  178,  13, 43,  1,
  255,   2, 10,  1};

// Gradient palette "lava_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/neota/elem/tn/lava.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 52 bytes of program space.

DEFINE_GRADIENT_PALETTE( lava_gp ) {
    0,   0,  0,  0,
   46,  18,  0,  0,
   96, 113,  0,  0,
  108, 142,  3,  1,
  119, 175, 17,  1,
  146, 213, 44,  2,
  174, 255, 82,  4,
  188, 255,115,  4,
  202, 255,156,  4,
  218, 255,203,  4,
  234, 255,255,  4,
  244, 255,255, 71,
  255, 255,255,255};

// Gradient palette "fire_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/neota/elem/tn/fire.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 28 bytes of program space.

DEFINE_GRADIENT_PALETTE( fire_gp ) {
    0,   1,  1,  0,
   76,  32,  5,  0,
  146, 192, 24,  0,
  197, 220,105,  5,
  240, 252,255, 31,
  250, 252,255,111,
  255, 255,255,255};

// Gradient palette "Colorfull_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/nd/atmospheric/tn/Colorfull.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 44 bytes of program space.

DEFINE_GRADIENT_PALETTE( Colorfull_gp ) {
    0,  10, 85,  5,
   25,  29,109, 18,
   60,  59,138, 42,
   93,  83, 99, 52,
  106, 110, 66, 64,
  109, 123, 49, 65,
  113, 139, 35, 66,
  116, 192,117, 98,
  124, 255,255,137,
  168, 100,180,155,
  255,  22,121,174};

// Gradient palette "Magenta_Evening_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/nd/atmospheric/tn/Magenta_Evening.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 28 bytes of program space.

DEFINE_GRADIENT_PALETTE( Magenta_Evening_gp ) {
    0,  71, 27, 39,
   31, 130, 11, 51,
   63, 213,  2, 64,
   70, 232,  1, 66,
   76, 252,  1, 69,
  108, 123,  2, 51,
  255,  46,  9, 35};

// Gradient palette "Pink_Purple_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/nd/atmospheric/tn/Pink_Purple.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 44 bytes of program space.

DEFINE_GRADIENT_PALETTE( Pink_Purple_gp ) {
    0,  19,  2, 39,
   25,  26,  4, 45,
   51,  33,  6, 52,
   76,  68, 62,125,
  102, 118,187,240,
  109, 163,215,247,
  114, 217,244,255,
  122, 159,149,221,
  149, 113, 78,188,
  183, 128, 57,155,
  255, 146, 40,123};

// Gradient palette "Sunset_Real_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/nd/atmospheric/tn/Sunset_Real.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 28 bytes of program space.

DEFINE_GRADIENT_PALETTE( Sunset_Real_gp ) {
    0, 120,  0,  0,
   22, 179, 22,  0,
   51, 255,104,  0,
   85, 167, 22, 18,
  135, 100,  0,103,
  198,  16,  0,130,
  255,   0,  0,160};

// Gradient palette "es_autumn_19_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/es/autumn/tn/es_autumn_19.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 52 bytes of program space.

DEFINE_GRADIENT_PALETTE( es_autumn_19_gp ) {
    0,  26,  1,  1,
   51,  67,  4,  1,
   84, 118, 14,  1,
  104, 137,152, 52,
  112, 113, 65,  1,
  122, 133,149, 59,
  124, 137,152, 52,
  135, 113, 65,  1,
  142, 139,154, 46,
  163, 113, 13,  1,
  204,  55,  3,  1,
  249,  17,  1,  1,
  255,  17,  1,  1};

// Gradient palette "BlacK_Blue_Magenta_White_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/nd/basic/tn/BlacK_Blue_Magenta_White.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 28 bytes of program space.

DEFINE_GRADIENT_PALETTE( BlacK_Blue_Magenta_White_gp ) {
    0,   0,  0,  0,
   42,   0,  0, 45,
   84,   0,  0,255,
  127,  42,  0,255,
  170, 255,  0,255,
  212, 255, 55,255,
  255, 255,255,255};

// Gradient palette "BlacK_Magenta_Red_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/nd/basic/tn/BlacK_Magenta_Red.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 20 bytes of program space.

DEFINE_GRADIENT_PALETTE( BlacK_Magenta_Red_gp ) {
    0,   0,  0,  0,
   63,  42,  0, 45,
  127, 255,  0,255,
  191, 255,  0, 45,
  255, 255,  0,  0};

// Gradient palette "BlacK_Red_Magenta_Yellow_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/nd/basic/tn/BlacK_Red_Magenta_Yellow.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 28 bytes of program space.

DEFINE_GRADIENT_PALETTE( BlacK_Red_Magenta_Yellow_gp ) {
    0,   0,  0,  0,
   42,  42,  0,  0,
   84, 255,  0,  0,
  127, 255,  0, 45,
  170, 255,  0,255,
  212, 255, 55, 45,
  255, 255,255,  0};

// Gradient palette "Blue_Cyan_Yellow_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/nd/basic/tn/Blue_Cyan_Yellow.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 20 bytes of program space.

DEFINE_GRADIENT_PALETTE( Blue_Cyan_Yellow_gp ) {
    0,   0,  0,255,
   63,   0, 55,255,
  127,   0,255,255,
  191,  42,255, 45,
  255, 255,255,  0};

//const TProgmemPalette16 blackAndWhiteStripedPalette_p PROGMEM =
//{
//    // 'black out' all 16 palette entries...
//    // and set every fourth one to white.
//    //I had gray to smooth the effect:
//    CRGB::White,
//    CRGB::Black,//CRGB::Gray;
//    CRGB::Black,
//    CRGB::Black,//CRGB::Gray;
//    CRGB::White,
//    CRGB::Black,
//    CRGB::Black,
//    CRGB::Black,
//    CRGB::White,
//    CRGB::Black,
//    CRGB::Black,
//    CRGB::Black,
//    CRGB::White,
//    CRGB::Black,
//    CRGB::Black,
//    CRGB::Black,
//};

//const TProgmemPalette16 myWeddingPalette_p PROGMEM =
//{
//    CRGB::Blue,
//    CRGB::Blue,
//    CRGB::Blue,
//    CRGB::Green,
//    CRGB::Green,
//    CRGB::Green,
//    CRGB::Yellow,
//    CRGB::Yellow,
//    CRGB::Gray,// 'white' is too bright compared to red and blue
//    CRGB::Pink,
//    CRGB::Pink,
//    CRGB::Pink,
//    CRGB::Red,
//    CRGB::Purple,
//    CRGB::Purple,
//    CRGB::Purple
//};

// Single array of defined cpt-city color palettes.
// This will let us programmatically choose one based on
// a number, rather than having to activate each explicitly 
// by name every time.
// Since it is const, this array could also be moved 
// into PROGMEM to save SRAM, but for simplicity of illustration
// we'll keep it in a regular SRAM array.
//
// This list of color palettes acts as a "playlist"; you can
// add or delete, or re-arrange as you wish.

// FastLED provides several 'preset' palettes: RainbowColors_p, RainbowStripeColors_p,
// OceanColors_p, CloudColors_p, LavaColors_p, ForestColors_p, and PartyColors_p.
//
// Additionally, you can manually define your own color palettes, or you can write
// code that creates color palettes on the fly.  All are shown here.

////ONLY BOF:
////const TProgmemRGBGradientPalettePtr gGradientPalettes[] = {
//const TProgmemRGBGradientPalettePtr gGradientPalettes[] = {//we put it into PROGMEM
////  RainbowColors_p,
////  ForestColors_p,
////  OceanColors_p,
////  CloudColors_p,
////  ForestColors_p,
////  blackAndWhiteStripedPalette_p,
////  myWeddingPalette_p,
////  LavaColors_p,
////  PartyColors_p,
//  es_rivendell_15_gp, //1 blanc vert cyan pulse moauis
//  retro2_16_gp, //4 jaune orange TOP
//  es_pinksplash_08_gp, //BLEUE VIOLET CUAN ROUGE TOP
//  Coral_reef_gp, //3 cyan blanc vert TOP
//  es_pinksplash_07_gp, //4 rose blanc TOP
//  rainbowsherbet_gp, //14 rose magenta vert blanc TOP
//  gr65_hult_gp, //15 violet blanc bleu cyan TOP
//  GMT_drywet_gp, //7 jaune cyan ok
//  es_vintage_57_gp, //19 jaune rouge vert TOP
//  ib15_gp, //9 rose rouge pale mouais
//  Fuschia_7_gp, //21 rose bleue lent OK
//  es_emerald_dragon_08_gp, //22 vert noir bof
//  lava_gp, //12 rouge jaune blanc TOP
//  fire_gp, //23 rouge jaune blanc TOP
//  Magenta_Evening_gp, //violet rose TOP
//  Pink_Purple_gp, //27 bleue gris violet pale
//  BlacK_Blue_Magenta_White_gp, //29 bleu gnoir jaune
//  BlacK_Red_Magenta_Yellow_gp, //31 vert jaune noir TOP
//  Blue_Cyan_Yellow_gp }; //32 Jaune vert cyan TOP


////const TProgmemRGBGradientPalettePtr gGradientPalettes[] = {
//const TProgmemRGBGradientPalettePtr gGradientPalettes[] PROGMEM = {//we put it into PROGMEM
////  RainbowColors_p,
////  ForestColors_p,
////  OceanColors_p,
////  CloudColors_p,
////  ForestColors_p,
////  blackAndWhiteStripedPalette_p,
////  myWeddingPalette_p,
////  LavaColors_p,
////  PartyColors_p,
//
//  Sunset_Real_gp, //0 jaune rouge rose bleue TOP
//  es_rivendell_15_gp, //1blanc pale BOF pas de mouvmeent
//  es_ocean_breeze_036_gp, //blanc jaune cyan TOP
//  rgi_15_gp, //3violet rose
//  retro2_16_gp, //4 blanc balnc bof
//  Analogous_1_gp, //5 jaune vert cyan violet TOP
//  es_pinksplash_08_gp, //6 blanc gris bof
//  Coral_reef_gp, //7 blanc gris bof
//  es_ocean_breeze_068_gp, //8 jaune vert cyan TOP
//  es_pinksplash_07_gp, //9 rose blanc pas mal
//  es_vintage_01_gp, //10 rouge vert cyan TOP
//  departure_gp, //11 bleu jaune cyna TOP
//  es_landscape_64_gp, //12 rouge rose cyan bleue TOP
//  es_landscape_33_gp, //13 blanc oragen pale TOP
//  rainbowsherbet_gp, //14 blanc blanc bof
//  gr65_hult_gp, //15 violet noir pulse mouais
//  gr64_hult_gp, //16 rouge blanc noir dim TOP TOP TOP
//  GMT_drywet_gp, //17 blanc violet bleue mouais
//  ib_jul01_gp, //18 rose vert TOP
//  es_vintage_57_gp, //19 blanc blanc bof
//  ib15_gp, //20 rose dim pulse mouais
//  Fuschia_7_gp, //21 bleue rose dim pale mouais
//  es_emerald_dragon_08_gp, //22 bleue blanc très léger mouais
//  lava_gp, //23 blanc blanc bof
//  fire_gp, //24 rose violet ok
//  Colorfull_gp,//25 vert jaune pale TOP
//  Magenta_Evening_gp, //26 blanc blanc BOF
//  Pink_Purple_gp, //27 rose violet pulse bof
//  es_autumn_19_gp, //28 blanc jaune orange pale violet dim pulse TOP
//  BlacK_Blue_Magenta_White_gp, //29 blanc rose bleue pulse BOF
//  BlacK_Red_Magenta_Yellow_gp, //31 violet pulse bof
//  Blue_Cyan_Yellow_gp }; //32 blanc gris rose pale pulse

//const TProgmemRGBGradientPalettePtr gGradientPalettes[] = {
const TProgmemRGBGradientPalettePtr gGradientPalettes[] = {
    // We put it into PROGMEM: fail !!
    //  RainbowColors_p,
    //  ForestColors_p,
    //  OceanColors_p,
    //  CloudColors_p,
    //  ForestColors_p,
    //  blackAndWhiteStripedPalette_p,
    //  myWeddingPalette_p,
    //  LavaColors_p,
    //  PartyColors_p,
    Sunset_Real_gp,  // 0jaune rouge rose
    es_rivendell_15_gp,  // 1blanc pale
    es_ocean_breeze_036_gp,  // 2violet noir
    rgi_15_gp,  // 3violet rose
    retro2_16_gp,  // 4 jaune pale blanc
    Analogous_1_gp,  // 5 violet pale vert
    es_pinksplash_08_gp,  // 6 blanc gris
    Coral_reef_gp,  // 7 violet noir
    es_ocean_breeze_068_gp,  // 8 blanc rosepale blanc dim
    es_pinksplash_07_gp,  // 9 violet noir
    es_vintage_01_gp,  // 10 violet blanc
    departure_gp,  // 11 violet noir pulse
    es_landscape_64_gp,  // 12 cyan bleu violet
    es_landscape_33_gp,  // 13 jaune pale blanc bcyan TOP
    rainbowsherbet_gp,  // 14 vert bleu blanc pale pulse
    gr65_hult_gp,  // 15 blanc cyan pulse
    gr64_hult_gp,  // 16 violet noir pulse
    GMT_drywet_gp,  // 17 bleu jaune pale blanc TOP
    ib_jul01_gp,  // 18 bleue pale blanc
    es_vintage_57_gp,  // 19 violet noir pulse
    ib15_gp,  // 20 blanc blanc
    Fuschia_7_gp,  // 21 blanc gris
    es_emerald_dragon_08_gp,  // 22 cyan violet pulse
    lava_gp, // 23 jauen rose blanc TOP
    fire_gp,  // 24 bleue dim
    Colorfull_gp,  // 25 vert rouge rose TOP
    Magenta_Evening_gp,  // 26 blanc rose pale
    Pink_Purple_gp,  // 27 blanc gris
    es_autumn_19_gp,  // 28 blanc vioolet dim pulse
    BlacK_Blue_Magenta_White_gp,  // 29 blanc rose
    BlacK_Magenta_Red_gp,  // 30 jauen orange TOP
    BlacK_Red_Magenta_Yellow_gp,  // 31 violet noir pulse
    Blue_Cyan_Yellow_gp  //32 blanc rose violet pale pulse
};

// Count of how many cpt-city gradients are defined:
const uint8_t gGradientPaletteCount = 
    sizeof( gGradientPalettes) / sizeof( TProgmemRGBGradientPalettePtr );
