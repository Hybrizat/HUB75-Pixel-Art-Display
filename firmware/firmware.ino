//Original by mzashh https://github.com/mzashh
//Forked by Hybrizat https://github.com/Hybrizat

#define FIRMWARE_VERSION "v0.1.0a"
#define FILESYSTEM LittleFS
#include <LittleFS.h>
#include <AnimatedGIF.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "webpages.h"
#include "time.h"
#include <U8g2_for_Adafruit_GFX.h>
#include <ESP32-HUB75-VirtualMatrixPanel_T.hpp>
//#include <U8g2lib.h>

// ============================================================
//  Panel configuration area - just modify here
// ============================================================

// Resolution of each panel
#define PANEL_WIDTH  64
#define PANEL_HEIGHT 32

// Panel array (number of columns × number of rows)
// 1×1 = Single panel；2×1 = Two horizontal pieces；2×1 = Two vertical pieces；3×2 = Six pieces
#define PANEL_COLS   2
#define PANEL_ROWS   1

// How you connect multiple panels（ineffective when a single block is used）：
//   Serpentine arrangement：CHAIN_TOP_LEFT_DOWN  / CHAIN_TOP_RIGHT_DOWN
//   ZigZag：CHAIN_TOP_LEFT_DOWN_ZZ / CHAIN_TOP_RIGHT_DOWN_ZZ
#define CHAIN_TYPE   CHAIN_TOP_RIGHT_DOWN

// Pin Configuration
#define R1_PIN  25
#define G1_PIN  26
#define B1_PIN  27
#define R2_PIN  14
#define G2_PIN  12
#define B2_PIN  13
#define A_PIN   22
#define B_PIN   32
#define C_PIN   33
#define D_PIN   17
#define E_PIN   -1   // set to 17 if 1/32 scan；set to -1 if 1/16 scan
#define LAT_PIN  4
#define OE_PIN  15
#define CLK_PIN 16

// ============================================================
//  Automatically calculated by configuration
// ============================================================
#define TOTAL_WIDTH   (PANEL_WIDTH  * PANEL_COLS)
#define TOTAL_HEIGHT  (PANEL_HEIGHT * PANEL_ROWS)

// Clock coordinates
#define CLOCK_Y      (TOTAL_HEIGHT / 2 - 8)
#define CLOCK_H_X    (TOTAL_WIDTH  / 2 - 29)
#define CLOCK_M_X    (TOTAL_WIDTH  / 2 + 3)
#define CLOCK_DOT_X  (TOTAL_WIDTH  / 2 - 5)
#define CLOCK_DOT_Y1 (CLOCK_Y - 1)
#define CLOCK_DOT_Y2 (CLOCK_Y + 5)

// Scrolling text Y position
#define SCROLL_Y_SIZE1 (TOTAL_HEIGHT - 6) /2
#define SCROLL_Y_SIZE2 (TOTAL_HEIGHT - 9) /2
#define SCROLL_Y_SIZE3 (TOTAL_HEIGHT - 13) /2
#define SCROLL_Y_SIZE4 (TOTAL_HEIGHT - 17) /2
// ============================================================

MatrixPanel_I2S_DMA *dma_display = nullptr;
U8G2_FOR_ADAFRUIT_GFX u8g2Fonts;

VirtualMatrixPanel_T<CHAIN_TYPE> *virtualDisp = nullptr;

// drawpixel
inline void dispDrawPixel(int16_t x, int16_t y, uint16_t color) {
  virtualDisp->drawPixel(x, y, color);
}

// clearscreen
inline void dispClearScreen() {
  virtualDisp->clearScreen();
}

uint16_t myBLACK, myWHITE, myRED, myGREEN, myBLUE;
uint16_t textWidth = 0;
uint16_t w, h;
uint8_t colorR = 255;
uint8_t colorG = 255;
uint8_t colorB = 255;
uint8_t scrollFontSize = 2;
uint8_t scrollSpeed    = 18;
int16_t xOne, yOne;

const String default_ssid              = "SSID";
const String default_wifipassword      = "PASSWORD";
const String default_httpuser          = "admin";
const String default_httppassword      = "admin";
const int    default_webserverporthttp  = 80;
const char*  ntpServer                 = "ntp.aliyun.com";
const char*  PARAM_INPUT               = "value";
const long   gmtOffset_sec             = 28800;
const int    daylightOffset_sec        = 0;
const int    maxGIFsPerPage            = 4;

int textXPosition = TOTAL_WIDTH;
int textYPosition = TOTAL_HEIGHT / 2;

unsigned long lastPixelToggle  = 0;
unsigned long lastScrollUpdate = 0;
unsigned long isAnimationDue;
bool showFirstSet      = true;
bool clockEnabled      = true;
bool gifEnabled        = true;
bool scrollTextEnabled = false;
bool loopGifEnabled    = true;
bool lastPlayGifs      = true;

String inputMessage;
String sliderValue      = "100";
String scrollText       = "Hello";
String currentGifPath   = "";
String requestedGifPath = "";

struct Config {
  String ssid;
  String wifipassword;
  String httpuser;
  String httppassword;
  int    webserverporthttp;
};

Config          config;
bool            shouldReboot = false;
AsyncWebServer *server;

String listFiles(bool ishtml = false);

AnimatedGIF gif;
File        f;
int         x_offset, y_offset;


// ── GIFDraw ────────────────────────────────────────────────
void GIFDraw(GIFDRAW *pDraw)
{
  uint8_t  *s;
  uint16_t *d, *usPalette, usTemp[320];
  int       x, y, iWidth;

  iWidth = pDraw->iWidth;
  if (iWidth > TOTAL_WIDTH) iWidth = TOTAL_WIDTH;

  usPalette = pDraw->pPalette;
  y = pDraw->iY + pDraw->y;
  s = pDraw->pPixels;

  if (pDraw->ucDisposalMethod == 2) {
    for (x = 0; x < iWidth; x++) {
      if (s[x] == pDraw->ucTransparent) s[x] = pDraw->ucBackground;
    }
    pDraw->ucHasTransparency = 0;
  }

  if (gifEnabled) {
    if (pDraw->ucHasTransparency) {
      uint8_t *pEnd, c, ucTransparent = pDraw->ucTransparent;
      int iCount;
      pEnd = s + pDraw->iWidth;
      x = iCount = 0;
      while (x < pDraw->iWidth) {
        c = ucTransparent - 1;
        d = usTemp;
        while (c != ucTransparent && s < pEnd) {
          c = *s++;
          if (c == ucTransparent) { s--; }
          else { *d++ = usPalette[c]; iCount++; }
        }
        if (iCount) {
          for (int xOffset = 0; xOffset < iCount; xOffset++)
            dispDrawPixel(x + xOffset, y, usTemp[xOffset]);
          x += iCount; iCount = 0;
        }
        c = ucTransparent;
        while (c == ucTransparent && s < pEnd) {
          c = *s++;
          if (c == ucTransparent) iCount++;
          else s--;
        }
        if (iCount) { x += iCount; iCount = 0; }
      }
    } else {
      s = pDraw->pPixels;
      for (x = 0; x < iWidth; x++)
        dispDrawPixel(x, y, usPalette[*s++]);
    }
  }

  // ── Clock ─────────────────────────────────────────────
  if (clockEnabled) {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
      Serial.println("Failed to obtain time");
    }
    virtualDisp->setTextColor(dma_display->color565(colorR, colorG, colorB));
    virtualDisp->setTextSize(2);
    virtualDisp->setCursor(CLOCK_H_X, CLOCK_Y);
    virtualDisp->print(&timeinfo, "%H");   // "%I"12小时；换 "%H" 切换为24小时
    virtualDisp->setCursor(CLOCK_M_X, CLOCK_Y);
    virtualDisp->print(&timeinfo, "%M");

    if (millis() - lastPixelToggle >= 1000) {
      showFirstSet    = !showFirstSet;
      lastPixelToggle = millis();
    }
    uint16_t dotColor = showFirstSet
      ? virtualDisp->color565(colorR, colorG, colorB)
      : virtualDisp->color565(0, 0, 0);
    virtualDisp->setTextColor(dotColor);
    virtualDisp->setTextSize(1);
    virtualDisp->setCursor(CLOCK_DOT_X, CLOCK_DOT_Y1); virtualDisp->print(".");
    virtualDisp->setCursor(CLOCK_DOT_X, CLOCK_DOT_Y2); virtualDisp->print(".");
  }

  // ── Scrolling text ─────────────────────────────────────────
  if (scrollTextEnabled) {
    virtualDisp->setTextWrap(false);

    if      (scrollFontSize == 1) textYPosition = SCROLL_Y_SIZE1 +4;
    else if (scrollFontSize == 2) textYPosition = SCROLL_Y_SIZE2 +8;
    else if (scrollFontSize == 3) textYPosition = SCROLL_Y_SIZE3 +12;
    else if (scrollFontSize == 4) textYPosition = SCROLL_Y_SIZE4 +16;
    else                          textYPosition = SCROLL_Y_SIZE2 +8;

    byte offSet = 25;
    unsigned long now = millis();
    if (now > isAnimationDue) {
      virtualDisp->setTextSize(scrollFontSize);
      isAnimationDue = now + scrollSpeed;
      textXPosition -= 1;

      virtualDisp->getTextBounds(scrollText.c_str(), textXPosition, textYPosition,
                                 &xOne, &yOne, &w, &h);
      if (textXPosition + w <= 0)
        textXPosition = virtualDisp->width() + offSet;

      virtualDisp->setCursor(textXPosition, textYPosition);
      //dma_display->drawRect(0, textYPosition - 12, dma_display->width(), 42,
                            //dma_display->color565(0, 0, 0));
      //dma_display->fillRect(0, textYPosition - 12, dma_display->width(), 42,
                            //dma_display->color565(0, 0, 0));
      virtualDisp->clearScreen();
      for (uint8_t i = 0; i < strlen(scrollText.c_str()); i++) {
        //dma_display->setTextColor(dma_display->color565(colorR, colorG, colorB));
        u8g2Fonts.setForegroundColor(virtualDisp->color565(colorR, colorG, colorB));
        u8g2Fonts.drawUTF8(textXPosition, textYPosition, scrollText.c_str());
      }
    }
  }

} /* GIFDraw() */


// ── GIF file I/O  ────────────────────────────────────────
void *GIFOpenFile(const char *fname, int32_t *pSize) {
  //Serial.print("Playing gif: "); Serial.println(fname);
  f = FILESYSTEM.open(fname);
  if (f) { *pSize = f.size(); return (void *)&f; }
  return NULL;
}

void GIFCloseFile(void *pHandle) {
  File *fp = static_cast<File *>(pHandle);
  if (fp != NULL) fp->close();
}

int32_t GIFReadFile(GIFFILE *pFile, uint8_t *pBuf, int32_t iLen) {
  int32_t iBytesRead = iLen;
  File *fp = static_cast<File *>(pFile->fHandle);
  if ((pFile->iSize - pFile->iPos) < iLen)
    iBytesRead = pFile->iSize - pFile->iPos - 1;
  if (iBytesRead <= 0) return 0;
  iBytesRead = (int32_t)fp->read(pBuf, iBytesRead);
  pFile->iPos = fp->position();
  return iBytesRead;
}

int32_t GIFSeekFile(GIFFILE *pFile, int32_t iPosition) {
  File *fp = static_cast<File *>(pFile->fHandle);
  fp->seek(iPosition);
  pFile->iPos = (int32_t)fp->position();
  return pFile->iPos;
}

unsigned long start_tick = 0;

void ShowGIF(char *name) {
  // print GIF information only once
  static String lastPlayedGif = "";  
  bool gifInfoPrinted = true;

  if (lastPlayedGif != String(name)) {
    Serial.print("Playing gif: "); Serial.println(name);
    lastPlayedGif = String(name);
    gifInfoPrinted = false;
  } else {
    gifInfoPrinted = true;
  }

  if (gif.open(name, GIFOpenFile, GIFCloseFile, GIFReadFile, GIFSeekFile, GIFDraw)) {
    // print Canvas size only once
    if (!gifInfoPrinted) {
      Serial.printf("Successfully opened GIF; Canvas size = %d x %d\n",
                  gif.getCanvasWidth(), gif.getCanvasHeight());
      gifInfoPrinted = true;
    }
    //dma_display->clearScreen();
    while (gif.playFrame(true, NULL) == 1) {
      if (scrollText.length() > 0) {
        drawScrollingText();
      }
      yield();
      delay(5);
    }
    gif.close();
  }
  gifInfoPrinted = false;  // reset flag
}


// ── tool functions ─────────────────────────────────────────────────
void drawScrollingText() {
  static int scrollX = TOTAL_WIDTH;
  static unsigned long lastScrollTime = 0;

  if (scrollText.length() == 0) return;

  unsigned long now = millis();
  if (now > isAnimationDue) {
    isAnimationDue = now + scrollSpeed;
    textXPosition -= 1;
    if      (scrollFontSize == 1) textYPosition = SCROLL_Y_SIZE1;
    else if (scrollFontSize == 2) textYPosition = SCROLL_Y_SIZE2;
    else if (scrollFontSize == 3) textYPosition = SCROLL_Y_SIZE3;
    else if (scrollFontSize == 4) textYPosition = SCROLL_Y_SIZE4;
    else                          textYPosition = SCROLL_Y_SIZE2;
    dma_display->fillRect(0, scrollFontSize, TOTAL_WIDTH, 20, myBLACK);  
    int textWidth = u8g2Fonts.getUTF8Width(scrollText.c_str());

    scrollX -= 1; 
    if (scrollX < -textWidth) {
      scrollX = TOTAL_WIDTH;
    }
  }
}

void rebootESP(String message) {
  Serial.print("Rebooting ESP32: "); Serial.println(message);
  ESP.restart();
}

String humanReadableSize(const size_t bytes) {
  if (bytes < 1024)              return String(bytes) + " B";
  else if (bytes < (1024*1024))  return String(bytes / 1024.0) + " KB";
  else if (bytes < (1024*1024*1024)) return String(bytes / 1024.0 / 1024.0) + " MB";
  else return String(bytes / 1024.0 / 1024.0 / 1024.0) + " GB";
}

String listFiles(bool ishtml, int page = 1, int pageSize = maxGIFsPerPage) {
  String returnText = "";
  int fileIndex  = 0;
  int startIndex = (page - 1) * pageSize;
  int endIndex   = startIndex + pageSize;

  File root      = FILESYSTEM.open("/");
  File foundfile = root.openNextFile();

  if (ishtml) {
    returnText += "<!DOCTYPE HTML><html lang=\"en\"><head>";
    returnText += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
    returnText += "<meta charset=\"UTF-8\">";
    returnText += "</head><body>";
    returnText += "<table><tr><th>Name</th><th>Size</th><th>Preview</th><th>Actions</th></tr>";
  }
  while (foundfile) {
    if (fileIndex >= startIndex && fileIndex < endIndex) {
      if (ishtml) {
        returnText += "<tr><td>" + String(foundfile.name()) + "</td>";
        returnText += "<td>" + humanReadableSize(foundfile.size()) + "</td>";
        returnText += "<td><img src=\"/file?name=" + String(foundfile.name()) + "&action=show\" width=\"64\"></td>";
        returnText += "<td>";
        returnText += "<button onclick=\"downloadDeleteButton('" + String(foundfile.name()) + "', 'play')\">Play</button>";
        returnText += "<button onclick=\"downloadDeleteButton('" + String(foundfile.name()) + "', 'download')\">Download</button>";
        returnText += "<button onclick=\"downloadDeleteButton('" + String(foundfile.name()) + "', 'delete')\">Delete</button>";
        returnText += "</td></tr>";
      }
    }
    fileIndex++;
    foundfile = root.openNextFile();
  }
  if (ishtml) {
    returnText += "</table>";
    returnText += "<button onclick=\"window.location.href='/'\">Home</button>";
    if (page > 1)
      returnText += "<button onclick=\"window.location.href='/list?page=" + String(page-1) + "'\">Previous</button>";
    if (fileIndex > endIndex)
      returnText += "<button onclick=\"window.location.href='/list?page=" + String(page+1) + "'\">Next</button>";
    returnText += "<script>";
    returnText += "function downloadDeleteButton(filename, action) {";
    returnText += "    const url = `/file?name=${filename}&action=${action}`;";
    returnText += "    if (action === 'delete') {";
    returnText += "        fetch(url).then(r=>r.text()).then(()=>{alert('File deleted!');location.reload();});";
    returnText += "    } else if (action === 'download') {";
    returnText += "        window.open(url, '_blank');";
    returnText += "    } else if (action === 'play') {";
    returnText += "        fetch(url).then(r=>r.text()).then(()=>{alert('Playing...');});";
    returnText += "    }";
    returnText += "}";
    returnText += "</script>";
    returnText += "</body></html>";
  }
  root.close();
  return returnText;
}


// ── setup() ──────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  Serial.print("Firmware: "); Serial.println(FIRMWARE_VERSION);
  Serial.println("Booting ...");

  // --- HUB75 matrix setup ---
  HUB75_I2S_CFG mxconfig;
  mxconfig.mx_width     = PANEL_WIDTH;
  mxconfig.mx_height    = PANEL_HEIGHT;
  mxconfig.chain_length = PANEL_COLS * PANEL_ROWS;
  mxconfig.gpio.r1  = R1_PIN;  mxconfig.gpio.g1  = G1_PIN;  mxconfig.gpio.b1  = B1_PIN;
  mxconfig.gpio.r2  = R2_PIN;  mxconfig.gpio.g2  = G2_PIN;  mxconfig.gpio.b2  = B2_PIN;
  mxconfig.gpio.a   = A_PIN;   mxconfig.gpio.b   = B_PIN;   mxconfig.gpio.c   = C_PIN;
  mxconfig.gpio.d   = D_PIN;   mxconfig.gpio.e   = E_PIN;
  mxconfig.gpio.lat = LAT_PIN; mxconfig.gpio.oe  = OE_PIN;  mxconfig.gpio.clk = CLK_PIN;
  mxconfig.clkphase = false;

  dma_display = new MatrixPanel_I2S_DMA(mxconfig);
  dma_display->begin();
  dma_display->setBrightness8(sliderValue.toInt());
  dma_display->clearScreen();

  // color
  myBLACK = dma_display->color565(0,   0,   0);
  myWHITE = dma_display->color565(255, 255, 255);
  myRED   = dma_display->color565(255, 0,   0);
  myGREEN = dma_display->color565(0,   255, 0);
  myBLUE  = dma_display->color565(0,   0,   255);

  

  // --- virtual matrix ---
  virtualDisp = new VirtualMatrixPanel_T<CHAIN_TYPE>(
    PANEL_ROWS,
    PANEL_COLS,
    PANEL_WIDTH,
    PANEL_HEIGHT
  );
  virtualDisp->setDisplay(*dma_display);
  virtualDisp->clearScreen();
  Serial.printf("Virtual panel: %d col x %d row, total %d x %d px\n",
                PANEL_COLS, PANEL_ROWS, TOTAL_WIDTH, TOTAL_HEIGHT);

  u8g2Fonts.begin(*virtualDisp);
  u8g2Fonts.setFontDirection(0);// rotation
  u8g2Fonts.setForegroundColor(myWHITE);  // default text color
  u8g2Fonts.setBackgroundColor(myBLACK);   // background color
  u8g2Fonts.setFont(u8g2_font_wqy16_t_gb2312);  // fonts

  // --- LittleFS ---
  Serial.println("Mounting LittleFS ...");
  if (!LittleFS.begin(true)) {
    Serial.println("ERROR: Cannot mount LittleFS, Rebooting");
    rebootESP("ERROR: Cannot mount LittleFS");
  }
  Serial.println("Starting AnimatedGIFs Sketch");
  Serial.print("Flash Free: ");  Serial.println(humanReadableSize(LittleFS.totalBytes() - LittleFS.usedBytes()));
  Serial.print("Flash Used: ");  Serial.println(humanReadableSize(LittleFS.usedBytes()));
  Serial.print("Flash Total: "); Serial.println(humanReadableSize(LittleFS.totalBytes()));

  // --- WiFi ---
  config.ssid              = default_ssid;
  config.wifipassword      = default_wifipassword;
  config.httpuser          = default_httpuser;
  config.httppassword      = default_httppassword;
  config.webserverporthttp = default_webserverporthttp;

  Serial.print("\nConnecting to WiFi: ");
  WiFi.begin(config.ssid.c_str(), config.wifipassword.c_str());
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }

  Serial.println("\n\nNetwork Configuration:");
  Serial.println("----------------------");
  Serial.print("         SSID: "); Serial.println(WiFi.SSID());
  Serial.print("  Wifi Status: "); Serial.println(WiFi.status());
  Serial.print("Wifi Strength: "); Serial.print(WiFi.RSSI()); Serial.println(" dBm");
  Serial.print("          MAC: "); Serial.println(WiFi.macAddress());
  Serial.print("           IP: "); Serial.println(WiFi.localIP());
  Serial.print("       Subnet: "); Serial.println(WiFi.subnetMask());
  Serial.print("      Gateway: "); Serial.println(WiFi.gatewayIP());
  Serial.print("        DNS 1: "); Serial.println(WiFi.dnsIP(0));
  Serial.print("        DNS 2: "); Serial.println(WiFi.dnsIP(1));
  Serial.print("        DNS 3: "); Serial.println(WiFi.dnsIP(2));
  Serial.println();

  // --- Web server ---
  Serial.println("Configuring Webserver ...");
  server = new AsyncWebServer(config.webserverporthttp);
  configureWebServer();
  Serial.println("Starting Webserver ...");
  server->begin();

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  // --- Startup information display (adaptive overall resolution)---
  virtualDisp->fillScreen(myBLACK);
  virtualDisp->setTextSize(1);
  virtualDisp->setTextColor(myWHITE);
  virtualDisp->setCursor(0, 0);
  virtualDisp->print("ID:"); virtualDisp->print(FIRMWARE_VERSION);
  virtualDisp->setCursor(0, 9);
  virtualDisp->print("IP:"); virtualDisp->print(WiFi.localIP());
  if (TOTAL_HEIGHT >= 24) {
    virtualDisp->setCursor(0, 18);
    virtualDisp->print("RSSI:"); virtualDisp->print(WiFi.RSSI());
  }
  if (TOTAL_HEIGHT >= 33) {
    virtualDisp->setCursor(0, 27);
    virtualDisp->print("SSID:"); virtualDisp->print(WiFi.SSID());
  }
  delay(2000);

  virtualDisp->fillScreen(myBLACK);
  virtualDisp->clearScreen();
  gif.begin(LITTLE_ENDIAN_PIXELS);

  u8g2Fonts.setCursor(10, 20);
  u8g2Fonts.print("你好世界！测试中文");
  delay(2000);
  virtualDisp->clearScreen();
}


// ── loop() ───────────────────────────────────────────────────
String gifDir = "/";
char   filePath[256] = {0};
File   root, gifFile;

void loop() {
  if (shouldReboot) {
    rebootESP("Web Admin Initiated Reboot");
  }
  

  while (1) {
    root = FILESYSTEM.open(gifDir);
    if (root) {
      if (!requestedGifPath.isEmpty()) {
        currentGifPath   = requestedGifPath;
        requestedGifPath = "";
        while (gifFile = root.openNextFile()) {
          if (String(gifFile.path()) == currentGifPath) break;
        }
        if (gifFile) {
          memset(filePath, 0x0, sizeof(filePath));
          strcpy(filePath, gifFile.path());
          ShowGIF(filePath);
          if (loopGifEnabled) continue;
        }
      }
      // resume
      else if (!currentGifPath.isEmpty()) {
        while (gifFile = root.openNextFile()) {
          if (String(gifFile.path()) == currentGifPath) break;
        }
      }
      // from beginning
      else {
        gifFile = root.openNextFile();
      }

      while (gifFile) {
        if (!gifFile.isDirectory()) {
          memset(filePath, 0x0, sizeof(filePath));
          strcpy(filePath, gifFile.path());
          currentGifPath = String(filePath);
          ShowGIF(filePath);
          if (loopGifEnabled) continue;
        }
        if (!loopGifEnabled) {
          gifFile.close();
          gifFile = root.openNextFile();
          if (!gifFile) {
            root.close();
            root    = FILESYSTEM.open(gifDir);
            gifFile = root.openNextFile();
          }
        } else {
          break;
        }
      }
      root.close();
    }
    delay(10);
  }
}
