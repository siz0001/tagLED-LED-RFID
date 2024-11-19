#include <Arduino.h>
#include <PN5180ISO15693.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <WiFi.h>
// 핀 설정
// const byte nssPin = 5; //SSPin
// const byte busyPin = 16;
// const byte resetPin = 17;
// const byte SCKPin = 18;
// const byte MISOPin = 19;
// const byte MOSIPin = 21;

const byte busyPin = 34;
const byte SCKPin = 33;
const byte MISOPin = 32;
const byte MOSIPin = 21;
const byte nssPin = 22; // SSPin
const byte resetPin = 2;

// PN5180 객체 생성
PN5180ISO15693 nfc(nssPin, busyPin, resetPin);

const int panelResX = 32;  // Number of pixels wide of each INDIVIDUAL panel module.
const int panelResY = 32;  // Number of pixels tall of each INDIVIDUAL panel module.
const int panel_chain = 1; // Total number of panels chained one to another

MatrixPanel_I2S_DMA *dma_display = nullptr;

uint16_t myBLACK = dma_display->color565(0, 0, 0);
uint16_t myWHITE = dma_display->color565(255, 255, 255);
uint16_t myRED = dma_display->color565(255, 0, 0);
uint16_t myGREEN = dma_display->color565(0, 255, 0);
uint16_t myBLUE = dma_display->color565(0, 0, 255);


void displaySetup()
{
  HUB75_I2S_CFG mxconfig(
      panelResX,  // module width
      panelResY,  // module height
      panel_chain // Chain length
  );

  // If you are using a 64x64 matrix you need to pass a value for the E pin
  // The trinity connects GPIO 18 to E.
  // This can be commented out for any smaller displays (but should work fine with it)
  mxconfig.gpio.e = 18;

  // May or may not be needed depending on your matrix
  // Example of what needing it looks like:
  // https://github.com/mrfaptastic/ESP32-HUB75-MatrixPanel-I2S-DMA/issues/134#issuecomment-866367216
  mxconfig.clkphase = false;

  // Some matrix panels use different ICs for driving them and some of them have strange quirks.
  // If the display is not working right, try this.
  // mxconfig.driver = HUB75_I2S_CFG::FM6126A;

  dma_display = new MatrixPanel_I2S_DMA(mxconfig);
  dma_display->begin();
}


void setup()
{
  // 시리얼 통신 초기화
  Serial.begin(115200);
  WiFi.mode(WIFI_MODE_NULL);
  SPI.begin(SCKPin, MISOPin, MOSIPin, nssPin); // SCK, MISO, MOSI, SS

  // 초기화 메시지
  Serial.println("리더 초기화 중...");

  // NFC 리더 초기화
  nfc.begin();

  // NFC 리더 리셋
  Serial.println("리더 리셋 중...");
  nfc.reset();

  // NFC 필드 활성화 메시지
  Serial.println("NFC 필드 활성화 중...");

  // 제품 버전 확인
  uint8_t productVersion[2];
  nfc.readEEprom(PRODUCT_VERSION, productVersion, sizeof(productVersion));
  if (0xff == productVersion[1])
  { // 초기화 실패 확인
    Serial.println(F("리더 초기화 실패!"));
    Serial.println(F("다시 시도하려면 리셋을 누르세요..."));
    Serial.flush();
    exit(-1); // 실행 중지
  }

  // 준비 완료 메시지
  Serial.println("사용 준비 완료...");

  // NFC 리더 RF 필드 설정
  nfc.setupRF();

  // Initialize the display
  displaySetup();
  // Can be set between 0 and 255
  // WARNING: The birghter it is, the more power it uses
  // Could take up to 3A on full brightness
  dma_display->setBrightness8(255); // 0-255

  dma_display->clearScreen();
  dma_display->fillScreen(myWHITE);
}

void loop()
{
  // 태그 감지
  uint8_t uid[8];

  // 인벤토리 읽기 시도 (태그 ID)
  ISO15693ErrorCode rc = nfc.getInventory(uid);

  // 읽기 성공 시
  if (rc == ISO15693_EC_OK)
  {
    Serial.print(F("NFC 디바이스 감지됨... ID: "));
    for (int j = 0; j < sizeof(uid); j++)
    {
      Serial.print(uid[j], HEX);
      Serial.print(" ");
    }
    Serial.println();
    //  dma_display->fillScreen(myGREEN);
    dma_display->clearScreen();
  }
  else
  {
    dma_display->fillScreen(myRED);
  }
delay(100);
  // 다음 읽기 전 1초 지연
  // delay(1000);
}
