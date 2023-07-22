#include <Arduino.h>
#include "BTFL_430.h"
using namespace BTFL_ENV;

ReturnVal RTN;

void setup() {
  RTN = btfl.setTXpin(12, 4); //设置sbus输出引脚：参数一：ESP有效输出引脚；参数二：ESP无用引脚
  RTN = btfl.allInit();       //初始化参数数值
  RTN = btfl.startTaskLoop(); //开始sbus数据输出
  RTN = btfl.flyUnlock();     //无人机解锁飞控
}

void loop() {
  if (btfl.flyUnlock_Flag == 1) {
    /*
    * 以下为飞控数值控制函数的五个示例：
    * 参数一：上下、前后、左右倾、左右旋、通道AUX1
    * 参数二：-100~100
    */
    RTN = btfl.flyDirection(Up_and_Down, -100);
    //RTN = btfl.flyDirection(Forward_and_Back, -100);
    //RTN = btfl.flyDirection(Left_and_Right, -100);
    //RTN = btfl.flyDirection(rLeft_and_rRight, -100);
    //RTN = btfl.flyDirection(AUX1, -100);
  }
}