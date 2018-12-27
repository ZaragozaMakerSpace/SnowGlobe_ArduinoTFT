#define BLACK   0x0001
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define ROSE  0xF81F
#define WHITE   0xFFFF
  
#define TFT_SD 10
#define pixelsize 30
  
#include <SPI.h>
#include <SD.h>
#include <ILI9341_kbv.h>
ILI9341_kbv tft;
  
class pixel {
      public: 
        int16_t _x,_y,_xp, _yp;
        uint16_t  _c, _cp;
        uint16_t  _xc = 0xFFFF; //exception color
  
        pixel::pixel(int16_t x= 0,int16_t y= 0,uint16_t c= 0): _x(x),_y(y),_c( c){
           
        };
        void setX( int16_t x= 0 ){ _x  = x; };
        void setY(int16_t y= 0 ){ _y  = y; };
        void setXY( int16_t x= 0,int16_t y= 0 ){ setX(x);setY(y); };
        void setColour(uint16_t c){  _c = c; }
        void setbckgColour(ILI9341_kbv& _tft){_cp = _tft.readPixel( _x, _y); }
        void setXColour(uint16_t c){ _xc = c;}
        void move( int8_t dx, int8_t dy){
          _xp = _x;
          _yp = _y;
          _x+= dx;
          _y+= dy;
        };
  
        void draw( ILI9341_kbv& _tft){
            
          if( _x >= _tft.width() ){
            _x = 0;
          }else if( _x <= 0 ){ _x = _tft.width(); } if( _y >= _tft.height() ){
            _y = 0;
          }else if( _y <= 0 ){
           _y =  _tft.height();
          }
          //Dibujamos el pixel que habia anteriormente
          if( _cp != _xc){
            _tft.drawPixel( _xp, _yp, _cp );
          }
          //Guardamos el siguiente pixel tras el movimiento
          _cp = _tft.readPixel( _x, _y);
          //Dibujamos el pixel actual
          _tft.drawPixel( _x, _y, _c );
        }
};
  
//Carpeta en la que guardamos las imagenes
String FOLDERROOT = "Pictures/";
uint8_t max_img = 40;
uint8_t image_counter = 0;
unsigned long auxtimer, colortimer;
  
//Creamos un vector de pixeles
pixel pixelvector[ pixelsize ];
  
void setup() {
  Serial.begin(9600);
  Serial.println("TFT Snow Globe");
    
  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(YELLOW);
    
  if(SD.begin(TFT_SD)){
    Serial.println("SD Init");
  }else{
    Serial.println("SD No Init");
    
  }
  String filename = FOLDERROOT +"A_0.bmp";
      
    Serial.print("Picture ");Serial.print(filename);Serial.println(" exists");
    int len = 30;
    char filecharname[len];
    filename.toCharArray(filecharname, len);
    bmpDraw(filecharname , 0, 0);
    
   //Configuraciones iniciales de cada pixel
  for(int i = 0 ; i< pixelsize ;i++){ pixelvector[i].setbckgColour(tft); 
    pixelvector[i].setColour( WHITE ); 
    pixelvector[i].setXColour( WHITE ); 
    pixelvector[i].setXY( random(0, tft.width() ) , random( 0 , tft.height()) ); 
   
  } 
   
  } 
   
  void loop() { 
    if( millis() - auxtimer > 10000){
      auxtimer = millis();
      image_counter++;
      if(image_counter > max_img){
        image_counter= 0;
       }
        String filename = FOLDERROOT +"A_"+image_counter+".bmp";
      
        Serial.print("Picture ");Serial.print(filename);Serial.println(" exists");
        int len = 30;
        char filecharname[len];
        filename.toCharArray(filecharname, len);
        bmpDraw(filecharname , 0, 0);
        for(int i = 0 ; i< pixelsize ;i++){
          pixelvector[i].setbckgColour(tft);
        }
    }
    for(int i = 0 ; i< pixelsize ;i++){ 
      pixelvector[i].move ( random(-3,3) , random(0,5)); 
      pixelvector[i].draw( tft ); 
      //pixelvector[i].setColour( WHITE ); 
    } 

    if( millis() - colortimer > 500){
      for(int i = 0 ; i< pixelsize ;i++){ 
        colortimer = millis();
       // pixelvector[i].setColour( tft.color565( random(0,255) , random(0,255), random(0,255)  ) ); 
        pixelvector[i].setColour( WHITE  ); 
        pixelvector[i].setXColour( RED );
      }
    }
    delay(50);
} 
  
 
  
uint16_t _read16(File f) {
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read(); // MSB
  return result;
}
 
uint32_t _read32(File f) {
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); // MSB
  return result;
}
 
#define BUFFPIXEL 20
void bmpDraw(char *fileName, int x, int y){
      File     bmpFile;
      int      bmpWidth, bmpHeight;   // W+H in pixels
      uint8_t  bmpDepth;              // Bit depth (currently must be 24)
      uint32_t bmpImageoffset;        // Start of image data in file
      uint32_t rowSize;               // Not always = bmpWidth; may have padding
      uint8_t  sdbuffer[3*BUFFPIXEL]; // pixel buffer (R+G+B per pixel)
         
      uint16_t lcdbuffer[BUFFPIXEL];  // pixel out buffer (16-bit per pixel)
      uint8_t  lcdidx = 0;
      boolean  first = true;
         
      uint8_t  buffidx = sizeof(sdbuffer); // Current position in sdbuffer
      boolean  goodBmp = false;       // Set to true on valid header parse
      boolean  flip    = true;        // BMP is stored bottom-to-top
      int      w, h, row, col;
      uint8_t  r, g, b;
      uint32_t pos = 0, startTime = millis();
 
      if((x >= tft.width()) || (y >= tft.height())) return;
 
      // Open requested file on SD card
      if ((bmpFile = SD.open(fileName)) == NULL) {
        Serial.println("File Not Found" );
        return;
      }else{
         Serial.println("File Found" );
        }
 
      if(_read16(bmpFile) == 0x4D42) { // BMP signature
 
        (void)_read32(bmpFile); // Read & ignore creator bytes
        Serial.print("File size: ");Serial.println( _read32(bmpFile) ); 
        bmpImageoffset = _read32(bmpFile); // Start of image data
        Serial.print(" Header size: ");Serial.println( _read32(bmpFile) );
 
        // Read DIB header
 
        bmpWidth  = _read32(bmpFile);
        bmpHeight = _read32(bmpFile);
        if(_read16(bmpFile) == 1) { // # planes -- must be '1'
          bmpDepth = _read16(bmpFile); // bits per pixel
 
          if((bmpDepth == 24) && (_read32(bmpFile) == 0)) { // 0 = uncompressed
 
            goodBmp = true; // Supported BMP format -- proceed!
 
            // BMP rows are padded (if needed) to 4-byte boundary
            rowSize = (bmpWidth * 3 + 3) & ~3;
 
            // If bmpHeight is negative, image is in top-down order.
            // This is not canon but has been observed in the wild.
            if(bmpHeight < 0) {
              bmpHeight = -bmpHeight;
              flip      = false;
            }
 
            // Crop area to be loaded
            w = bmpWidth;
            h = bmpHeight;
            if((x+w-1) >= tft.width())  w = tft.width()  - x;
            if((y+h-1) >= tft.height()) h = tft.height() - y;
 
            // Set TFT address window to clipped image bounds
            tft.setAddrWindow(x, y, x+w-1, y+h-1);
 
            for (row=0; row<h; row++) { // For each scanline...
              // Seek to start of scan line.  It might seem labor-
              // intensive to be doing this on every line, but this
              // method covers a lot of gritty details like cropping
              // and scanline padding.  Also, the seek only takes
              // place if the file position actually needs to change
              // (avoids a lot of cluster math in SD library).
              if(flip) // Bitmap is stored bottom-to-top order (normal BMP)
                pos = bmpImageoffset + (bmpHeight - 1 - row) * rowSize;
              else     // Bitmap is stored top-to-bottom
                pos = bmpImageoffset + row * rowSize;
              if(bmpFile.position() != pos) { // Need seek?
                bmpFile.seek(pos);
                buffidx = sizeof(sdbuffer); // Force buffer reload
              }
 
              for (col=0; col<w; col++) { // For each column...
                // Time to read more pixel data?
                if (buffidx >= sizeof(sdbuffer)) { // Indeed
                  // Push LCD buffer to the display first
 
                    if(lcdidx > 0) {
                      tft.pushColors(lcdbuffer, lcdidx, first);
                      lcdidx = 0;
                      first  = false;
                    }
 
                  bmpFile.read(sdbuffer, sizeof(sdbuffer));
                  buffidx = 0; // Set index to beginning
                }
 
                // Convert pixel from BMP to TFT format
                b = sdbuffer[buffidx++];
                g = sdbuffer[buffidx++];
                r = sdbuffer[buffidx++];
                 
                    lcdbuffer[lcdidx++] = tft.color565(r,g,b);
                } // end pixel
              } // end scanline
               
                if(lcdidx > 0) {
                  tft.pushColors(lcdbuffer, lcdidx, first);  
                } 
              Serial.print("Loaded in: ");Serial.println( millis() - startTime);
            } // end goodBmp
          }
        }
 
      bmpFile.close();
      if(!goodBmp) Serial.println("BMP format not recognized.");
    }
