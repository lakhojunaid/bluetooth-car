#include <Bluepad32.h>

GamepadPtr myGamepad = nullptr;

// motor pins
const int LEFT_IN1  = 16;
const int LEFT_IN2  = 17;
const int RIGHT_IN1 = 18;
const int RIGHT_IN2 = 19;
const int ENA = 22;   // enable pin for left motor driver channel
const int ENB = 23;   // enable pin for right motor driver channel


void onGamepadConnected(GamepadPtr gp) {
    Serial.println("Gamepad connected!");
    myGamepad = gp;
}

void onGamepadDisconnected(GamepadPtr gp) {
    Serial.println("Gamepad disconnected!");
    if (myGamepad == gp) myGamepad = nullptr;
}

void setup() {
    Serial.begin(115200);
    Serial.println("Starting ESP32 + PS4 car...");

    pinMode(LEFT_IN1, OUTPUT);
    pinMode(LEFT_IN2, OUTPUT);
    pinMode(RIGHT_IN1, OUTPUT);
    pinMode(RIGHT_IN2, OUTPUT);

    pinMode(ENA, OUTPUT);
    pinMode(ENB, OUTPUT);
    digitalWrite(ENA, HIGH);
    digitalWrite(ENB, HIGH);   // enable both motor channels


    BP32.setup(&onGamepadConnected, &onGamepadDisconnected);
    BP32.forgetBluetoothKeys();
}

void setMotor(int in1, int in2, int speed) {
    speed = constrain(speed, -255, 255);

    if (speed > 0) {
        analogWrite(in1, speed);
        analogWrite(in2, 0);
    }
    else if (speed < 0) {
        analogWrite(in1, 0);
        analogWrite(in2, -speed);
    }
    else {
        analogWrite(in1, 0);
        analogWrite(in2, 0);
    }
}

void loop() {
    BP32.update();

    if (myGamepad && myGamepad->isConnected()) {

        // PS4 triggers
        int accelRaw = myGamepad->throttle(); // R2
        int brakeRaw = myGamepad->brake();    // L2

        // Steering (left stick X)
        int steerRaw = myGamepad->axisX();

        // Debug prints
        Serial.print("R2: ");
        Serial.print(accelRaw);
        Serial.print(" | L2: ");
        Serial.print(brakeRaw);
        Serial.print(" | LX: ");
        Serial.print(steerRaw);

        // Convert trigger values to motor power
        int accel = map(accelRaw, 0, 1023, 0, 255);
        int brake = map(brakeRaw, 0, 1023, 0, 255);

        // Base speed forward/backward
        int baseSpeed = accel - brake;    // forward - backward

        // Steering amount
        int turn = map(steerRaw, -512, 512, 128, -128);

        // Final motor speeds
        int leftSpeed  = baseSpeed + turn;
        int rightSpeed = baseSpeed - turn;

        // Debug printing
        Serial.print(" | LSpeed: ");
        Serial.print(leftSpeed);
        Serial.print(" | RSpeed: ");
        Serial.println(rightSpeed);

        // Set motors
        setMotor(LEFT_IN1, LEFT_IN2, leftSpeed);
        setMotor(RIGHT_IN1, RIGHT_IN2, rightSpeed);
    }

    delay(100);
}
