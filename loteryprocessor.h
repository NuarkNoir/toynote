#ifndef LOTERYPROCESSOR_H
#define LOTERYPROCESSOR_H

#include <QString>


namespace nuarkd {
    class LoteryProcessor;
}

class nuarkd::LoteryProcessor {
private:
    static const int array_size = 8;  // 03194159_7 -> 7+1
    QString prizes[array_size] = {
        "Toyota Supra '97",                 // 1
        "Trip to the ♂Gym♂",                // 2
        "Anime Dakimakura Pillow",          // 3
        "20 mg. of anti-matter",            // 4
        "Violent clock",                    // 5
        "Beard oil",                        // 6
        "Pocket universe 2.0",              // 7
        "♂Dungeon master's♂ phone number"   // 8
    };

public:
    //! Функция отвечает за получение приза.
    std::pair<bool, QString> obtainPrize();
};

#endif // LOTERYPROCESSOR_H
