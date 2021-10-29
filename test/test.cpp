/* Codigo para comprobar que pines son los de la placa
    - ESP8266EX:
        - 4 interfaces de salida PWM:
            - Nombre de pin: MTDI, MTDO, MTMS, GPIO4
            - Pin num: 10, 13, 9, 16
            - IO: IO12, IO15, IO14, IO4
            - Function Name: PWM0, PWM1, PWM2, PWM3
*/

#include <Arduino.h>

#define PWM0 12
#define PWM1 15
#define PWM2 14
#define PWM3 4

void setup() {
    Serial.begin(9600); // Puede que sea 115200

    analogWriteRange(255);  // Configuracion PWM

    pinMode(PWM0, OUTPUT);    //PWM0
    pinMode(PWM1, OUTPUT);    //PWM1
    pinMode(PWM2, OUTPUT);    //PWM2
    pinMode(PWM3, OUTPUT);     //PWM3
}

void loop() {
    // Enciende cada pin 2 segundos
    analogWrite(PWM3, 0);
    analogWrite(PWM0, 255);
    Serial.println("PWM0 activo. IO12.");
    delay(2000);
    analogWrite(PWM0, 0);
    analogWrite(PWM1, 255);
    Serial.println("PWM1 activo. IO15.");
    delay(2000);
    analogWrite(PWM1, 0);
    analogWrite(PWM2, 255);
    Serial.println("PWM2 activo. IO14.");
    delay(2000);
    analogWrite(PWM2, 0);
    analogWrite(PWM3, 255);
    Serial.println("PWM3 activo. IO4.");
    delay(2000);
}