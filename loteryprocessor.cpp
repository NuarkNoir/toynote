#include "loteryprocessor.h"

#include <QRandomGenerator>

std::pair<bool, QString> nuarkd::LoteryProcessor::obtainPrize() {
    // Генератор псевдо-случайных чисел
    QRandomGenerator r(time(nullptr));
    if (r.generateDouble() <= 0.4) {  // 8/20 = 0.4
        // bounded(highest) - возвращает число в пределах [0, highest)
        int pos = r.bounded(LoteryProcessor::array_size);
        return std::make_pair(true, this->prizes[pos]);
    }

    return std::make_pair(false, "nothingness");
}
