#define SDA_PORT PORTC
#define SDA_PIN 4
#define SCL_PORT PORTC
#define SCL_PIN 5
#define I2C_SLOWMODE 1

#include <SoftI2CMaster.h>
#include <SoftWire.h>


// standard I2C address for eepron
int eepronAdress=80;

// standard I2C address for Smart Battery packs
byte deviceAddress =0x0b;

// Global variables
#define bufferLen 32
uint8_t i2cBuffer[bufferLen];
char read_buffer[8];

// Standard common for BQ2040
#define MANUFACTURER_ACCESS      0X00
#define REMANING_CAPACITY_ALARM  0X01
#define REMANING_TIME_ALARM      0X02
#define BATTERY_MODE             0x03
#define AT_RATE                  0X04
#define AT_RATETIME_TO_FULL      0X05
#define AT_RATETIME_TO_EMPTY     0X06
#define AT_RATE_OK               0X07
#define TEMPERATURE              0x08
#define VOLTAGE                  0x09
#define CURRENT                  0x0A
#define AVERAGE_CURRENT          0X0B
#define MAX_ERROR                0X0C
#define RELATIVE_STATE_CHARGE    0x0D
#define ABSOLUTE_STATE_CHARGE    0x0E
#define REMAINING_CAPACITY       0x0F
#define FULL_CHARGE_CAPACITY     0x10
#define RUN_TIME_TO_EMPTY        0X11
#define AVARAGE_TIME_TO_EMPTY    0X12
#define TIME_TO_FULL             0x13
#define CHARGING_CURRENT         0x14
#define CHARGING_VOLTAGE         0x15
#define BATTERY_STATUS           0x16
#define CYCLE_COUNT              0x17
#define DESIGN_CAPACITY          0x18
#define DESIGN_VOLTAGE           0x19
#define SPEC_INFO                0x1A
#define MFG_DATE                 0x1B
#define SERIAL_NUM               0x1C
#define MFG_NAME                 0x20   // String
#define DEV_NAME                 0x21   // String
#define CELL_CHEM                0x22   // String
#define MFG_DATA                 0x23   // String
#define FLAG1_FLAG2              0X2F
#define EDV1                     0x3E
#define EDVF                     0x3F


// Non starndard command for BQ2040
//#define MFG_DATA3B               0x4F   // String
//#define MFG_DATA3C               0x3C   // String
//#define MFG_DATA3D               0x3D   // String
//#define MFG_DATA3E               0x3E   // String
//#define MFG_DATA30               0x48   // String
//#define MFG_DATA31               0x49   // String
//#define MFG_DATA32               0x4A   // String
//#define MFG_DATA33               0x4B   // String
//#define CELL8_VOLTAGE            0x43
//#define CELL7_VOLTAGE            0x42
//#define CELL6_VOLTAGE            0x41
//#define CELL5_VOLTAGE            0x40
//#define CELL4_VOLTAGE            0x3C
//#define CELL3_VOLTAGE            0x3D
//#define CELL2_VOLTAGE            0x3E
//#define CELL1_VOLTAGE            0x3F
//#define STATE_OF_HEALTH          0x54
//--------------------------------------------------


SoftWire Wire = SoftWire();
void setup()
{
  
  Serial.begin(9600);
  while(!Serial){ //wait for connection
    Serial.println("waiting for serial port connection");
  }

  while(!i2c_init()){ //wait for connection, wait for pull up the ports
    Serial.println("waiting for start I2C");
  }

  Serial.println("All connections stabilized");
}

int fetchWord(byte func)
{
  i2c_start(deviceAddress<<1 | I2C_WRITE);
  i2c_write(func);
  i2c_rep_start(deviceAddress<<1 | I2C_READ);
  byte b1 = i2c_read(false);
  byte b2 = i2c_read(true);
  i2c_stop();
  return (int)b1|(((int)b2)<<8);
}

uint8_t i2c_smbus_read_block ( uint8_t command, uint8_t* blockBuffer, uint8_t blockBufferLen ) 
{
  uint8_t x, num_bytes;
  i2c_start((deviceAddress<<1) + I2C_WRITE);
  i2c_write(command);
  i2c_rep_start((deviceAddress<<1) + I2C_READ);
  num_bytes = i2c_read(false); // num of bytes; 1 byte will be index 0
  num_bytes = constrain(num_bytes,0,blockBufferLen-2); // room for null at the end
  for (x=0; x<num_bytes-1; x++) { // -1 because x=num_bytes-1 if x<y; last byte needs to be "nack"'d, x<y-1
    blockBuffer[x] = i2c_read(false);
  }
  blockBuffer[x++] = i2c_read(true); // this will nack the last byte and store it in x's num_bytes-1 address.
  blockBuffer[x] = 0; // and null it at last_byte+1
  i2c_stop();
  return num_bytes;
}

void scan()  
{
  byte i = 0;
  for ( i= 0; i < 127; i++  )
  {
    bool ack = i2c_start(i<<1 | I2C_WRITE); 
    if ( ack ) {
          Serial.print("Address: 0x");
    Serial.print(i,HEX);

      Serial.println(": OK");
      Serial.flush();
    }
    else {

    }
    i2c_stop();
  }
}

void writeEEPROM(byte address, byte info) {
  Wire.beginTransmission(eepronAdress);
  Wire.write(address);
  Wire.write(info);
  Wire.endTransmission(true);
  delay(6);
}



byte readEEPROM(byte address) {
  byte data;  
  Wire.beginTransmission(eepronAdress);
  Wire.write(address);
  Wire.endTransmission(false);
  Wire.requestFrom(eepronAdress,1);
  data=Wire.read();
  return data;
}


unsigned long parseHex(char* read_buffer) {
  unsigned long result = 0L;
  char byte = '\0';
  int i=0;

  while (byte != '#') {
    i++;
    byte =read_buffer[i] ;
    if (byte == '#') break;
    if (byte >= 'a' && byte <= 'f')
      byte = byte -'a' + 'A';
    if ((byte >= '0' && byte <= '9') ||
  (byte >= 'A' && byte <= 'F')) {
      if (byte >= '0' && byte <= '9') byte = byte - '0';
      else byte = byte - 'A' + 10;
      result = result * 16;
      result = result + byte;
    }
  }
  return result;
}



void loop()
{
  uint8_t length_read = 0;
  unsigned long address=1024;
  unsigned long info=0;
  unsigned int flags;
  byte FLAGS1;
  byte FLAGS2;
  float temperature;
  String formatted_date;
  int mdate;
  int mday;
  int mmonth;
  int myear;
  
 while (!Serial.available());
  
  if (Serial.available()>0){
    
      Serial.readBytes(read_buffer,8);
          if(read_buffer[1]!='\n')
            address=parseHex(&read_buffer[0]);
          if(read_buffer[4]!='\n')
            info=parseHex(&read_buffer[3]);

    switch(read_buffer[0]){
        case 'a':
          Serial.print("Serial Number: ");
          Serial.println((unsigned int)fetchWord(SERIAL_NUM));
          break;
          
        case 'b':
          Serial.print("Manufacturer name: ");
          length_read = i2c_smbus_read_block(MFG_NAME, i2cBuffer, bufferLen);
          Serial.write(i2cBuffer, length_read);
          Serial.println("");
          break;

        case 'c':
          Serial.print("Device Name: ");
          length_read = i2c_smbus_read_block(DEV_NAME, i2cBuffer, bufferLen);
          Serial.write(i2cBuffer, length_read);
          Serial.println("");
          break;


        case 'd':
           Serial.print("Chemistry: ");
           length_read = i2c_smbus_read_block(CELL_CHEM, i2cBuffer, bufferLen);
           Serial.write(i2cBuffer, length_read);
           Serial.println("");
           break;

        case 'e':
           Serial.print("Manufacturer Data: ");
           length_read = i2c_smbus_read_block(MFG_DATA, i2cBuffer, bufferLen);
           Serial.write(i2cBuffer, length_read);
           Serial.println("");
           break;
        
        case 'f' :
          Serial.print("Remaining Capacity Alarm (mAh): ");
          Serial.println((unsigned int)fetchWord(REMANING_CAPACITY_ALARM));
          break;

        case 'g':
          Serial.print("Design Capacity (mAh): " );
          Serial.println((unsigned int)fetchWord(DESIGN_CAPACITY));
          break;

        case 'h':
          Serial.print("Full Charge Capacity (mAh): " );
          Serial.println((unsigned int)fetchWord(FULL_CHARGE_CAPACITY));
          break;

        case 'i':
          formatted_date = "Manufacture Date (Y-M-D): ";
          mdate = fetchWord(MFG_DATE);
          mday = B00011111 & mdate;
          mmonth = mdate>>5 & B00001111;
          myear = 1980 + (mdate>>9 & B01111111);
          formatted_date += myear;
          formatted_date += "-";
          formatted_date += mmonth;
          formatted_date += "-";
          formatted_date += mday;
          Serial.println(formatted_date);
          break;

        case 'j':
          Serial.print("Design Voltage (mV): " );
          Serial.println((unsigned int)fetchWord(DESIGN_VOLTAGE));
          break;
          
        case 'l' :
          Serial.print("EDVF: ");
          Serial.println(fetchWord(EDVF));
          break;

        case 'k' :
          Serial.print("EDV1: ");
          Serial.println(fetchWord(EDV1));
          break; 

        case 'm':
          Serial.print("Battery Mode (BIN): 0b");
          Serial.println(fetchWord(BATTERY_MODE),BIN);
           break;
        
        case 'n':
          Serial.print("Battery Status (BIN): 0b");
          Serial.println(fetchWord(BATTERY_STATUS),BIN);
           break;

        case 'o':
          Serial.print("Charging Current (mA): ");
          Serial.print((unsigned int)fetchWord(CHARGING_CURRENT));
          Serial.println("");
          break;

        case 'p':
          Serial.print("Charging Voltage (mV): ");
          Serial.println((unsigned int)fetchWord(CHARGING_VOLTAGE));
           break;

        case 'q':
          Serial.print("Cycle Count: " );
          Serial.println((unsigned int)fetchWord(CYCLE_COUNT));
          break;
          
        case 'r' :
          flags=fetchWord(FLAG1_FLAG2);
          FLAGS1= (flags & 0xFF);
          FLAGS2= (flags>>8) & 0xFF;
          Serial.print("FLAGS1: ");
          Serial.println(FLAGS1,BIN);
          Serial.print("FLAGS2: ");
          Serial.println(FLAGS2,BIN);
          break;

        case 's':
          Serial.print("Remaining Capacity (mAh): " );
          Serial.println((unsigned int)fetchWord(REMAINING_CAPACITY));
          break;

        case 't':
          Serial.print("Relative Charge(%): ");
          Serial.println((unsigned int)fetchWord(RELATIVE_STATE_CHARGE ));
           break;

        case 'u':
          Serial.print("Absolute Charge(%): ");
          Serial.println((unsigned int)fetchWord(ABSOLUTE_STATE_CHARGE));
          break;

        case 'v':
          Serial.print("Avarage time to full (min): ");
          Serial.println((unsigned int)fetchWord(TIME_TO_FULL));
          break;
          
        case 'w' :
          Serial.print("Avarage time to empty (min): ");
          Serial.println((unsigned int)fetchWord(AVARAGE_TIME_TO_EMPTY));
          break;

        case 'x':
          Serial.print("Voltage (mV): ");
          Serial.println((unsigned int)fetchWord(VOLTAGE));
          break;

        case 'y' :
          Serial.print("Manufacturer Access: ");
          Serial.println(fetchWord(MANUFACTURER_ACCESS));
          break;

        case 'z' :
          Serial.print("At rate (mA): ");
          Serial.println(fetchWord(AT_RATE));
          break;

        case 'A' :
          Serial.print("At rate time to full (min): ");
          Serial.println((unsigned int)fetchWord(AT_RATETIME_TO_FULL));
          break;

        case 'B' :
          Serial.print("At rate time to empty (min): ");
          Serial.println((unsigned int)fetchWord(AT_RATETIME_TO_EMPTY));
          break;

        case 'C' :
          Serial.print("At rate OK: ");
          Serial.println(fetchWord(AT_RATE_OK),BIN);
          break;

        case 'D' :
          Serial.print("Temperature in (°C): ");
          temperature = ((((unsigned int)fetchWord(TEMPERATURE))/10)-273);
          Serial.println(temperature);
          break;

        case 'E' :
          Serial.print("Current (mA): ");
          Serial.println(fetchWord(CURRENT));
          break;

        case 'F' :
          Serial.print("Avarage current (mA): ");
          Serial.println(fetchWord(AVERAGE_CURRENT));
          break;

        case 'G' :
          Serial.print("MAX ERROR (%): ");
          Serial.println((unsigned int)fetchWord(MAX_ERROR));
          break;

        case 'H' :
          Serial.print("Run time to empty (min): ");
          Serial.println((unsigned int)fetchWord(RUN_TIME_TO_EMPTY));
          break;
          
        case 'I':
          Serial.print("Specification Info: ");
          Serial.println(fetchWord(SPEC_INFO));
          break;
          
        case 'J':
          Serial.print("Remaining time alarm (min): ");
          Serial.println((unsigned int)fetchWord(REMANING_TIME_ALARM));
          break;
               
        case 'R':
          Serial.print(address,HEX);
          Serial.print(" : ");
          Serial.println(readEEPROM(address),HEX);
          break;
          
        case 'W':
          writeEEPROM(address,info);
          break;

        default:
          Serial.println("Invalid Operation");
          break;
    }
  }
        
}