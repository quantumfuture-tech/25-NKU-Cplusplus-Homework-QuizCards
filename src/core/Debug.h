#pragma once
#include <QDebug>


class NumberedDebug {
    static int counter;                // 静态计数器，跨所有实例
    QDebug debug;                      // 内部的 QDebug 对象

public:
    NumberedDebug();
    ~NumberedDebug() = default;

    // 通用类型：完美转发到 QDebug
    template<typename T>
    NumberedDebug& operator<<(const T& value) {
        debug << value;
        return *this;
    }

    // 处理流操纵符（如 Qt::endl, qSetFieldWidth 等）
    NumberedDebug& operator<<(QTextStreamFunction f) {
        debug << f;
        return *this;
    }

    // 处理其他标准 manipulator（如果需要）
    NumberedDebug& operator<<(QTextStreamManipulator m) {
        debug << m;
        return *this;
    }
};

// 全局函数，模仿 qDebug() 的便捷用法
inline NumberedDebug myDebug() {
    return NumberedDebug();
}
