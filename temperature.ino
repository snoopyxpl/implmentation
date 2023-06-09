#include "DHT.h"

DHT dht(28, DHT11);

void setup() {
dht.begin();
Serial.begin(9600);
}

void loop() {
// put your main code here, to run repeatedly
float t = dht.readTemperature();
float h = dht.readHumidity();
Serial.print("Temperature : ");
Serial.println(t);
Serial.print("Humidite : ");
Serial.println(h);
delay(1000);
}