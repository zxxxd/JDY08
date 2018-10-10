// Visual Micro is in vMicro>General>Tutorial Mode
// 
/*
    Name:       JDY08_AT_Burn.ino
    Created:	2018/8/24 9:14:48
    Author:     ZXD
	用途：用于给JDY08 IBeacon模块快速配置参数。一次可配置4片IBeacon。
	可自动检测模块有无，有LED灯指示烧录状态。
	串口监视器波特率为115200，可通过串口监视器配置IBeacon参数。
*/

#include <EEPROM.h>
#include <string.h>

#define BAUD_RATE 115200	//波特率
#define SERIAL_WAIT_TIME 40000		//串口输入等待时间,40秒
#define Button_pin 9
#define S1_LED 4
#define S2_LED 3
#define S3_LED 2
#define Button_LED 8

/*
	如果需要更改预设参数，请把 FIRST_USE 置1。烧录后记得运行一次，然后置0并再烧录。
	下面的参数为预设参数，仅在 FIRST_USE 置1时起作用。
	注意：Major和Minor的最大值为65532。
 */
#define FIRST_USE 0
char UUID[33] = "FDA50693A4E24FB1AFCFC6EB07647825";		//必须为字符串，十六进制
uint16_t Major_hex = 10190;									//十进制
uint16_t Minor_hex = 41;									//十进制
uint32_t SYS_ID = 21587852;		//系统ID, 十进制
uint32_t Dev_Code = 49;			//设备编号，十进制
char Major[5];					//字符串，代表uint16_t的十六进制
char Minor[5];


/*
	用EEPROM存储上次的配置参数
	地址0-31：由高位到低位存储 char UUID[33]
	地址32-33：由高位到低位存储 uint16_t Major
	地址34-35：由高位到低位存储 uint16_t Minor
	地址36-39：由高位到低位存储 uint32_t SYS_ID
	地址40-43：由高位到低位存储 uint32_t Dev_Code
*/
const uint16_t EEPROM_address = 0;	//EEPROM的初始地址

void read_EEPROM_data(char *UUID_Rom);
void write_EEPROM_data(char *UUID_Rom, uint32_t SYS_ID, uint32_t dev_code);
//将十六进制的ACSII字符串转换成整数
uint16_t StrToHex(char *str_read, int len = 4);
//将十进制的ACSII字符串转换成整数
uint32_t StrToDEC(char *str_read, int len);
//将整数转为ACSII字符串，字符串为16进制
void HexToStr(uint16_t number, char *str_out);
//清空串口的buffer
void clear_Serial_buffer(uint8_t port = 0);

//NO.1 烧录
int burn_AT_cmd(uint8_t port);	//返回1为成功，0为失败



void setup()
{
	//下面的初始化无意义
	char UUID_Rom[33] = "01234567890123456789012345678901";	//UUID
	char string_read[10];									//临时存储
	int len;
	//硬件初始化
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

	//等待串口就绪
	while (!Serial) {
		; // wait for serial port to connect. Needed for native USB
	}

#if FIRST_USE
	write_EEPROM_data(UUID, SYS_ID, Dev_Code);		//向EEPROM写入参数
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
	 * 下面的代码是用于用户更改参数的操作流程
	 * 1. 检查相应参数是否正确。正确则输入“Y”或“y”，错误就输入“N”或“n”，然后点击“发送”。注意不要输入回车。
	 * 2. 若参数错误，则输入正确的参数并点击发送。不要输入回车。
	 * 3. 对于不能更改的参数，请在第26行把 FIRST_USE 置1，并更改下面的参数为正确的值，烧录并运行。
		  完成后把 FIRST_USE 置0.
	 * 4. 输入的参数，请用0补全高位,，例如参数10，需要输入为00010
	 */

	//更改 Major 参数
	clear_Serial_buffer();
	Serial.setTimeout(SERIAL_WAIT_TIME);	//设置串口等待数据的时间为40秒（SERIAL_WAIT_TIME）
	Serial.println("Is Major data Right(Y/N)?");
	Serial.readBytes(string_read, 1);
	if ('N' == string_read[0] || 'n' == string_read[0])
	{
		memset(string_read, NULL, 10);
		
		Serial.println("Enter the new Major. The Max is 65532.");
		while (!Serial.available())
		{
		}
		delay(50);	//必须要，否则只能读出1
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
	Serial.setTimeout(SERIAL_WAIT_TIME);	//设置串口等待数据的时间为40秒（SERIAL_WAIT_TIME）


	//更改 Minor 参数
	Serial.println("Is Minor data Right(Y/N)?");
	Serial.readBytes(string_read, 1);
	if ('N' == string_read[0] || 'n' == string_read[0])
	{
		memset(string_read, NULL, 10);
		Serial.println("Enter the new Minor. The Max is 65532.");
		while (!Serial.available())
		{
		}
		delay(50);	//必须要，否则只能读出1
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
	Serial.setTimeout(SERIAL_WAIT_TIME);	//设置串口等待数据的时间为40秒（SERIAL_WAIT_TIME）


	//更改 SYS_ID 参数
	Serial.println("Is SYS_ID data Right(Y/N)?");
	Serial.readBytes(string_read, 1);
	if ('N' == string_read[0] || 'n' == string_read[0])
	{
		memset(string_read, NULL, 10);
		Serial.println("Enter the new SYS_ID.");
		while (!Serial.available())
		{
		}
		delay(50);	//必须要，否则只能读出1
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
	Serial.setTimeout(SERIAL_WAIT_TIME);	//设置串口等待数据的时间为40秒（SERIAL_WAIT_TIME）


	//更改 Dev_Code 参数
	Serial.println("Is Dev_Code data Right(Y/N)?");
	Serial.readBytes(string_read, 1);
	if ('N' == string_read[0] || 'n' == string_read[0])
	{
		memset(string_read, NULL, 10);
		Serial.println("Enter the new Dev_Code.");
		while (!Serial.available())
		{
		}
		delay(50);	//必须要，否则只能读出1
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
	write_EEPROM_data(UUID, SYS_ID, Dev_Code);		//向EEPROM写入参数
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
				write_EEPROM_data(UUID, SYS_ID, Dev_Code);		//向EEPROM写入参数
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
	用于读取EEPROM中存储的参数。
	输入：一个参数的字符串。会被指针给更改。
	输出：无
*/
void read_EEPROM_data(char *UUID_Rom)
{
	uint32_t buffer_u32 = 0;	//临时存储过程中产生的每个8位
	for (int i = 0; i < 32; i++)
	{
		*(UUID_Rom + i) = EEPROM.read(EEPROM_address + i);
	}
	//读取Major
	Major_hex = Minor_hex = 0;
	buffer_u32 = EEPROM.read(EEPROM_address + 32);	//读高位
	Major_hex = (buffer_u32 << 8) & 0xff00;
	buffer_u32 = EEPROM.read(EEPROM_address + 33);	//读低位
	Major_hex = Major_hex + (buffer_u32 & 0xff);
	//读取Minor
	buffer_u32 = EEPROM.read(EEPROM_address + 34);
	Minor_hex = (buffer_u32 << 8) & 0xff00;
	buffer_u32 = EEPROM.read(EEPROM_address + 35);
	Minor_hex = Minor_hex + (buffer_u32 & 0xff);

	//读取Dev相关
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
	用于把参数写入EEPROM中。
	输入：三个参数的字符串
*/
void write_EEPROM_data(char *UUID_Rom, uint32_t SYS_ID, uint32_t dev_code)
{
	uint8_t buffer_u8 = 0;	//临时存储过程中产生的每个8位
	for (int i = 0; i < 32; i++)	//写入 UUID
	{
		EEPROM.write(EEPROM_address + i, *(UUID_Rom + i));
	}
	
	//Major_hex = Minor_hex = 0;
	//由高位到低位写入 Major
	buffer_u8 = (Major_hex & 0xff00) >> 8;
	EEPROM.write(EEPROM_address + 32, buffer_u8);	//写高8位
	buffer_u8 = Major_hex & 0xff;
	EEPROM.write(EEPROM_address + 33, buffer_u8);	//写低8位

	//由高位到低位写入 Minor
	buffer_u8 = (Minor_hex & 0xff00) >> 8;
	EEPROM.write(EEPROM_address + 34, buffer_u8);	//写高8位
	buffer_u8 = Minor_hex & 0xff;
	EEPROM.write(EEPROM_address + 35, buffer_u8);	//写低8位


	//SYS_ID 分解
	buffer_u8 = (SYS_ID >> 24) & 0xff;
	EEPROM.write(EEPROM_address + 36, buffer_u8);
	buffer_u8 = (SYS_ID >> 16) & 0xff;
	EEPROM.write(EEPROM_address + 37, buffer_u8);
	buffer_u8 = (SYS_ID >> 8) & 0xff;
	EEPROM.write(EEPROM_address + 38, buffer_u8);
	buffer_u8 = SYS_ID & 0xff;
	EEPROM.write(EEPROM_address + 39, buffer_u8);

	//dev_code 分解
	buffer_u8 = (dev_code >> 24) & 0xff;
	EEPROM.write(EEPROM_address + 40, buffer_u8);
	buffer_u8 = (dev_code >> 16) & 0xff;
	EEPROM.write(EEPROM_address + 41, buffer_u8);
	buffer_u8 = (dev_code >> 8) & 0xff;
	EEPROM.write(EEPROM_address + 42, buffer_u8);
	buffer_u8 = dev_code & 0xff;
	EEPROM.write(EEPROM_address + 43, buffer_u8);
}

//将大写转为小写
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
	功能：将十六进制的ACSII字符串转换成整数。仅支持16位及以下的。
	输入：需要被转换的字符串
	输出：对应的数字
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
功能：将十进制的ACSII字符串转换成整数。
输入：需要被转换的字符串，字符串长度
输出：对应的数字
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
	void HexToStr(uint16_t number, char *str_out)；
	功能：把数字转为对应的字符串
	输入：number，输入的数字。
		  *str_out，以指针形式输出转换后的字符串
	输出：无
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

	/*********  配置IBeacon模式  ***********/
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
	
	/******  重启  *******/
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
	//delay(250);	//经测试不能改小
	clear_Serial_buffer(port);
	/******** 配置UUID  ************/
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
		/*************  校验参数  ******************/
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
		if (strcmp((str_Serial + 10), (str_check + 6)))	//比较参数。相同就返回0.
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


	/******  配置Major  ********/
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
		/*************  校验参数  ******************/
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

	/***************  配置Minor  ******************/
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
		/*************  校验参数  ******************/
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


/* 函数名：void clear_Serial_buffer(uint8_t port = 0)
 * 功能：清空串口的缓存区
 * 参数：端口号。Serial 为 0，Serial1 为 1， 等等。默认为0.
 * 返回值：无
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
