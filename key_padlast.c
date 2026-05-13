#include <reg51.h>              // 8051 header file

// LCD control pins
sbit RS = P2^1;                 // RS connected to P2.1
sbit EN = P2^2;                 // EN connected to P2.2
#define LCD_DATA P1             // LCD data bus on Port 1

// Keypad row connections
sbit R1 = P3^0;                 
sbit R2 = P3^1;
sbit R3 = P3^2;
sbit R4 = P3^3;

// Keypad column connections
sbit C1 = P3^4;
sbit C2 = P3^5;
sbit C3 = P3^6;
sbit C4 = P3^7;

// Global variables
unsigned int num1 = 0, num2 = 0; // Store input numbers
unsigned char result = 0;        // Store result

char operation = 0;              // Store operator
char input_stage = 0;            // 0=num1, 1=num2

bit error_flag = 0;              // Error status flag

// Delay function
void delay(unsigned int ms) {

    unsigned int i, j;           // Loop counters

    for(i = 0; i < ms; i++)      // Outer loop
        for(j = 0; j < 1275; j++); // Inner delay loop
}

// Send command to LCD
void lcd_cmd(unsigned char cmd) {

    RS = 0;                      // Select command register
    LCD_DATA = cmd;              // Put command on bus

    EN = 1;                      // Enable HIGH
    delay(1);                    // Small delay

    EN = 0;                      // Enable LOW
    delay(5);                    // Wait for execution
}

// Send data to LCD
void lcd_data(unsigned char dat) {

    RS = 1;                      // Select data register
    LCD_DATA = dat;              // Put data on bus

    EN = 1;                      // Enable HIGH
    delay(1);                    // Small delay

    EN = 0;                      // Enable LOW
    delay(5);                    // Wait for execution
}

// Initialize LCD
void lcd_init() {

    delay(20);                   // LCD startup delay

    lcd_cmd(0x38);               // 8-bit, 2-line mode
    lcd_cmd(0x0C);               // Display ON
    lcd_cmd(0x06);               // Cursor increment
    lcd_cmd(0x01);               // Clear display

    delay(2);                    // Wait after clear
}

// Display string on LCD
void lcd_string(char *str) {

    while(*str)                  // Until null character
        lcd_data(*str++);        // Display character
}

// Display number on LCD
void lcd_number(unsigned int num) {

    unsigned char digits[5];     // Digit array
    char i = 0;                  // Index variable

    if(num == 0) {               // Special case for 0
        lcd_data('0');           // Display 0
        return;                  // Exit function
    }

    while(num > 0) {             // Extract digits

        digits[i++] = (num % 10) + '0'; // Convert to ASCII
        num /= 10;               // Remove last digit
    }

    while(i > 0)                 // Print digits
        lcd_data(digits[--i]);   // Display in order
}

// Show overflow message
void show_overflow() {

    lcd_cmd(0x01);               // Clear LCD
    lcd_string("OVERFLOW");      // Display message
    error_flag = 1;              // Set error flag
}

// Perform calculation
void calculate() {

    signed int temp = 0;         // Temporary result

    switch(operation) {          // Check operator

        case '+':
            temp = num1 + num2;  // Addition
            break;

        case '-':
            temp = num1 - num2;  // Subtraction
            break;

        case '*':
            temp = num1 * num2;  // Multiplication
            break;

        case '/':

            if(num2 == 0) {      // Divide by zero check

                lcd_cmd(0x01);   // Clear LCD
                lcd_string("ERROR"); // Show error

                error_flag = 1;  // Set error flag
                return;          // Exit function
            }

            temp = num1 / num2;  // Division
            break;
    }

    if(temp > 255 || temp < 0) { // Result range check

        show_overflow();         // Display overflow
    }
    else {

        result = (unsigned char)temp; // Store result
    }
}


// Keypad layout
char keypad[4][4] = {
    {'7','8','9','/'},
    {'4','5','6','*'},
    {'1','2','3','-'},
    {'C','0','=','+'}
};

// Read keypad function
unsigned char read_keypad() {

    unsigned char row, col;

    // Set all rows HIGH initially
    R1 = R2 = R3 = R4 = 1;

    // Scan rows one by one
    for(row = 0; row < 4; row++) {

        // Make all rows HIGH
        R1 = R2 = R3 = R4 = 1;

        // Make only current row LOW
        switch(row) {

            case 0: R1 = 0; break;
            case 1: R2 = 0; break;
            case 2: R3 = 0; break;
            case 3: R4 = 0; break;
        }

        // Check each column
        if(C1 == 0) {

            delay(20);              // debounce

            while(C1 == 0);         // wait release

            return keypad[row][0];
        }

        if(C2 == 0) {

            delay(20);

            while(C2 == 0);

            return keypad[row][1];
        }

        if(C3 == 0) {

            delay(20);

            while(C3 == 0);

            return keypad[row][2];
        }

        if(C4 == 0) {

            delay(20);

            while(C4 == 0);

            return keypad[row][3];
        }
    }

    return 0;   // No key pressed
}

// Main program
void main() {

    unsigned char key;          // Store pressed key

    lcd_init();                 // Initialize LCD

    while(1) {                  // Infinite loop

        key = read_keypad();    // Read keypad

        if(key != 0) {          // If key detected

            if(key == 'C') {    // Clear operation

                num1 = 0;       // Reset num1
                num2 = 0;       // Reset num2
                result = 0;     // Reset result

                operation = 0;  // Reset operator
                input_stage = 0;// Reset stage
                error_flag = 0; // Clear error

                lcd_cmd(0x01);  // Clear LCD
            }

            else if(error_flag == 0) { // Continue if no error

                if(key >= '0' && key <= '9') { // Number input

                    lcd_data(key); // Display key

                    if(input_stage == 0) { // First number

                        num1 = (num1 * 10) + (key - '0');

                        if(num1 > 255) { // Overflow check
                            show_overflow();
                        }
                    }
                    else { // Second number

                        num2 = (num2 * 10) + (key - '0');

                        if(num2 > 255) { // Overflow check
                            show_overflow();
                        }
                    }
                }

                else if(key == '+' || key == '-' ||
                        key == '*' || key == '/') {

                    operation = key; // Store operator
                    input_stage = 1; // Move to num2

                    lcd_data(key);   // Display operator
                }

                else if(key == '=') { // Equal pressed

                    calculate();      // Perform operation

                    if(!error_flag) {

                        lcd_cmd(0x01); // Clear LCD
                        lcd_number(result); // Show result
                    }
                }
            }

            delay(20); // Debounce delay
        }
    }
}