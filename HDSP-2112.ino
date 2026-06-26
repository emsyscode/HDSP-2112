// Dedicated control pin definitions
const int pin_WR  = 5;
const int pin_CLK = 6;
const int pin_CLS = 7;
const int pin_FL  = 8;
const int pin_RST = 9;
const int pin_RD  = 10;
const int pin_CE  = 11;
const int pin_LED = 12;

// Digit addresses on the HDSP-2112 (A4=1 to access characters)
// On the bus connected to PORTC (PC0=A0, PC1=A1, PC2=A2, PC3=A3, PC4=A4)
// Digit 0 is on the far left (end of the string) or right depending on the physical mapping, 
// typically mapped from 0x10 (Digit 0) to 0x17 (Digit 7).
const uint8_t DIGIT_ADDR[8] = {0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F};


// Generic function to write data/commands to the bus
void hdsp_write(uint8_t address, uint8_t data) {
  // Sets the address on PORTC (keeping PC6 and PC7 bits intact)
  PORTC = (PORTC & 0xC0) | (address & 0x1F);
  
  // Places the data on PORTA
  PORTA = data;
  
  // Write pulse (Activates Chip Enable and Write)
  digitalWrite(pin_CE, LOW);
  delayMicroseconds(10);
  digitalWrite(pin_WR, LOW);
  delayMicroseconds(10); // Ensures minimum setup times from the datasheet (T_w)
  digitalWrite(pin_WR, HIGH);
  delayMicroseconds(10);
  digitalWrite(pin_CE, HIGH);
  delayMicroseconds(10);
}

// Writes to the control register (A4 = 0)
void hdsp_write_cmd(uint8_t reg, uint8_t cmd) {
  hdsp_write(reg & 0x0F, cmd); 
}

// Writes a character to a specific digit (A4 = 1, meaning address + 0x10)
void hdsp_write_char(uint8_t digit, char c) {
  if (digit < 8) {
    hdsp_write(DIGIT_ADDR[digit], (uint8_t)c);
  }
}

// Sends a string of up to 8 characters to the display
void writeString(const char* str) {
  for (int i = 0; i < 8; i++) {
    if (str[i] == '\0') break;
    hdsp_write_char(i, str[i]);
  }
}

// Clears all digits by sending the ASCII space character (0x20)
void clearDisplay() {
  for (int i = 0; i < 8; i++) {
    hdsp_write_char(i, ' ');
  }
}

void setup() {
  // 1. Configure Ports as Outputs via DDR (Data Direction Register)
  DDRA = 0xFF; // PA0~PA7 (Pins 22-29) as outputs
  PORTA = 0x7F; // Bit 7 defines when equal to 0, the 128 ASCII CODE
  DDRC |= 0x1F; // PC0~PC5 (Pins 37-32) as outputs (mask 0b00111111)

  // 2. Configure individual control pins
  pinMode(pin_WR, OUTPUT);
  pinMode(pin_CLK, OUTPUT);
  pinMode(pin_CLS, OUTPUT);
  pinMode(pin_FL, OUTPUT);
  pinMode(pin_RST, OUTPUT);
  pinMode(pin_RD, OUTPUT);
  pinMode(pin_CE, OUTPUT);
  pinMode(pin_LED, OUTPUT); // This LED is only to debug, is not necessary use it!

  // Safe initial state (Active LOW signals start HIGH)
  digitalWrite(pin_WR, HIGH);
  digitalWrite(pin_RD, HIGH);
  digitalWrite(pin_CE, HIGH);
  digitalWrite(pin_FL, HIGH);
  
  // Use internal oscillator (CLS = LOW). If using external, CLS = HIGH
  digitalWrite(pin_CLS, HIGH); //Used to select either an internal (CLS = 1) or external (CLS = 0) clock source. 
  digitalWrite(pin_CLK, HIGH); //Outputs the master clock (CLS = 1) or inputs a clock (CLS = 0) for slave 
                               //displays. //This will be a output pin if CLS=1 !!!!

  // 3. HDSP-2112 Hardware Reset Sequence
  digitalWrite(pin_RST, LOW);
  delay(10);
  digitalWrite(pin_RST, HIGH);
  delay(10);

  // 4. Initialize the Display (Write to the Control Word)
  // To access the Control Word: A4=0, A3=0, A2=0, A1=0, A0=0 -> PORTC = 0x00
  // Control data: e.g., 0x04 for maximum brightness, no flashing (check datasheet table)
  hdsp_write_cmd(0x00, 0x04); 

  // Clear the display by writing blank spaces
  clearDisplay();
  
  // Write a test message
  writeString("HELLO ");
  delay(750);
}

void loop() {
  // The loop can remain empty to keep the text static
  writeString("Hi Folks");
  delay(1500);
    for(uint8_t i=0; i < 4; i++){
      writeString("HDSP2112");
      delay(800);
      clearDisplay();
      delay(300);
    }
  writeString("12345678");
  delay(1000);
  writeString("87654321");
  // delay(1000);
  // digitalWrite(pin_LED, LOW); //I use this LED only to debug, is not necessary be implemented!
  // delay(100);
  // digitalWrite(pin_LED, HIGH);
}
