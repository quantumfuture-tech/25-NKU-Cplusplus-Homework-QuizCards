#include "Debug.h"

int NumberedDebug::counter = 0;

NumberedDebug::NumberedDebug()
    : debug(qDebug())
{
    // 输出编号，不额外添加引号
    debug.noquote() << "[" << ++counter << "] ";
}