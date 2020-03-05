#include<avr/io.h>
#include<util/delay.h>
 
void init_lcd();
void cmd_4bit(unsigned char cmd);
void data_4bit(unsigned char x);
void update_number_status(int num);
void calculate_number();
void display_result();
bool select_choice();
void rotate_servo(int Angle);

signed int r1=0;  //Number 1
signed int r2=0;  //Number 2
signed int r3=0;  //Number 3- A+/-B=C, A=C
int AF=0; //Arithmetic flag, indicating +, -, *, or /
 
int main()
{
  init_lcd();
 
  while (1)
  {
    update_lcd();
   take_input();
     
  }
 
  return 0;
}
 
void init_lcd()
{
 
  DDRB = 0xff; 
  cmd_4bit(0x02); // return home
  cmd_4bit(0x28); // 4 bit mode
  cmd_4bit(0x0f); // display on, cursor blinking
  cmd_4bit(0x06); // inreament cusror
  cmd_4bit(0x01); // clear screen
  cmd_4bit(0x80); // cursor begins in the first line
 
}
 
void update_lcd()
{
  cmd_4bit(0x28);
  cmd_4bit(0x0f);
}
 
void cmd_4bit(unsigned char cmd)
{
  unsigned char a, b;
  a = cmd & 0xf0;   //MSB 4-bits
  a = a >> 2;
  a |= 0x02;
  PORTB = a;
  _delay_ms(10);
  PORTB &= ~ 0x02;
 
  b = cmd << 4;     //LSB 4-bits
  b = b >> 2;
  b |= 0x02;
  PORTB = b;
  _delay_ms(10);
  PORTB &= ~ 0x02;
 
}
 
void data_4bit(unsigned char x)
{
  unsigned char a, b;
  a = x & 0xf0;
  a = a >> 2;
  a |= 0x03;
  PORTB = a;
  _delay_ms(10);
 
  PORTB &= ~ 0x02;
 
  b = x << 4;
  b = b >> 2;
  b |= 0x03;
  PORTB = b;
  _delay_ms(10);
  PORTB &= ~ 0x02;
 
}
 
void take_input()
{
 
  DDRD = 0xff;
  PORTD = 0x00;
  DDRD = 0x0f;

  PORTD = 0b00000001; // row 1
  
  if(bit_is_set(PIND, 4))
  {
    data_4bit('1');
    update_number_status(1);
    _delay_ms(300);
    return;
  }
  else if(bit_is_set(PIND, 5))
  {
    data_4bit('2');
    update_number_status(2);
    _delay_ms(300);
    return;
  }
  else if(bit_is_set(PIND, 6))
  {
    data_4bit('3');
    update_number_status(3);
    _delay_ms(300);
    return;
  }
  else if(bit_is_set(PIND, 7))
  {
    // multiply
    AF = 3;
    calculate_number();
    data_4bit('x');
    _delay_ms(300);
    return;
  }

  PORTD = 0b00000010; //row 2
  
  if(bit_is_set(PIND, 4))
  {
    data_4bit('4');
    update_number_status(4);
    _delay_ms(300);
    return;
  }
  else if(bit_is_set(PIND, 5))
  {
    data_4bit('5');
    update_number_status(5);
    _delay_ms(300);
    return;
  }
  else if(bit_is_set(PIND, 6))
  {
    data_4bit('6');
    update_number_status(6);
    _delay_ms(300);
    return;
  }
  else if(bit_is_set(PIND, 7))
  {
    // divide
    AF = 4;
    calculate_number();
    data_4bit('/');
    _delay_ms(300);
    return;
  }

  PORTD = 0b00000100;
  if(bit_is_set(PIND, 4))
  {
    data_4bit('7');
    update_number_status(7);
    _delay_ms(300);
    return;
  }
  else if(bit_is_set(PIND, 5))
  {
    data_4bit('8');
    update_number_status(8);
    _delay_ms(300);
    return;
  }
  else if(bit_is_set(PIND, 6))
  {
    data_4bit('9');
    update_number_status(9);
    _delay_ms(300);
    return;
  }
  else if(bit_is_set(PIND, 7))
  {
    // add
    AF = 1;
    calculate_number();
    data_4bit('+');
    _delay_ms(300);
    return;
  }
  
  PORTD = 0b00001000;
  if(bit_is_set(PIND, 4))
  {
    //clear or rotate
    init_lcd();
    update_lcd();
    r3 = r2 = r1 = 0;
    _delay_ms(300);
    return;
  }

  else if(bit_is_set(PIND, 5))
  {
    data_4bit('0');
    update_number_status(0);
    _delay_ms(300);
    return;
  }
  else if(bit_is_set(PIND, 6))
  {
    //equals
    calculate_number();
    display_result();
    AF = 0;
   
    _delay_ms(500);
    
    char quest[] = "    Rotate?";
    for(int i = 0;i < sizeof(quest)-1;++i)
      data_4bit(quest[i]);
      
     check : if(bit_is_set(PIND, 7)){
       rotate_servo(r3);
        r1 = r3;
        r2 = r1 = 0;
       init_lcd();
       return;
     }
       
      else
      {
        if(bit_is_set(PIND, 4))
        {
          _delay_ms(500);
          return;
        }
        goto check;
      }
          
    cmd_4bit(0x80);
    
    _delay_ms(300);
    return;
  }
  else if(bit_is_set(PIND, 7))
  {
    //subtract
    AF = 2;
    calculate_number();
    data_4bit('-');
    _delay_ms(300);
    return;
  }
  
}

void update_number_status(int key)
{
  if ( AF > 0)  //if operand has been pressed, second number assumed
  {
    r2 = r2 * 10; //This accounts for previous numbers
    r2 = r2 + key;
  }
 
  else  //If not, first number assumed, 
  { 
    //if r3 was the previous number, it will be affected
     r1 = r1 * 10; 
     r1 = r1 + key;  
  }
}

void calculate_number()
{
  if (AF == 0)  //This makes sure that if someone 
  { 
    //presses equals before an operand is pressed, the case is handled.
    r3 = r1;
  }
  else if (AF == 1) //+ key pressed and equals key pressed
  {
    r3=r1+r2;
  } 
   
  else if (AF == 2) //-key pressed and equals key pressed
  {
    r3=r1-r2;
  }
  else if (AF == 3) // * key pressed and equals key pressed
  {
    r3 = r1 * r2;
  }
  else if (AF == 4) // Divide key pressed and equals key pressed
  {
    if(r2 != 0)
      r3= r1/r2;
  }
}

void display_result()
{
  // change r3 to char array
  int temp = r3, count = 0;
  if(temp < 0)
    temp *= -1;
  char *num = new char;
  do
  {
    num[count++] = (temp % 10) + '0';
    temp /= 10;
  } while (temp != 0);
  
  init_lcd();
  if(r3 < 0)
    data_4bit('-');
    
  for (int i = count - 1; i > -1; --i)
    data_4bit(num[i]);
  
}

void rotate_servo(int Angle)
{
  DDRC = 0b00000001; // configure pin
 while(Angle > 0)
 {
   Angle--;
   PORTC = 0b00000001; // set 1th bit to HIGH
   _delay_us(1600);
   PORTC = 0b00000000; // set 1th bit to LOW
   _delay_us(1450);
 }
}