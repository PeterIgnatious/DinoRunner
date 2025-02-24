/*

Endereço I2C: 0x27;
LCD 16x2;
SDA: A4;
SCL: A5;
Botão: Digital Pin 2;

*/



#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <EEPROM.h>

int COLS = 16;
int ROWS = 2;


void EEPROMWriteInt(int address, int value); // 2 Bytes
int  EEPROMReadInt(int address);


LiquidCrystal_I2C lcd(0x27, COLS, ROWS);

uint8_t dino_front_1[] = {0x00, 0x1C, 0x16, 0x1E, 0x10, 0x18, 0x10, 0x18};
uint8_t dino_back_1[] = {0x00, 0x00, 0x00, 0x01, 0x17, 0x0F, 0x02, 0x04};
uint8_t dino_front_2[] = {0x00, 0x1C, 0x16, 0x1E, 0x10, 0x18, 0x00, 0x00};
uint8_t dino_back_2[] = {0x00, 0x00, 0x00, 0x01, 0x07, 0x1F, 0x02, 0x03};
uint8_t cactus[] = {0x05, 0x05, 0x17, 0x14, 0x1C, 0x04, 0x04, 0x04};
uint8_t dino_stand_tail_1[] = {0x01, 0x03, 0x01, 0x01, 0x03, 0x01, 0x17, 0x0F};
uint8_t dino_stand_tail_2[] = {0x01, 0x03, 0x01, 0x01, 0x03, 0x11, 0x0B, 0x0F};
uint8_t dino_stand_front[] = {0x18, 0x0C, 0x1C, 0x10, 0x18, 0x10, 0x14, 0x1C};

int button = 2;
unsigned long last_time_changed = millis();
unsigned long debounce_duration = 1000;
int cactus_positions[8] = {"", "", "", "", "", "", "", ""};

long highscore = EEPROMReadInt(1);

bool control_steps = false;
bool button_pressed = false;
bool initial_click = true;
bool running = false;
bool death = true;

int screen = 0;
int score = 0;
long start_timer = 0;
long timer_steps = 0;
long timer_cactus = 0;
long timer_spawn = 2000;
long timer_jump = 0;
long timer_points = 0;
long timer_scroll = 0;
long timer_status = 0;
long delay_steps = 200;
long delay_move = 200;
long delay_spawn = 2000;
long delay_jump = 1000;
long delay_points = 100;
long delay_scroll = 200;
long delay_status = 1000;

/* 

Início

int screen = 0;
int score = 0;
long start_timer = 0;
long timer_steps = 0;
long timer_cactus = 0;
long timer_spawn = 2000;
long timer_jump = 0;
long timer_points = 0;
long timer_scroll = 0;
long timer_status = 0;
long delay_steps = 200;
long delay_move = 200;
long delay_spawn = 2000;
long delay_jump = 1000;
long delay_points = 100;
long delay_scroll = 200;
long delay_status = 1000;

Fim

int screen = 0;
int score = 0;
int start_timer = 0;
int timer_steps = 0;
int timer_cactus = 0;
int timer_spawn = 2000;
int timer_jump = 0;
int timer_points = 0;
int timer_scroll = 0;
int delay_steps = 100;
int delay_move = 75;
int delay_spawn = 1100;
int delay_jump = 1000;
int delay_points = 50;
int delay_scroll = 200;


*/


void EEPROMWriteInt(int address, int value) {
   byte hiByte = highByte(value);
   byte loByte = lowByte(value);

   EEPROM.write(address, hiByte);
   EEPROM.write(address + 1, loByte);   
}


int EEPROMReadInt(int address) {
   byte hiByte = EEPROM.read(address);
   byte loByte = EEPROM.read(address + 1);
   
   return word(hiByte, loByte); 
}


void update_status() 
{
  if (delay_steps > 100) {
    delay_steps--;
  }
  if (delay_move > 75) {
    delay_move--;
  }
  if (delay_spawn > 1100) {
    delay_spawn = delay_spawn - 10;
  }
  if (delay_points > 50) {
    delay_points--;
  }
}


bool steps_dino(bool control_steps, int timer, int delay_steps)
{
  if (screen == 0) {
    lcd.setCursor(2, 1);
    if (control_steps) {
      lcd.write(0);
    } else {
      lcd.write(1);
    }
  } else if (screen == 1) {
    if (control_steps) {
      lcd.setCursor(2, 1);
      lcd.write(1);
      lcd.setCursor(3, 1);
      lcd.write(0);

    } else {
      lcd.setCursor(2, 1);
      lcd.write(3);
      lcd.setCursor(3, 1);
      lcd.write(2);
    }
  }
  
  return not control_steps;

}


void spawn_cactus(int cactus_positions[])
{
  int randnum = random(0, 101);
  
  if (randnum < 40) {
    for (int i = 0; i < 8; i++){
      if (cactus_positions[i] == ""){
        cactus_positions[i] = 15;
        break;
      }
    }
  } 

  if (randnum >= 40 && randnum < 70) {
    int control = 0;
    for (int i = 0; i < 8; i++) {
      if (cactus_positions[i] == "") {
        cactus_positions[i] = 15 + control;
        control++;
      }
      
      if (control == 2) {
        break;
      }
    }
  }

  if (randnum >= 70 && randnum <= 80) {
    int control = 0;
    for (int i = 0; i < 8; i++) {
      if (cactus_positions[i] == "") {
        cactus_positions[i] = 15 + control;
        control++;
      }
      
      if (control == 3) {
        break;
      }
    }
  }
}


void move_cactus(int cactus_positions[])
{ 
  for (int i = 0; i < 8; i++){
    int position = cactus_positions[i];

    if (position >= -1 && position != ""){
      lcd.setCursor(position, 1);
      lcd.write(4);
      lcd.setCursor(position + 1, 1);
      lcd.print(" ");
      cactus_positions[i]--;
    }
    else{
      cactus_positions[i] = "";
    }

    if (position == 2 || position == 3 || position == 1) {
      death = check_colision();
    }
  }
}


bool check_colision()
{
  if (running) {
    return true;
  }
  else {
    return false;
  }
}


void dino_jump(int timer, bool falling)
{
  if (falling) {
    lcd.setCursor(2, 0);
    lcd.print("  ");
    lcd.setCursor(2, 1);
    lcd.write(1);
    lcd.setCursor(3, 1);
    lcd.write(0);
  } else {
    lcd.setCursor(2, 1);
    lcd.print("  ");
    lcd.setCursor(2, 0);
    lcd.write(1);
    lcd.setCursor(3, 0);
    lcd.write(0);
  }
}


void reset_dino()
{
  score = 0;
  timer_steps = 0;
  timer_cactus = 0;
  timer_spawn = 2000;
  timer_jump = 0;
  timer_points = 0;
  timer_scroll = 0;
  timer_status = 0;
  delay_steps = 200;
  delay_move = 200;
  delay_spawn = 2000;
  delay_jump = 1000;
  delay_points = 100;
  delay_scroll = 200;
}


void display_score()
{
  score++;
  lcd.setCursor(12, 0);
  lcd.print(score);
}


String spacing_message(String message)
{
  for (int i = 0; i < COLS; i++) {
    message = " " + message;
  }
  message = message + "  ";
  return message;
}


void scroll_message(String message, int position)
{
  lcd.setCursor(0, 0);
  lcd.print(message.substring(position, position + COLS));
}

void intro()
{
  for (int i = 0; i <= 15; i++) {
    lcd.setCursor(15-i, 0);
    lcd.write(255);
    lcd.setCursor(15-i, 1);
    lcd.write(255);
    delay(50);
  }
  for (int i = 0; i <= 15; i++) {
    lcd.setCursor(15-i, 0);
    lcd.print(" ");
    lcd.setCursor(15-i, 1);
    lcd.print(" ");
    delay(50);
  }
}


void setup()
{
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  lcd.createChar(0, dino_stand_tail_1);
  lcd.createChar(1, dino_stand_tail_2);
  lcd.createChar(2, dino_stand_front);
  pinMode(button, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(button), handle_interrupt, FALLING);
}


void loop()
{
  start_timer = millis();
  unsigned long timer;
  int position = 0;
  button_pressed = false;

  String message = "Dino Runner";
  message = spacing_message(message);

  while (screen == 0) {
    timer = millis() - start_timer;
    lcd.setCursor(3, 1);
    lcd.write(2);

    if (timer >= timer_scroll) {
      scroll_message(message, position);
      timer_scroll = timer + delay_scroll;
      position++;
    }

    if (timer > timer_steps) {
      control_steps = steps_dino(control_steps, timer, delay_steps);
      timer_steps = timer + delay_steps;
    }

    if (button_pressed) {
      button_pressed = false;
      screen = 1;
      death = false;
      intro();
    }

    if (position > message.length()) {
      position = 0;
    }
  }

  if (screen == 1) {
    lcd.createChar(0, dino_front_1);
    lcd.createChar(1, dino_back_1);
    lcd.createChar(2, dino_front_2);
    lcd.createChar(3, dino_back_2);
    lcd.createChar(4, cactus);
    lcd.clear();
    timer_steps = 0;
    start_timer = millis();

    while (not death) {

      timer = millis() - start_timer;

      if (timer >= timer_status) {
        update_status();
        timer_status = timer + delay_status;
      }

      if (timer > timer_steps && running) {
        control_steps = steps_dino(control_steps, timer, delay_steps);
        timer_steps = timer + delay_steps;
      }
      if (timer > timer_cactus)
      {
        move_cactus(cactus_positions);
        timer_cactus = timer + delay_move;
      }
      if (timer > timer_spawn)
      {
        spawn_cactus(cactus_positions);
        timer_spawn = timer + delay_spawn;
      }
      if (timer > timer_jump && running == false) {
        dino_jump(timer, true);
        running = true;
      }
      if (timer > timer_points) {
        display_score();
        timer_points = timer + delay_points;
      }

      if (button_pressed){
        button_pressed = false;
        running = false;
        tone(8, 269, 100);
        dino_jump(timer, false);
        delay(50);
        noTone(8);
        timer_jump = timer + delay_jump;
      }
      delay(10);
    }

    lcd.createChar(0, dino_stand_tail_1);
    lcd.createChar(1, dino_stand_tail_2);
    lcd.createChar(2, dino_stand_front);

    lcd.setCursor(2, 1);
    lcd.write(0);
    lcd.setCursor(3, 1);
    lcd.write(2);
    lcd.setCursor(0, 0);
    lcd.print("      ");

    delay(1000);
    lcd.clear();
    lcd.print("SCORE: ");
    lcd.print(score);
    lcd.setCursor(0, 1);

    if (score > highscore) {
      lcd.print("Novo HIGHSCORE!");
      EEPROMWriteInt(1, score);
      highscore = EEPROMReadInt(1);
    } else {
      lcd.print("HIGHSCORE: ");
      lcd.print(highscore);
    }

    delay(2000);
    screen = 0;
    reset_dino();
    lcd.clear();

    for (int i = 0; i < 5; i++)
    {
      cactus_positions[i] = "";
    }
  }
}


void handle_interrupt()
{
  if (not initial_click) {
    if (millis() - last_time_changed >= debounce_duration) {
      last_time_changed = millis();
      button_pressed = true;
    }
  }

  initial_click = false;
}




