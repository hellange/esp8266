/************************************************
 *  Next generation weather station base
 *  
 *  FT81x graphics driver is copied from jamesbowman / gd2-lib
 *  (https://github.com/jamesbowman/gd2-lib.git)
 *  
 *  Connections:

 VM810C          UNO

 SCK             13
 MOSI            11          
 MISO            12
 CS               8 
 5V              5V
 GND            GND


 ER-TPC070-6    UNO

 1                          GND
 2              A4          SDA
 3              A5          SCL
 4                          RESET
 5              2           INT
 6                          NC
 7              3.3V        VDD
 8                          VDD
 9                          GND
 10             GND         GND
 
 ************************************************/

#include <SPI.h>
#include "GD2.h"
#include "Wire.h"
#include "walk_assets.h"

#define CTP_INT           2    // touch data ready for read from touch panel


void serialDebugOutput(int nr_of_touches, word *coordinates) {
  for (byte i = 0; i < nr_of_touches; i++){

    word x = coordinates[i * 2];
    word y= coordinates[i * 2 + 1];
    
    Serial.print("x");
    Serial.print(i);
    Serial.print("=");
    Serial.print(x);
    Serial.print(",");
    Serial.print("y");
    Serial.print(i);
    Serial.print("=");
    Serial.println(y);
  }
}

void setup()
{
  wdt_disable();
  Serial.begin(9600);
  Serial.println("Initializing WeatherNG graphics controller FT81x...");
  GD.begin(0);
  LOAD_ASSETS();
  Serial.println("Done.");

}

void drawMainText() {



  GD.ColorA(GD.random(256));

  // X centered at upper left corner
  GD.cmd_text(0, 0 , 31, OPT_CENTER, "X");

  // Text centered on screen
  GD.cmd_text(GD.w / 2, GD.h / 2, 31, OPT_CENTER, "WeatherNG");
  GD.cmd_text(GD.w / 2, GD.h / 2 + 30 , 28, OPT_CENTER, "Hello, mini world");

  // X centered at lower right corner
  GD.cmd_text(GD.w, GD.h, 31, OPT_CENTER, "X");

  randomSeed(analogRead(0));

}


void drawCircle(word x, word y, word pixels) {
  GD.PointSize(16 * pixels);
  GD.Begin(POINTS);
  //GD.Vertex2ii(x, y); // vertex2ii only handles up to 512 pixels !!! Use 2f instead
  GD.Vertex2f(x * 16, y * 16);
}

void drawSprite(int16_t x, int16_t y, byte handle, byte cell) {
  // In order to draw bitmap on x coordinates higher that 512
  // we use VertexTranslateX which according to documentation:
  // "Specifies the offset added to vertex X coordinates. 
  //  This command allows drawing to be shifted on the screen"
  // Not sure if this is the right approach, but it seems to work
//  if (x < 0) {
//    // For negative x we shift instead of setting x
//    GD.VertexTranslateX(x * 16 );
//    x = 0; 
//  } else if (x > 511) {
//    // for high values we start to use shift
//    // and reduces the x value accordingly
//    GD.VertexTranslateX(511 * 16);
//    x = x - 512;
//  } else {
//    GD.VertexTranslateX(0);
//  }
 
  GD.Begin(BITMAPS);
  GD.ColorRGB(255,255,255);
  GD.ColorA(255);
  GD.Vertex2ii(x, y, handle, cell);

  GD.RestoreContext();
}

void drawRandomCircles(int nr) {
  GD.Begin(POINTS);
  for (int i = 0; i < nr; i++) {
    GD.PointSize(GD.random(16*50));
    GD.ColorRGB(GD.random(256),
                GD.random(256),
                GD.random(256));
    GD.ColorA(GD.random(256));

    int16_t x = GD.random(800);
    int16_t y = GD.random(480);
    
//    if (x < 0) {
//      // For negative x we shift instead of setting x
//      GD.VertexTranslateX(x * 16 );
//      x = 0; 
//    } else if (x > 511) {
//      // for high values we start to use shift
//      // and reduces the x value accordingly
//      GD.VertexTranslateX(511 * 16);
//      x = x - 511;
//    } else {
//      GD.VertexTranslateX(0);
//    }

    GD.Vertex2ii(x, y);
  }
}

// Trying to solve problem with constant watchdog resets
// ref. http://internetofhomethings.com/homethings/?p=396
void delayWithYield(int ms) {
  int i;
  for(i=1;i!=ms;i++) {
    delay(1);
    if(i%50 == 0) {
      ESP.wdtFeed(); 
      yield();
    }
  }
}

int outsize_screen = 20; // how many pixels outside screen we will use
int walker_figure = 0;
int walker_position = -outsize_screen;

void loop()
{

  Serial.println("looping...");
  GD.ClearColorRGB(0x0000dd);
  GD.Clear();


  drawRandomCircles(100);
  yield();
  drawMainText();

  walker_position = walker_position + 2;
  if (walker_position > 800 + outsize_screen) {
    walker_position = - outsize_screen;
  }

  walker_figure = walker_figure + 1;
  if (walker_figure > 7) {
    walker_figure = 0;
  }
  drawSprite(walker_position, 250, WALK_HANDLE, walker_figure); // animate
  
  GD.swap();    
  delayWithYield(100);

}
