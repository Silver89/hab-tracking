#include <TinyGPS++.h>
#include <string.h>
#include <util/crc16.h>
#include <unistd.h>

#define RADIOPIN 9

TinyGPSPlus gps;

char datastring2[80];

void setup()
{
	Serial.begin(9600);

	pinMode(RADIOPIN, OUTPUT);
	setPwmFrequency(RADIOPIN, 1);
}

void loop()
{
	smartDelay(5000);
	transmitGpsData();
}

void transmitGpsData(){

	snprintf(datastring2, 80, " ");
  
	if (gps.location.isUpdated())
	{
		char strdate[16], strlat[16], strlng[16], strsat[16], strmeter[16];
		
		dtostrf(gps.time.value(), 8, 0, strdate);
		dtostrf(gps.location.lat(), 9, 6, strlat);
		dtostrf(gps.location.lng(), 8, 6, strlng);
		sprintf(strsat, "%d", gps.satellites.value());
		dtostrf(gps.altitude.meters(), 5, 2, strmeter);
		
		strcpy (datastring2,"GPS: ");
		
		strcat (datastring2,strdate);
		strcat (datastring2,", ");
		
		strcat (datastring2,strlat);
		strcat (datastring2,", ");
		
		strcat (datastring2,gps.location.rawLat().negative ? "S" : "N");
		strcat (datastring2,", ");
		
		strcat (datastring2,strlng);
		strcat (datastring2,", ");
		
		strcat (datastring2,gps.location.rawLng().negative ? "W" : "E"); 
		strcat (datastring2,", ");
		
		strcat (datastring2,strsat);
		strcat (datastring2,", ");
		
		strcat (datastring2,strmeter);	
		
		//puts (datastring2);

		snprintf(datastring2, 80, datastring2);
	} 
	else
	{
		snprintf(datastring2, 80, "Aquiring...");
	}
        
	unsigned int CHECKSUM = gps_CRC16_checksum(datastring2); // Calculates the checksum for this datastring		
	char checksum_str[6];              		
	sprintf(checksum_str, "*%04X", CHECKSUM);
	strcat(datastring2, checksum_str);	
	rtty_txstring (datastring2);
}

static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do 
  {
    while (Serial.available())
      gps.encode(Serial.read());
  } 
  while (millis() - start < ms);
}

//Simple function to sent a char at a time to rtty_txbyte function.NB Each char is one byte (8 Bits)
void rtty_txstring (char * string)
{
	char c;
	c = *string++;

	while ( c != '\0')
	{
		rtty_txbyte (c);
		c = *string++;
	}
}

// Simple function to sent each bit of a char to rtty_txbit function. NB The bits are sent Least Significant Bit first
// All chars should be preceded with a 0 and proceded with a 1. 0 = Start bit; 1 = Stop bit
void rtty_txbyte (char c)
{
	int i;
	rtty_txbit (0); // Start bit

	// Send bits for for char LSB first

	for (i=0;i<7;i++) // Change this here 7 or 8 for ASCII-7 / ASCII-8
	{
		if (c & 1)
			rtty_txbit(1);
		else
			rtty_txbit(0);

		c = c >> 1;
	}
	rtty_txbit (1); // Stop bit
	rtty_txbit (1); // Stop bit
}

void rtty_txbit (int bit)
{
	if (bit)
		analogWrite(RADIOPIN,110);
	else
		analogWrite(RADIOPIN,100);

	//delayMicroseconds(3370); // 300 baud
	delayMicroseconds(10000); // For 50 Baud uncomment this and the line below.
	delayMicroseconds(10150); // You can't do 20150 it just doesn't work as the
}

uint16_t gps_CRC16_checksum (char *string)
{
	size_t i;
	uint16_t crc;
	uint8_t c;

	crc = 0xFFFF;

	// Calculate checksum ignoring the first two $s
	for (i = 2; i < strlen(string); i++)
	{
		c = string[i];
		crc = _crc_xmodem_update (crc, c);
	}
	return crc;
}

void setPwmFrequency(int pin, int divisor) {
	
	byte mode;
	
	if(pin == 5 || pin == 6 || pin == 9 || pin == 10) {
		switch(divisor) {
			case 1:
			mode = 0x01;
			break;
			case 8:
			mode = 0x02;
			break;
			case 64:
			mode = 0x03;
			break;
			case 256:
			mode = 0x04;
			break;
			case 1024:
			mode = 0x05;
			break;
			default:
			return;
		}
		if(pin == 5 || pin == 6) {
			TCCR0B = TCCR0B & 0b11111000 | mode;
		}
		else {
			TCCR1B = TCCR1B & 0b11111000 | mode;
		}
	}
	else if(pin == 3 || pin == 11) {
		switch(divisor) {
		case 1:
		mode = 0x01;
		break;
		case 8:
		mode = 0x02;
		break;
		case 32:
		mode = 0x03;
		break;
		case 64:
		mode = 0x04;
		break;
		case 128:
		mode = 0x05;
		break;
		case 256:
		mode = 0x06;
		break;
		case 1024:
		mode = 0x7;
		break;
		default:
		return;
		}
		TCCR2B = TCCR2B & 0b11111000 | mode;
	}
}
