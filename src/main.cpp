#include <Arduino.h>
#include <U8x8lib.h>
#include "pwmtables.h"

// FSM states
enum State
{ IDLE,
  COVER_OPEN,
  STARTING,
  RUNNING,
  STOPPING,
  PLAY_SOUND
};

// IO Pins
#define ENCODER_A     2   // int0
#define ENCODER_B     3   // int1
#define DOOR_CLOSED   4   // din
#define MOTOR_ON      10  // pwm out
#define UV_LIGHT_ON   11  // pwm out
#define START_BUTTON  7   // din
#define STOP_BUTTON   8   // din
#define BUZZER        9   // dout

U8X8_SH1106_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);

#define MAX_TIME      (3600)      // no of seconds maximum time
#define VERSION       "v1.1"      // Software version
#define TIMEADJ       (-15)       // adjustment of timing loop (ms)
//#define DEBUG                     // uncomment for debugging

// DEBUG inverts the polarity of the door switch so motor can be running
// without the cover.
#ifdef DEBUG
#define DOOR_SWITCH (!digitalRead(DOOR_CLOSED))
#else
#define DOOR_SWITCH (digitalRead(DOOR_CLOSED))
#endif

volatile int step = 0;
volatile int16_t seconds = 0;
int16_t buf_sec = 0;
int time_changed = 0;
int count = 0;
int sound = 0;
char string[16] = "";
char status_line[16] = "Timer Stopped";
unsigned long elapsed = 0;
unsigned long lasttime = millis();
int state = IDLE;

// Interrupt service routines for encoder
void doEncoderA() {
  // look for a low-to-high on channel A
  if (digitalRead(ENCODER_A) == HIGH) {
    // check channel B to see which way encoder is turning
    if (digitalRead(ENCODER_B) == LOW) {
      seconds += step;         // CW
    }
    else {
      seconds -= step;         // CCW
    }
  }
  else   // must be a high-to-low edge on channel A
  {
    // check channel B to see which way encoder is turning
    if (digitalRead(ENCODER_B) == HIGH) {
      seconds += step;          // CW
    }
    else {
      seconds -= step;          // CCW
    }
  }
}

void doEncoderB() {
  // look for a low-to-high on channel B
  if (digitalRead(ENCODER_B) == HIGH) {
    // check channel A to see which way encoder is turning
    if (digitalRead(ENCODER_A) == HIGH) {
      seconds += step;         // CW
    }
    else {
      seconds -= step;         // CCW
    }
  }   // Look for a high-to-low on channel B
  else {
    // check channel B to see which way encoder is turning
    if (digitalRead(ENCODER_A) == LOW) {
      seconds += step;          // CW
    }
    else {
      seconds -= step;          // CCW
    }
  }
}

// Play sound to notify user
void play_sound(void)
{
  int i;
  for (i=0 ; i < 4 ; i++){
    tone(BUZZER, 880+(i*440));
    delay(400);
    noTone(BUZZER);
    delay(100);
  }
}

// Update timer MM:SS, ramp status and SW version
void update_display(void)
{
  sprintf(string, "%02d:%02d", (buf_sec / 60), (buf_sec % 60));
  u8x8.setFont(u8x8_font_courB24_3x4_r);
  u8x8.drawString(0, 2, string);

  u8x8.setFont(u8x8_font_chroma48medium8_r);
  sprintf(string, "Ramp:%3d%c", count*2, '%');
  u8x8.drawString(0, 7, string);
  u8x8.drawString(11, 7, VERSION);

  u8x8.setFont(u8x8_font_chroma48medium8_r);
  u8x8.drawString(0, 0, status_line);
}

void setup() {
  pinMode(STOP_BUTTON, INPUT);
  pinMode(START_BUTTON, INPUT);
  pinMode(ENCODER_A, INPUT);
  pinMode(ENCODER_B, INPUT);
  pinMode(MOTOR_ON, OUTPUT);
  pinMode(UV_LIGHT_ON, OUTPUT);

  // setup ISRs for encoder
  attachInterrupt(digitalPinToInterrupt(ENCODER_A), doEncoderA, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENCODER_B), doEncoderB, CHANGE);

  // setup OLED display
  u8x8.begin();
}

void loop() {
  // Timing for seconds countdown
  elapsed = millis();
  if ((elapsed - lasttime) > 1000+TIMEADJ)
  {
    lasttime = elapsed;
    if (state == RUNNING)
    {
      seconds--;
      time_changed = 1;
      if (seconds == 0)
      {
        sound = 1;
      }
    }
  }

  noInterrupts();     // Start critical section
  if (seconds >= 60)
  {
    if (seconds >= 300)
    {
      if (seconds >= 600)
      {
        step = 60;
      }
      else
      {
        step = 30;
      }
    }
    else
    {
      step = 10;
    }
  }
  else
  {
    step = 5;
  }

  if (seconds > MAX_TIME)
  {
    seconds = MAX_TIME;
  }
  if (seconds < 0)
  {
    seconds = 0;
  }
  buf_sec = seconds;
  interrupts();     // End critical section

  // Finite State Machine for implementing the Timer function
  switch (state)
  {
    case IDLE:
      if (digitalRead(START_BUTTON) && (buf_sec > 0))
      {
        state = STARTING;
      }
      if (DOOR_SWITCH)
      {
        state = COVER_OPEN;
      }
      if (sound)
      {
        state = PLAY_SOUND;
      }
    break;

    case COVER_OPEN:
      sprintf(status_line, "  NO COVER!! ");
      if (!DOOR_SWITCH)
      {
        sprintf(status_line, "Timer Stopped");
        state = IDLE;
      }
    break;

    case STARTING:
      if (count < getTableRange())
      {
        analogWrite(MOTOR_ON, getDcMotorTable(count));
        analogWrite(UV_LIGHT_ON, getUVLedTable(count));
        count++;
      }
      else
      {
        state = RUNNING;
      }
      sprintf(status_line, "Timer Running");
    break;

    case RUNNING:
      if (digitalRead(STOP_BUTTON) || (buf_sec == 0) || DOOR_SWITCH)
      {
        state = STOPPING;
      }
    break;

    case STOPPING:
      if (count > 0)
      {
        count--;
        analogWrite(MOTOR_ON, getDcMotorTable(count));
        analogWrite(UV_LIGHT_ON, getUVLedTable(count));
      }
      else
      {
        if (DOOR_SWITCH)
        {
          state = COVER_OPEN;
        }
        else
        {
          state = IDLE;
          seconds = 0;
          sprintf(status_line, "Timer Stopped");
        }
      }
    break;

    case PLAY_SOUND:
      sprintf(status_line, "Timer Expired");
      play_sound();
      sound = 0;
      state = IDLE;
    break;

    default:
    break;
  }

  // Update display. Frequency depends on the current state
  // to avoid unneccesary I2C communication
  switch (state)
  {
    case RUNNING:
      if (time_changed)
      {
        update_display();
        time_changed = 0;
      }
      break;

    case STARTING:
    case STOPPING:
    if ((millis() % 100) < 10)
    {
      update_display();
    }
    break;

    case IDLE:
    case COVER_OPEN:
    case PLAY_SOUND:
      update_display();
    break;

    default:
    break;
  }

  delay(5);
}
