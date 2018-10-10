// Visual Micro is in vMicro>General>Tutorial Mode
// 
/*
    Name:       JDY08_AT_Burn.ino
    Created:	2018/8/24 9:14:48
    Author:     ZXD
	��;�����ڸ�JDY08 IBeaconģ��������ò�����һ�ο�����4ƬIBeacon��
	���Զ����ģ�����ޣ���LED��ָʾ��¼״̬��
	���ڼ�����������Ϊ115200����ͨ�����ڼ���������IBeacon������
*/

#include <EEPROM.h>
#include <string.h>

#define BAUD_RATE 115200	//������
#define SERIAL_WAIT_TIME 40000		//��������ȴ�ʱ��,40��
#define Button_pin 9
#define S1_LED 4
#define S2_LED 3
#define S3_LED 2
#define Button_LED 8

/*
	�����Ҫ����Ԥ���������� FIRST_USE ��1����¼��ǵ�����һ�Σ�Ȼ����0������¼��
	����Ĳ���ΪԤ����������� FIRST_USE ��1ʱ�����á�
	ע�⣺Major��Minor�����ֵΪ65532��
 */
#define FIRST_USE 0
char UUID[33] = "FDA50693A4E24FB1AFCFC6EB07647825";		//����Ϊ�ַ�����ʮ������
uint16_t Major_hex = 10190;									//ʮ����
uint16_t Minor_hex = 41;									//ʮ����
uint32_t SYS_ID = 21587852;		//ϵͳID, ʮ����
uint32_t Dev_Code = 49;			//�豸��ţ�ʮ����
char Major[5];					//�ַ���������uint16_t��ʮ������
char Minor[5];


/*
	��EEPROM�洢�ϴε����ò���
	��ַ0-31���ɸ�λ����λ�洢 char UUID[33]
	��ַ32-33���ɸ�λ����λ�洢 uint16_t Major
	��ַ34-35���ɸ�λ����λ�洢 uint16_t Minor
	��ַ36-39���ɸ�λ����λ�洢 uint32_t SYS_ID
	��ַ40-43���ɸ�λ����λ�洢 uint32_t Dev_Code
*/
const uint16_t EEPROM_address = 0;	//EEPROM�ĳ�ʼ��ַ

void read_EEPROM_data(char *UUID_Rom);
void write_EEPROM_data(char *UUID_Rom, uint32_t SYS_ID, uint32_t dev_code);
//��ʮ�����Ƶ�ACSII�ַ���ת��������
uint16_t StrToHex(char *str_read, int len = 4);
//��ʮ���Ƶ�ACSII�ַ���ת��������
uint32_t StrToDEC(char *str_read, int len);
//������תΪACSII�ַ������ַ���Ϊ16����
void HexToStr(uint16_t number, char *str_out);
//��մ��ڵ�buffer
void clear_Serial_buffer(uint8_t port = 0);

//NO.1 ��¼
int burn_AT_cmd(uint8_t port);	//����1Ϊ�ɹ���0Ϊʧ��



void setup()
{
	//����ĳ�ʼ��������
	char UUID_Rom[33] = "01234567890123456789012345678901";	//UUID
	char string_read[10];									//��ʱ�洢
	int len;
	//Ӳ����ʼ��
	Serial.begin(BAUD_RATE);
	Serial.setTimeout(SERIAL_WAIT_TIME);
	Serial1.begin(BAUD_RATE);
	Serial2.begin(BAUD_RATE);
	Serial3.begin(BAUD_RATE);
	pinMode(Button_pin, INPUT_PULLUP);
	pinMode(S1_LED, OUTPUT);
	pinMode(S2_LED, OUTPUT);
	pinMode(S3_LED, OUTPUT);
	pinMode(Button_LED, OUTPUT);
	digitalWrite(S1_LED, 1);
	digitalWrite(S2_LED, 1);
	digitalWrite(S3_LED, 1);
	digitalWrite(Button_LED, 1);

	//�ȴ����ھ���
	while (!Serial) {
		; // wait for serial port to connect. Needed for native USB
	}

#if FIRST_USE
	write_EEPROM_data(UUID, SYS_ID, Dev_Code);		//��EEPROMд�����
#endif

	delay(100);
	Serial.print("Start system! Read the EEPROM.\n");
	read_EEPROM_data(UUID_Rom);
	Serial.print("UUID_ROM(HEX) = 0x");
	Serial.print(UUID_Rom);
	Serial.println();
	Serial.print("Major_ROM(DEC) = ");
	Serial.print(Major_hex, DEC);
	Serial.println();
	Serial.print("Minor_ROM(DEC) = ");
	Serial.print(Minor_hex, DEC);
	Serial.println();
	Serial.print("SYS_ID(DEC) = ");
	Serial.print(SYS_ID, DEC);
	Serial.println();
	Serial.print("Dev_Code(DEC) = ");
	Serial.print(Dev_Code, DEC);
	Serial.println();

	/*
	 * ����Ĵ����������û����Ĳ����Ĳ�������
	 * 1. �����Ӧ�����Ƿ���ȷ����ȷ�����롰Y����y������������롰N����n����Ȼ���������͡���ע�ⲻҪ����س���
	 * 2. ������������������ȷ�Ĳ�����������͡���Ҫ����س���
	 * 3. ���ڲ��ܸ��ĵĲ��������ڵ�26�а� FIRST_USE ��1������������Ĳ���Ϊ��ȷ��ֵ����¼�����С�
		  ��ɺ�� FIRST_USE ��0.
	 * 4. ����Ĳ���������0��ȫ��λ,���������10����Ҫ����Ϊ00010
	 */

	//���� Major ����
	clear_Serial_buffer();
	Serial.setTimeout(SERIAL_WAIT_TIME);	//���ô��ڵȴ����ݵ�ʱ��Ϊ40�루SERIAL_WAIT_TIME��
	Serial.println("Is Major data Right(Y/N)?");
	Serial.readBytes(string_read, 1);
	if ('N' == string_read[0] || 'n' == string_read[0])
	{
		memset(string_read, NULL, 10);
		
		Serial.println("Enter the new Major. The Max is 65532.");
		while (!Serial.available())
		{
		}
		delay(50);	//����Ҫ������ֻ�ܶ���1
		len = Serial.available();
		Serial.readBytes(string_read, len);
		Major_hex = StrToDEC(string_read, len);
		Serial.println(string_read);
		Serial.print("New Major is: ");
		Serial.println(Major_hex, DEC);
		memset(string_read, NULL, 10);
	}
	else
	{
		;
	}
	clear_Serial_buffer();
	Serial.setTimeout(SERIAL_WAIT_TIME);	//���ô��ڵȴ����ݵ�ʱ��Ϊ40�루SERIAL_WAIT_TIME��


	//���� Minor ����
	Serial.println("Is Minor data Right(Y/N)?");
	Serial.readBytes(string_read, 1);
	if ('N' == string_read[0] || 'n' == string_read[0])
	{
		memset(string_read, NULL, 10);
		Serial.println("Enter the new Minor. The Max is 65532.");
		while (!Serial.available())
		{
		}
		delay(50);	//����Ҫ������ֻ�ܶ���1
		len = Serial.available();
		Serial.readBytes(string_read, len);
		Minor_hex = StrToDEC(string_read, len);
		Serial.print("New Minor is: ");
		Serial.println(Minor_hex, DEC);
		memset(string_read, NULL, 10);
	}
	else
	{
		;
	}
	clear_Serial_buffer();
	Serial.setTimeout(SERIAL_WAIT_TIME);	//���ô��ڵȴ����ݵ�ʱ��Ϊ40�루SERIAL_WAIT_TIME��


	//���� SYS_ID ����
	Serial.println("Is SYS_ID data Right(Y/N)?");
	Serial.readBytes(string_read, 1);
	if ('N' == string_read[0] || 'n' == string_read[0])
	{
		memset(string_read, NULL, 10);
		Serial.println("Enter the new SYS_ID.");
		while (!Serial.available())
		{
		}
		delay(50);	//����Ҫ������ֻ�ܶ���1
		len = Serial.available();
		Serial.readBytes(string_read, len);
		SYS_ID = StrToDEC(string_read, len);
		Serial.print("New SYS_ID is: ");
		Serial.println(SYS_ID, DEC);
		memset(string_read, NULL, 10);
	}
	else
	{
		;
	}
	clear_Serial_buffer();
	Serial.setTimeout(SERIAL_WAIT_TIME);	//���ô��ڵȴ����ݵ�ʱ��Ϊ40�루SERIAL_WAIT_TIME��


	//���� Dev_Code ����
	Serial.println("Is Dev_Code data Right(Y/N)?");
	Serial.readBytes(string_read, 1);
	if ('N' == string_read[0] || 'n' == string_read[0])
	{
		memset(string_read, NULL, 10);
		Serial.println("Enter the new Dev_Code.");
		while (!Serial.available())
		{
		}
		delay(50);	//����Ҫ������ֻ�ܶ���1
		len = Serial.available();
		Serial.readBytes(string_read, len);
		Dev_Code = StrToDEC(string_read, len);
		Serial.print("New Dev_Code is: ");
		Serial.println(Dev_Code, DEC);
		memset(string_read, NULL, 10);
	}
	else
	{
		;
	}
	clear_Serial_buffer();
	
	Serial.print("UUID_ROM(HEX) = 0x");
	Serial.print(UUID_Rom);
	Serial.println();
	Serial.print("Major_ROM(DEC) = ");
	Serial.print(Major_hex, DEC);
	Serial.println();
	Serial.print("Minor_ROM(DEC) = ");
	Serial.print(Minor_hex, DEC);
	Serial.println();
	Serial.print("SYS_ID(DEC) = ");
	Serial.print(SYS_ID, DEC);
	Serial.println();
	Serial.print("Dev_Code(DEC) = ");
	Serial.print(Dev_Code, DEC);
	Serial.println();
	Serial.println("The parameter settings are complete. Ready to burn.\n\n");
	write_EEPROM_data(UUID, SYS_ID, Dev_Code);		//��EEPROMд�����
	digitalWrite(S1_LED, 0);
	digitalWrite(S2_LED, 0);
	digitalWrite(S3_LED, 0);
	Serial.setTimeout(100);
	Serial1.setTimeout(250);
	Serial2.setTimeout(250);
	Serial3.setTimeout(250);
	HexToStr(Major_hex, Major);
}

// Add the main program code into the continuous loop() function
void loop()
{
	char str_null[50];
	if (0 == digitalRead(Button_pin))
	{
		Serial1.begin(BAUD_RATE);
		Serial2.begin(BAUD_RATE);
		Serial3.begin(BAUD_RATE);
		digitalWrite(Button_LED, 0);
		digitalWrite(S1_LED, 0);
		digitalWrite(S2_LED, 0);
		digitalWrite(S3_LED, 0);
		Serial.println("\nStart burning!");
		for (uint8_t i = 1; i < 4; i++)
		{
			if (burn_AT_cmd(i))
			{
				Serial.print("SYS_ID = ");
				Serial.print(SYS_ID, DEC);
				Serial.print(",  Dev_Code = ");
				Serial.print(Dev_Code, DEC);
				Serial.print(",  Minor = ");
				Serial.print(Minor_hex, DEC);
				Serial.println("\n");
				SYS_ID++;
				Dev_Code++;
				Minor_hex++;
				write_EEPROM_data(UUID, SYS_ID, Dev_Code);		//��EEPROMд�����
				switch (i)
				{
				case 1:
					digitalWrite(S1_LED, 1);
					break;
				case 2:
					digitalWrite(S2_LED, 1);
					break;
				case 3:
					digitalWrite(S3_LED, 1);
					break;
				default:
					break;
				}
			}
		}
		Serial.println("\n\n");
		while (!digitalRead(Button_pin))
			;
		digitalWrite(Button_LED, 1);
	}
	

	if (Serial.available()) {      // If anything comes in Serial (USB),
		Serial1.write(Serial.read());   // read it and send it out Serial1 (pins 0 & 1)
	}

	if (Serial1.available()) {     // If anything comes in Serial1 (pins 0 & 1)
		Serial.write(Serial1.read());   // read it and send it out Serial (USB)
	}
}




/*
	void read_EEPROM_data(char *UUID_Rom, char *Major_Rom, char *Minor_Rom);
	���ڶ�ȡEEPROM�д洢�Ĳ�����
	���룺һ���������ַ������ᱻָ������ġ�
	�������
*/
void read_EEPROM_data(char *UUID_Rom)
{
	uint32_t buffer_u32 = 0;	//��ʱ�洢�����в�����ÿ��8λ
	for (int i = 0; i < 32; i++)
	{
		*(UUID_Rom + i) = EEPROM.read(EEPROM_address + i);
	}
	//��ȡMajor
	Major_hex = Minor_hex = 0;
	buffer_u32 = EEPROM.read(EEPROM_address + 32);	//����λ
	Major_hex = (buffer_u32 << 8) & 0xff00;
	buffer_u32 = EEPROM.read(EEPROM_address + 33);	//����λ
	Major_hex = Major_hex + (buffer_u32 & 0xff);
	//��ȡMinor
	buffer_u32 = EEPROM.read(EEPROM_address + 34);
	Minor_hex = (buffer_u32 << 8) & 0xff00;
	buffer_u32 = EEPROM.read(EEPROM_address + 35);
	Minor_hex = Minor_hex + (buffer_u32 & 0xff);

	//��ȡDev���
	SYS_ID = Dev_Code = 0;
	buffer_u32 = EEPROM.read(EEPROM_address + 36);
	SYS_ID = (buffer_u32 << 24) & 0xff000000;
	buffer_u32 = EEPROM.read(EEPROM_address + 37);
	SYS_ID = SYS_ID + ((buffer_u32 << 16) & 0xff0000);
	buffer_u32 = EEPROM.read(EEPROM_address + 38);
	SYS_ID = SYS_ID + ((buffer_u32 << 8) & 0xff00);
	buffer_u32 = EEPROM.read(EEPROM_address + 39);
	SYS_ID = SYS_ID + (buffer_u32 & 0xff);

	buffer_u32 = EEPROM.read(EEPROM_address + 40);
	Dev_Code = (buffer_u32 << 24) & 0xff000000;
	buffer_u32 = EEPROM.read(EEPROM_address + 41);
	Dev_Code = Dev_Code + ((buffer_u32 << 16) & 0xff0000);
	buffer_u32 = EEPROM.read(EEPROM_address + 42);
	Dev_Code = Dev_Code + ((buffer_u32 << 8) & 0xff00);
	buffer_u32 = EEPROM.read(EEPROM_address + 43);
	Dev_Code = Dev_Code + (buffer_u32 & 0xff);
}

/*
	void write_EEPROM_data(char *UUID_Rom, uint32_t SYS_ID, uint32_t dev_code)
	���ڰѲ���д��EEPROM�С�
	���룺�����������ַ���
*/
void write_EEPROM_data(char *UUID_Rom, uint32_t SYS_ID, uint32_t dev_code)
{
	uint8_t buffer_u8 = 0;	//��ʱ�洢�����в�����ÿ��8λ
	for (int i = 0; i < 32; i++)	//д�� UUID
	{
		EEPROM.write(EEPROM_address + i, *(UUID_Rom + i));
	}
	
	//Major_hex = Minor_hex = 0;
	//�ɸ�λ����λд�� Major
	buffer_u8 = (Major_hex & 0xff00) >> 8;
	EEPROM.write(EEPROM_address + 32, buffer_u8);	//д��8λ
	buffer_u8 = Major_hex & 0xff;
	EEPROM.write(EEPROM_address + 33, buffer_u8);	//д��8λ

	//�ɸ�λ����λд�� Minor
	buffer_u8 = (Minor_hex & 0xff00) >> 8;
	EEPROM.write(EEPROM_address + 34, buffer_u8);	//д��8λ
	buffer_u8 = Minor_hex & 0xff;
	EEPROM.write(EEPROM_address + 35, buffer_u8);	//д��8λ


	//SYS_ID �ֽ�
	buffer_u8 = (SYS_ID >> 24) & 0xff;
	EEPROM.write(EEPROM_address + 36, buffer_u8);
	buffer_u8 = (SYS_ID >> 16) & 0xff;
	EEPROM.write(EEPROM_address + 37, buffer_u8);
	buffer_u8 = (SYS_ID >> 8) & 0xff;
	EEPROM.write(EEPROM_address + 38, buffer_u8);
	buffer_u8 = SYS_ID & 0xff;
	EEPROM.write(EEPROM_address + 39, buffer_u8);

	//dev_code �ֽ�
	buffer_u8 = (dev_code >> 24) & 0xff;
	EEPROM.write(EEPROM_address + 40, buffer_u8);
	buffer_u8 = (dev_code >> 16) & 0xff;
	EEPROM.write(EEPROM_address + 41, buffer_u8);
	buffer_u8 = (dev_code >> 8) & 0xff;
	EEPROM.write(EEPROM_address + 42, buffer_u8);
	buffer_u8 = dev_code & 0xff;
	EEPROM.write(EEPROM_address + 43, buffer_u8);
}

//����дתΪСд
int tolower(int c)
{
	if (c >= 'A' && c <= 'Z')
	{
		return c + 'a' - 'A';
	}
	else
	{
		return c;
	}
}

/*
	���ܣ���ʮ�����Ƶ�ACSII�ַ���ת������������֧��16λ�����µġ�
	���룺��Ҫ��ת�����ַ���
	�������Ӧ������
*/
uint16_t StrToHex(char *str_read, int len = 4)
{
	uint8_t i = 0;
	uint16_t n = 0;
	for (; (*(str_read+i) >= '0' && *(str_read + i) <= '9') || (*(str_read + i) >= 'a' && *(str_read + i) <= 'f')\
			|| (*(str_read + i) >= 'A' && *(str_read + i) <= 'F'); ++i)
	{
		if (tolower(*(str_read + i)) > '9')
		{
			n = 16 * n + (10 + tolower(*(str_read + i)) - 'a');
		}
		else
		{
			n = 16 * n + (tolower(*(str_read + i)) - '0');
		}
	}
	if (i != len)
		return 0;
	else
		return n;
}

/*
���ܣ���ʮ���Ƶ�ACSII�ַ���ת����������
���룺��Ҫ��ת�����ַ������ַ�������
�������Ӧ������
*/
uint32_t StrToDEC(char *str_read, int len)
{
	uint8_t i = 0;
	uint32_t n = 0;
	for (; (*(str_read + i) >= '0' && *(str_read + i) <= '9') ; ++i)
	{
		n = 10 * n + (tolower(*(str_read + i)) - '0');
	}
	if (i != len)
		return 0;
	else
		return n;
}

/*
	void HexToStr(uint16_t number, char *str_out)��
	���ܣ�������תΪ��Ӧ���ַ���
	���룺number����������֡�
		  *str_out����ָ����ʽ���ת������ַ���
	�������
*/
void HexToStr(uint16_t number, char *str_out)
{
	char num_str[17] = "0123456789ABCDEF";
	char n;
	uint16_t i;
	i = number;
	i = (i & 0xf000) >> 12;
	*str_out = num_str[i];
	i = number;
	i = (i & 0x0f00) >> 8;
	*(str_out+1) = num_str[i];
	i = number;
	i = (i & 0x00f0) >> 4;
	*(str_out+2) = num_str[i]; 
	i = number;
	i = i & 0x000f;
	*(str_out+3) = num_str[i];
}

/*


*/
int burn_AT_cmd(uint8_t port)
{
	clear_Serial_buffer(port);
	char str_Serial[50];
	char str_check[50];
	memset(str_Serial, NULL, 50);
	memset(str_check, NULL, 50);
	HexToStr(Minor_hex, Minor);

	/*********  ����IBeaconģʽ  ***********/
	switch (port)				
	{
	case 1:
		Serial1.write("AT+HOSTEN3");
		delay(20);
		Serial1.readBytesUntil('\n', str_Serial, 10);
		break;
	case 2:
		Serial2.write("AT+HOSTEN3");
		delay(20);
		Serial2.readBytesUntil('\n', str_Serial, 10);
		break;
	case 3:
		Serial3.write("AT+HOSTEN3");
		delay(20);
		Serial3.readBytesUntil('\n', str_Serial, 10);
		break;
	default:
		break;
	}
	if (str_Serial[1] != 'O' && str_Serial[2] != 'K')
	{
		Serial.print("Dev ");
		Serial.print(port);
		Serial.println(" not find!");
		Serial.write(str_Serial);
		Serial.println(" ");
		delay(100);
		clear_Serial_buffer(port);
		return 0;
	}
	else
	{
		/*Serial.print("NO.");
		Serial.print(port);
		Serial.println(" Mode OK!");*/
	}
	//delay(50);
	clear_Serial_buffer(port);
	
	/******  ����  *******/
	switch (port)
	{
	case 1:
		Serial1.write("AT+RST");
		break;
	case 2:
		Serial2.write("AT+RST");
		break;
	case 3:
		Serial3.write("AT+RST");
		break;
	default:
		break;
	}
	//delay(250);	//�����Բ��ܸ�С
	clear_Serial_buffer(port);
	/******** ����UUID  ************/
	memset(str_Serial, NULL, 50);
	memcpy(str_Serial, "AT+STRUUID", 10);
	strcat(str_Serial, UUID);
	switch (port)
	{
	case 1:
		Serial1.write(str_Serial, 42);
		delay(20);
		Serial1.readBytesUntil('\n', str_Serial, 10);
		break;
	case 2:
		Serial2.write(str_Serial, 42);
		delay(20);
		Serial2.readBytesUntil('\n', str_Serial, 10);
		break;
	case 3:
		Serial3.write(str_Serial, 42);
		delay(20);
		Serial3.readBytesUntil('\n', str_Serial, 10);
		break;
	default:
		break;
	}
	if (str_Serial[1] != 'O' && str_Serial[2] != 'K')
	{
		Serial.print("NO.");
		Serial.print(port);
		Serial.println(" STRUUID write ERROR!!!");
		Serial.write(str_Serial);
		Serial.println(" ");
		return 0;
	}
	else
	{
		clear_Serial_buffer(port);
		/*************  У�����  ******************/
		switch (port)
		{
		case 1:
			Serial1.write("AT+STRUUID");
			delay(40);
			Serial1.readBytesUntil('\n', str_check, 38);
			break;
		case 2:
			Serial2.write("AT+STRUUID");
			delay(40);
			Serial2.readBytesUntil('\n', str_check, 38);
			break;
		case 3:
			Serial3.write("AT+STRUUID");
			delay(40);
			Serial3.readBytesUntil('\n', str_check, 38);
			break;
		default:
			break;
		}
		if (strcmp((str_Serial + 10), (str_check + 6)))	//�Ƚϲ�������ͬ�ͷ���0.
		{
			Serial.print("NO.");
			Serial.print(port);
			Serial.println(" STRUUID check ERROR!!!");
			return 0;
		}
		else
		{
			/*Serial.print("NO.");
			Serial.print(port);
			Serial.println(" STRUUID OK.");*/
			return 1;
		}
	}
	clear_Serial_buffer(port);


	/******  ����Major  ********/
	memset(str_Serial, NULL, 50);
	memset(str_check, NULL, 50);
	memcpy(str_Serial, "AT+MAJOR", 8);
	strcat(str_Serial, Major);
	switch (port)
	{
	case 1:
		Serial1.write(str_Serial, 12);
		delay(20);
		Serial1.readBytesUntil('\n', str_Serial, 10);
		break;
	case 2:
		Serial2.write(str_Serial, 12);
		delay(20);
		Serial2.readBytesUntil('\n', str_Serial, 10);
		break;
	case 3:
		Serial3.write(str_Serial, 12);
		delay(20);
		Serial3.readBytesUntil('\n', str_Serial, 10);
		break;
	default:
		break;
	}
	if (str_Serial[1] != 'O' && str_Serial[2] != 'K')
	{
		Serial.print("NO.");
		Serial.print(port);
		Serial.println(" Major write ERROR!!!");
		Serial.write(str_Serial);
		Serial.println();
		return 0;
	}
	else
	{
		clear_Serial_buffer(port);
		/*************  У�����  ******************/
		switch (port)
		{
		case 1:
			Serial1.write("AT+MAJOR");
			delay(40);
			Serial1.readBytesUntil('\n', str_check, 11);
			break;
		case 2:
			Serial2.write("AT+MAJOR");
			delay(40);
			Serial2.readBytesUntil('\n', str_check, 11);
			break;
		case 3:
			Serial3.write("AT+MAJOR");
			delay(40);
			Serial3.readBytesUntil('\n', str_check, 11);
			break;
		default:
			break;
		}
		if (strcmp((str_Serial + 8), (str_check + 7)))
		{
			Serial.print("NO.");
			Serial.print(port);
			Serial.println(" Major check ERROR!!!");
		}
		else
		{
			/*Serial.print("NO.");
			Serial.print(port);
			Serial.println(" Major OK.");*/
		}
	}
	clear_Serial_buffer(port);
	//delay(300);

	/***************  ����Minor  ******************/
	memset(str_Serial, NULL, 50);
	memset(str_check, NULL, 50);
	memcpy(str_Serial, "AT+MINOR", 8);
	strcat(str_Serial, Minor);
	switch (port)
	{
	case 1:
		Serial1.write(str_Serial, 12);
		delay(20);
		Serial1.readBytesUntil('\n', str_Serial, 10);
		break;
	case 2:
		Serial2.write(str_Serial, 12);
		delay(20);
		Serial2.readBytesUntil('\n', str_Serial, 10);
		break;
	case 3:
		Serial3.write(str_Serial, 12);
		delay(20);
		Serial3.readBytesUntil('\n', str_Serial, 10);
		break;
	default:
		break;
	}
	
	if (str_Serial[1] != 'O' && str_Serial[2] != 'K')
	{
		Serial.print("NO.");
		Serial.print(port);
		Serial.println(" Minor write ERROR!!!");
		Serial.write(str_Serial);
		Serial.println();
		return 0;
	}
	else
	{
		clear_Serial_buffer(port);
		/*************  У�����  ******************/
		switch (port)
		{
		case 1:
			Serial1.write("AT+MINOR");
			delay(40);
			Serial1.readBytesUntil('\n', str_check, 11);
			break;
		case 2:
			Serial2.write("AT+MINOR");
			delay(40);
			Serial2.readBytesUntil('\n', str_check, 11);
			break;
		case 3:
			Serial3.write("AT+MINOR");
			delay(40);
			Serial3.readBytesUntil('\n', str_check, 11);
			break;
		default:
			break;
		}
		if (strcmp((str_Serial + 8), (str_check + 7)))	
		{
			Serial.print("NO.");
			Serial.print(port);
			Serial.println(" MINOR check ERROR!!!");
		}
		else
		{
			/*Serial.print("NO.");
			Serial.print(port);
			Serial.println(" MINOR OK.");*/
		}
		memset(str_check, NULL, 50);
		Serial.print("NO.");
		Serial.print(port);
		Serial.print(" Minor is:");
		Serial.println(Minor_hex, DEC);
	}
	clear_Serial_buffer(port);
	return 1;
}


/* ��������void clear_Serial_buffer(uint8_t port = 0)
 * ���ܣ���մ��ڵĻ�����
 * �������˿ںš�Serial Ϊ 0��Serial1 Ϊ 1�� �ȵȡ�Ĭ��Ϊ0.
 * ����ֵ����
*/
void clear_Serial_buffer(uint8_t port = 0)
{
	char buffer[100];
	int len;
	delay(150);
	switch (port)
	{
	case 0:
		if (Serial.available())
		{
			delay(50);
			len = Serial.available();
			//Serial.print("len = "); Serial.println(len);
			Serial.readBytes(buffer, len);
		}
		break;
	case 1:
		if (Serial1.available())
		{
			delay(50);
			len = Serial1.available();
			//Serial.print("len = "); Serial.println(len);
			Serial1.readBytes(buffer, len);
		}
		break;
	case 2:
		if (Serial2.available())
		{
			delay(50);
			len = Serial2.available();
			//Serial.print("len = "); Serial.println(len);
			Serial2.readBytes(buffer, len);
		}
		break;
	case 3:
		if (Serial3.available())
		{
			delay(50);
			len = Serial3.available();
			//Serial.print("len = "); Serial.println(len);
			Serial3.readBytes(buffer, len);
		}
		break;
	default:
		break;
	}
}
