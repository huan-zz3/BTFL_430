#ifndef BTFL_430_h
#define BTFL_430_h

#include <Arduino.h>


namespace BTFLC
{
	typedef uint8_t errCodeType;
	typedef uint8_t channelType;
	typedef uint8_t dataType;
	typedef int valueType;
	typedef float percentType;
	
	const uint8_t Bytecount = 25;
	const uint8_t ChanneldataCount = 11;
	const unsigned int BAUD = 100000;  
	
	const channelType Forward_and_Back = 0;
	const channelType Left_and_Right = 1;
	const channelType Up_and_Down = 2;
	const channelType rLeft_and_rRight = 3;
	const channelType AUX1 = 4;
}

namespace BTFLErr
{
	using BTFLC::errCodeType;
	const errCodeType BTFL_OK  = 0x01;
	const errCodeType BTFL_NOFOUND = 0x02;
	const errCodeType BTFL_threadERR = 0x03;
}
namespace BTFLVariable
{
	using BTFLC::dataType;
	using BTFLC::Bytecount;
	uint8_t RXPIN = 4;      // No mean
	uint8_t TXPIN = 12;     // GPIO 12 => TX for Serial1
	dataType sendData[Bytecount];
	TaskHandle_t xHandle, xHandle2;
}
namespace BTFLStruct
{
	using BTFLC::errCodeType;
	using BTFLErr::BTFL_OK;
	struct Channel_defaultData {
		uint16_t Minimum;
		uint16_t Maximum;
		uint16_t Median;
		uint16_t Finetuning_Value;
	};
	struct BTFLReturnVal{
		errCodeType errCode = BTFL_OK;
	};
}
namespace BTFLNamespace
{
	using namespace BTFLC;
	using namespace BTFLErr;
	using namespace BTFLVariable;
	using namespace BTFLStruct;
	void unLock(void *);
	void mytaskloop(void *);
	void Lock(void *);
	
	
	class BTFLModel{
	public:
		uint8_t flyUnlock_Flag = 0;
		BTFLReturnVal setTXpin(uint8_t pinNum, uint8_t pinUless){
			BTFLReturnVal RTN;
			TXPIN = pinNum;
			RXPIN = pinUless;
			return RTN;
		}
		BTFLReturnVal allInit(void){
			BTFLReturnVal RTN;
			Serial1.begin(BAUD, SERIAL_8E2, RXPIN, TXPIN);
			for (int i = 0; i < Bytecount; i++) {
				sendData[i] = 200;
			}
			sendData[0] = 15;  //0x0F  Sbus协议为高位先行，发送结果为0xF0
			sendData[23] = 0;  //flag标志位
			sendData[24] = 0;  //0x00
			return RTN;
		}
		BTFLReturnVal flyDirection(channelType channelFlag, percentType percentage){
			BTFLReturnVal RTN, sRTN;
			percentage = dataHandle(channelFlag, percentage);
			switch(channelFlag){
				case Forward_and_Back:
					sRTN = SetChannel_2(percentage);
					break;
				case Left_and_Right:
					sRTN = SetChannel_1(percentage);
					break;
				case Up_and_Down:
					sRTN = SetChannel_3(percentage);
					break;
				case rLeft_and_rRight:
					sRTN = SetChannel_4(percentage);
					break;
				case AUX1:
					sRTN = SetChannel_5(percentage);
					break;
				default:
					RTN.errCode = BTFL_NOFOUND;
			}
			return RTN;
		}
		BTFLReturnVal fineTuningChangeValue(channelType channelFlag, uint16_t value){
			BTFLReturnVal RTN;
			switch(channelFlag){
				case Forward_and_Back:
					chan2dd.Finetuning_Value = value;
					break;
				case Left_and_Right:
					chan1dd.Finetuning_Value = value;
					break;
				case Up_and_Down:
					chan3dd.Finetuning_Value = value;
					break;
				case rLeft_and_rRight:
					chan4dd.Finetuning_Value = value;
					break;
				case AUX1:
					chan5dd.Finetuning_Value = value;
					break;
				default:
					RTN.errCode = BTFL_NOFOUND;
			}
			return RTN;
		}
		BTFLReturnVal flyReset(void){
			BTFLReturnVal RTN;
			RTN = SetChannel_1(0);
			RTN = SetChannel_2(0);
			RTN = SetChannel_3(0);
			RTN = SetChannel_4(0);
			return RTN;
		}
		BTFLReturnVal flyUnlock(void){
			BTFLReturnVal RTN;
			xTaskCreatePinnedToCore(unLock, "unLock", 4096, NULL, 3, &xHandle, 1);
			return RTN;
		}
		BTFLReturnVal startTaskLoop(void){
			BTFLReturnVal RTN;
			xTaskCreatePinnedToCore(mytaskloop, "mytaskloop", 4096, NULL, 1, &xHandle2, 0);
			return RTN;
		}
		BTFLReturnVal flyLock(void){
			BTFLReturnVal RTN;
			xTaskCreatePinnedToCore(Lock, "Lock", 4096, NULL, 1, &xHandle2, 0);
			return RTN;
		}
		
		
	private:
		Channel_defaultData chan1dd = { 250, 1700, 996, 0 };  //横滚
		Channel_defaultData chan2dd = { 250, 1700, 996, 0 };  //俯仰
		Channel_defaultData chan3dd = { 250, 1700, 996, 0 };  //油门
		Channel_defaultData chan4dd = { 250, 1700, 996, 0 };  //方向
		Channel_defaultData chan5dd = { 0, 110, 62, 0 };  //AUX1 1007~1307~1507~1707~2007
		
		BTFLReturnVal SetChannel_1(percentType percentage){
			BTFLReturnVal RTN;
			valueType temp = dataCount(chan1dd, percentage);
			int binaryarry[ChanneldataCount] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
			for (int i = 0; temp > 0; i++) {
				binaryarry[i] = temp % 2;
				temp /= 2;
			}

			dataType tempdata = 0;
			for (int i = 0; i < 8; i++) {
				tempdata += (Equate_2(i) * binaryarry[i]);
			}
			sendData[1] = tempdata;

			tempdata = 0;
			for (int i = 8; i < 11; i++) {
				tempdata += (Equate_2(i - 8) * binaryarry[i]);
			}
			sendData[2] |= tempdata;  //赋1 必须得 或1jied
			tempdata |= 0xF8;
			sendData[2] &= tempdata;  //赋0 必须得 和0
			return RTN;
		}
		BTFLReturnVal SetChannel_2(percentType percentage){
			BTFLReturnVal RTN;
			valueType temp = dataCount(chan2dd, percentage);
			int binaryarry[ChanneldataCount] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
			for (int i = 0; temp > 0; i++) {
				binaryarry[i] = temp % 2;
				temp /= 2;
			}

			dataType tempdata = 0;
			for (int i = 0; i < 5; i++) {
				tempdata += (Equate_2(i + 3) * binaryarry[i]);
			}
			sendData[2] |= tempdata;  //赋1 必须得 或1
			tempdata |= 0x07;
			sendData[2] &= tempdata;  //赋0 必须得 和0

			tempdata = 0;
			for (int i = 5; i < 11; i++) {
				tempdata += (Equate_2(i - 5) * binaryarry[i]);
			}
			sendData[3] |= tempdata;  //赋1 必须得 或1
			tempdata |= 0xC0;
			sendData[3] &= tempdata;  //赋0 必须得 和0
			return RTN;
		}
		BTFLReturnVal SetChannel_3(percentType percentage){
			BTFLReturnVal RTN;
			percentage = (percentage - 500) * 2;
			valueType temp = dataCount(chan3dd, percentage);
			
			/*Serial.print(temp);
			Serial.print("  ");*/
			
			int binaryarry[ChanneldataCount] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
			for (int i = 0; temp > 0; i++) {
				binaryarry[i] = temp % 2;
				temp /= 2;
			}

			dataType tempdata = 0;
			for (int i = 0; i < 2; i++) {
				tempdata += (Equate_2(i + 6) * binaryarry[i]);
			}
			sendData[3] |= tempdata;  //赋1 必须得 或1
			tempdata |= 0x3F;
			sendData[3] &= tempdata;  //赋0 必须得 和0

			tempdata = 0;
			for (int i = 2; i < 10; i++) {
				tempdata += (Equate_2(i - 2) * binaryarry[i]);
			}
			sendData[4] = tempdata;

			tempdata = 0;
			for (int i = 10; i < 11; i++) {
				tempdata += (Equate_2(i - 10) * binaryarry[i]);
			}
			sendData[5] |= tempdata;  //赋1 必须得 或1
			tempdata |= 0xFE;
			sendData[5] &= tempdata;  //赋0 必须得 和0
			return RTN;
		}
		BTFLReturnVal SetChannel_4(percentType percentage){
			BTFLReturnVal RTN;
			valueType temp = dataCount(chan4dd, percentage);
			int binaryarry[ChanneldataCount] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
			for (int i = 0; temp > 0; i++) {
				binaryarry[i] = temp % 2;
				temp /= 2;
			}

			dataType tempdata = 0;
			for (int i = 0; i < 7; i++) {
				tempdata += (Equate_2(i + 1) * binaryarry[i]);
			}
			sendData[5] |= tempdata;  //赋1 必须得 或1
			tempdata |= 0x01;
			sendData[5] &= tempdata;  //赋0 必须得 和0

			tempdata = 0;
			for (int i = 7; i < 11; i++) {
				tempdata += (Equate_2(i - 7) * binaryarry[i]);
			}
			sendData[6] |= tempdata;
			tempdata |= 0xF0;
			sendData[6] &= tempdata;
			return RTN;
		}
		BTFLReturnVal SetChannel_5(percentType percentage){
			BTFLReturnVal RTN;
			valueType temp = dataCount(chan5dd, percentage);
			sendData[7] = static_cast<dataType>(temp);
			return RTN;
		}
		
		valueType dataCount(Channel_defaultData chanxdd, percentType percentage){
			percentType temp;
			if(percentage>0){
				percentType a = chanxdd.Maximum - chanxdd.Median;
				percentType b = chanxdd.Finetuning_Value;
				percentType c = a/1000;
				percentType d = percentage;
				percentType e = c*d+b;
				temp = e + chanxdd.Median;
			}else{
				percentType a = chanxdd.Median - chanxdd.Minimum;
				percentType b = chanxdd.Finetuning_Value;
				percentType c = a/1000;
				percentType d = percentage;
				percentType e = c*d+b;
				temp = chanxdd.Median + e;
			}
			return static_cast<int>(temp);
		}
		
		unsigned int Equate_2(int count) {
			unsigned int temp = 1;
			for (int i = 0; i < count; i++) {
				temp *= 2;
			}
			return temp;
		}
		percentType dataHandle(channelType channelFlag, percentType percentage){
			if(channelFlag == Up_and_Down){
				if(percentage>1000 || percentage<0){
					return 500;
				}
				return percentage;
			}else{
				if(percentage>1000 || percentage<-1000){
					return 0;
				}
				return percentage;
			}
			
		}
	};
	
	BTFLModel btfl;
	void unLock(void *arg){
		vTaskDelay(5000);
		btfl.flyUnlock_Flag = 0;

		btfl.flyReset();
		btfl.flyDirection(Up_and_Down, 0);
		btfl.flyDirection(AUX1, -1000);
		
		vTaskDelay(5000);
		btfl.flyDirection(AUX1, 1000);
		vTaskDelay(2000);
		
		btfl.flyUnlock_Flag = 1;
		vTaskDelete(NULL);
	}
	void Lock(void *arg){
		btfl.flyReset();
		btfl.flyDirection(AUX1, -1000);
		vTaskDelete(NULL);
	}
	void mytaskloop(void *arg){
		while (1) {
			static unsigned long preview = millis();
			if (millis() - preview >= 14) {
				preview = millis();
				size_t sentBytes = Serial1.write(sendData, sizeof(sendData));
				sentBytes = sentBytes;
				vTaskDelay(5);
			}
		}
	}
}

namespace BTFL_ENV
{
    using namespace BTFLErr;
	using namespace BTFLC;
	using BTFLNamespace::btfl;
	using BTFLStruct::BTFLReturnVal;
}

#endif