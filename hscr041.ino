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
|Trig  = pin 12                    |
|Echo  = pin 11                    |
+----------------------------------+
|OLED Display                      |
|VCC   = 5V                        |
|GND   = GND                       |
|SCL   = A5 (SCL)                  |
|SDA   = A4 (SDA)                  |
+==================================+
***********************************/

// Mendefinisi ukuran layar OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1 // OLED I2C menggunakan address 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Definisi DHT11 sensor pada pin
#define DHTPIN 2       // Pin 2 untuk mengkoneksikan data untuk modul DHT
#define DHTTYPE DHT11  // Tipe modul = DHT 11
DHT dht(DHTPIN, DHTTYPE);

// Definisi HC-SR04 sensor
#define TRIGPIN 12      // Pin 12 koneksi > Trig
#define ECHOPIN 11      // Pin 11 koneksi > Echo

// Jarak threshold untuk mendeteksi kehadiran dalam 0.5 m
#define THRESHOLD_DISTANCE 10

// Timing variables
unsigned long lastDetectionTime = 0;
const unsigned long detectionTimeout = 100; //
bool oledOn = false;

void setup() {
  Serial.begin(9600);

  // Inisialisasi sensor DHT11
  dht.begin();

  // Inisialisasi OLED display
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("Alokasi SSD1306 GAGAL"));
    for(;;); // Berhenti disini jika OLED gagal diinisialisasi
  }
  display.clearDisplay();
  display.display();
  Serial.println(F("OLED initialized successfully"));

  // Inisialisasi sensor HC-SR04
  pinMode(TRIGPIN, OUTPUT);
  pinMode(ECHOPIN, INPUT);
}

void loop() {
  // Jarak untuk HC-SR04
  long distance = getDistance();
  Serial.print(F("Measured distance: "));
  Serial.println(distance);

  if (distance < THRESHOLD_DISTANCE) {
    // Temperatur dan kelembaban dari DHT11
    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();

    // Memeriksa jika modul tidak terbaca/gagal maka akan dicoba lagi
    if (isnan(humidity) || isnan(temperature)) {
      Serial.println(F("Gagal membaca dari sensor DHT!"));
      return;
    }

    /*
    +============================================+
    |               FUZZY LOGIC                  |
    +============================================+
    */
    String temperatureCategory = applyFuzzyLogic(temperature);

    // Output: hasil yang akan di display ke layar OLED
    displayResults(humidity, temperature, temperatureCategory, distance);

    // Update the last detection time
    lastDetectionTime = millis();
    oledOn = true;
  } else {
    if (millis() - lastDetectionTime > detectionTimeout && oledOn) {
      // Turn off the OLED display if no detection for 1 second
      display.clearDisplay();
      display.display();
      oledOn = false;
    } else if (!oledOn) {
      // Display a message if OLED is off
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println(F("Tidak ada orang"));
      display.display();
    }
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

  long distance = (duration / 2) / 29.1;
  return distance;
}

String applyFuzzyLogic(float temperature) {
  float cold = 0.0;
  float medium = 0.0;
  float hot = 0.0;

  // Menghitung nilai keanggotaan untuk variabel dingin (Cold)
  if (temperature <= 29) {
    cold = 1.0;
  } else if (temperature > 29 && temperature < 29.5) {
    cold = (29.5 - temperature) / 0.5;
  } else {
    cold = 0.0;
  }

  // Menghitung nilai keanggotaan untuk variabel medium temperatur
  if (temperature >= 29.5 && temperature <= 30.5) {
    medium = (temperature - 29.5) / 1.0;
  } else if (temperature > 30.5 && temperature <= 32) {
    medium = (32 - temperature) / 1.5;
  } else {
    medium = 0.0;
  }

  // Menghitung nilai keanggotaan variabel temperatur panas (Hot)
  if (temperature >= 32) {
    hot = 1.0;
  } else if (temperature > 31.5 && temperature <= 32) {
    hot = (temperature - 31.5) / 0.5;
  } else {
    hot = 0.0;
  }

  // Menentukan kategori dengan nilai keanggotaan tertinggi
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

void displayResults(float humidity, float temperature, String temperatureCategory, long distance) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

  display.println(F("-------Welcome-------"));
  display.print(F("Humidity: "));
  display.print(humidity);
  display.println(F(" %"));

  display.print(F("Temperature: "));
  display.print(temperature);
  display.println(F(" C"));

  display.print(F("Kategori: "));
  display.println(temperatureCategory);
/***
  display.print(F("Distance: "));
  display.print(distance);
  display.println(F(" cm"));
*///
  display.display();
}
