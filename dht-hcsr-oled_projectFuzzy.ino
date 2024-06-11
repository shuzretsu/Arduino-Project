#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_I2CDevice.h>
#include <DHT.h>
/***********************************
+==================================+
|DHT11                             |
|VCC   = 5V                        |
|GND   = GND                       |
|Data  = pin 2                     |
+----------------------------------+
|HC-SR04                           |
|VCC   = 5V                        |
|GND   = GND                       |
|Trig  = pin 9                     |
|Echo  = pin 10                    |
+----------------------------------+
|OLED Display                      |
|VCC   = 5V                        |
|GND   = GND                       |
|SCL   = A5 (SCL)                  |
|SDA   = A4 (SDA)                  |
+==================================+
***********************************/
// mendefinisi ukuran layar oled
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
// OLED I2C menggunakan address 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
//definisi DHT11 sensor pada pin 
#define DHTPIN 2       // Pin 2 untuk mengkoneksikan data untuk modul dht
#define DHTTYPE DHT11  // Tipe modul = DHT 11
DHT dht(DHTPIN, DHTTYPE);
// definisi HC-SR04 sensor
#define TRIGPIN 9      // Pin 9  koneksi > Trig
#define ECHOPIN 10     // Pin 10 konekis > Echo
// Jarak threshold untuk mendeteksi kehadiran dalam 10 m
#define THRESHOLD_DISTANCE 1000

void setup() {
  Serial.begin(9600);
  
  // inisialisasi sebsor DHT11 sensor
  dht.begin();
  
  // inisialisasi untuk mendisplay OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("Alokasi SSD1306 GAGAL"));
    for(;;);
  }
  display.clearDisplay();
  display.display();

  // inisialisasi sensor HC-SR04
  pinMode(TRIGPIN, OUTPUT);
  pinMode(ECHOPIN, INPUT);
}

void loop() {
  // jarak untuk hcsr
  long distance = getDistance();
  
  if (distance < THRESHOLD_DISTANCE) {
    // temperature dan humidity DHT11
    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();
    
    // memeriksa jika modul tidak terbaca/gagal maka akan dicoba lagi
    if (isnan(humidity) || isnan(temperature)) {
      Serial.println(F("Failed = read from DHT sensor!"));
      return;
    }
/*
+============================================+
|               FUZZY LOGIC                  |     
+============================================+
*/    
    String temperatureCategory = applyFuzzyLogic(temperature);

    // Output : hasil yang akan di display ke layar OLED
    displayResults(humidity, temperature, temperatureCategory);
  } else {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(F("tidak ada orang"));
    display.display();
  }
  
  delay(2000); // Delay
}

long getDistance() {
  digitalWrite(TRIGPIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGPIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGPIN, LOW);
  
  long duration = pulseIn(ECHOPIN, HIGH);
  long distance = (duration / 2) / 29.1; // Convert = cm
  
  return distance;
}

String applyFuzzyLogic(float temperature) {
  float cold = 0.0;
  float medium = 0.0;
  float hot = 0.0;
  
  // Menghitung nilai keanggotaan untuk variabel dingin(Cold)
  if (temperature <= 26) {
    cold = 1.0;
  } else if (temperature > 26 && temperature < 28) {
    cold = (28 - temperature) / 2.0;
  } else {
    cold = 0.0;
  }
  
  // Menghitung nilai keanggotaan untuk variabel medium temperatur
  if (temperature >= 26 && temperature <= 30) {
    medium = (temperature - 26) / 4.0;
  } else if (temperature > 30 && temperature <= 32) {
    medium = (32 - temperature) / 2.0;
  } else {
    medium = 0.0;
  }
  
  // menghitung nilai kenggotaan variabel temperatur panas(Hot)
  if (temperature >= 30 && temperature <= 32) {
    hot = (temperature - 30) / 2.0;
  } else if (temperature > 32 && temperature <= 36) {
    hot = (36 - temperature) / 4.0;
  } else {
    hot = 0.0;
  }
  
  // menentukan kategori dengan nilai keanggotaan tertinggi
  if (cold > medium && cold > hot) {
    return "Cold";
  } else if (medium > cold && medium > hot) {
    return "Medium";
  } else if (hot > cold && hot > medium) {
    return "Hot";
  } else {
    return "Uncertain";
  }
}
/*
+============================================+
|                  DISPLAY                   |     
+============================================+
*/

void displayResults(float humidity, float temperature, String temperatureCategory) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  
  display.print(F("Humidity: "));
  display.print(humidity);
  display.println(F(" %"));
  
  display.print(F("Temperature: "));
  display.print(temperature);
  display.println(F(" C"));

  display.print(F("Category: "));
  display.println(temperatureCategory);

  display.display();
}

