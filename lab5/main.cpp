#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <random>
#include <algorithm>
#include <stdexcept>
#include <limits>

using namespace std;

using Money = long long; // Все денежные суммы хранятся в копейках.

/** Возвращает сумму в рублях с двумя знаками после запятой. */
string moneyText(Money amount){
    ostringstream output;
    output << fixed << setprecision(2) << amount / 100.0;
    return output.str();
}

/** Генерирует целое число, включая обе границы диапазона. */
int randomInt(mt19937& generator, int minimum, int maximum){
    return uniform_int_distribution<int>(minimum, maximum)(generator);
}

struct Batch{
    int production_day;
    int quantity;
};

class MedicalDrug{
private:
    string _name;
    int _shelf_life;
    Money _price;
    Money _purchase_price;
    int _quantity;
    int _initial_quantity;
    bool _for_cold;
    vector<Batch> _batches;
    int _sold = 0;
    int _written_off = 0;
    Money _revenue = 0;

public:
    /** Создаёт препарат с одной начальной партией. */
    MedicalDrug(string const& name, int shelf_life, Money price, int quantity,
                int production_day, bool for_cold)
        : _name(name), _shelf_life(shelf_life), _price(price),
          _purchase_price(0), _quantity(quantity),
          _initial_quantity(quantity), _for_cold(for_cold){
        if(name.empty() || shelf_life < 30 || shelf_life > 365 ||
           price < 100 || price > 1000000 || quantity < 1 || quantity > 1000 ||
           production_day < 1 || production_day > 1000){
            throw invalid_argument("Некорректные параметры препарата.");
        }
        _purchase_price = price * 60 / 100;
        _batches.push_back({production_day, quantity});
    }

    /** Освобождает партии; памятью управляют string и vector. */
    ~MedicalDrug() = default;

    /** Возвращает название препарата. */
    string const& getName() const{ return _name; }
    /** Возвращает текущий остаток. */
    int getQuantity() const{ return _quantity; }
    /** Возвращает начальный целевой запас. */
    int getInitialQuantity() const{ return _initial_quantity; }
    /** Возвращает розничную цену в копейках. */
    Money getPrice() const{ return _price; }
    /** Возвращает закупочную цену в копейках. */
    Money getPurchasePrice() const{ return _purchase_price; }
    /** Определяет принадлежность к противопростудной группе. */
    bool isForCold() const{ return _for_cold; }
    /** Возвращает количество проданных упаковок. */
    int getSold() const{ return _sold; }
    /** Возвращает количество списанных упаковок. */
    int getWrittenOff() const{ return _written_off; }
    /** Возвращает накопленную выручку. */
    Money getRevenue() const{ return _revenue; }

    /** Проверяет строгое неравенство срока для отдельной партии. */
    bool checkExpiry(Batch const& batch, int current_day) const{
        return current_day >= batch.production_day &&
               current_day - batch.production_day < _shelf_life;
    }

    /** Проверяет наличие хотя бы одной годной упаковки. */
    bool checkExpiry(int current_day) const{
        for(Batch const& batch : _batches){
            if(batch.quantity > 0 && checkExpiry(batch, current_day)){
                return true;
            }
        }
        return false;
    }

    /** Удаляет просроченные партии и возвращает число списанных упаковок. */
    int writeOffExpired(int current_day){
        int removed = 0;
        for(auto batch = _batches.begin(); batch != _batches.end();){
            if(current_day >= batch->production_day && !checkExpiry(*batch, current_day)){
                removed += batch->quantity;
                batch = _batches.erase(batch);
            } else {
                ++batch;
            }
        }
        _quantity -= removed;
        _written_off += removed;
        return removed;
    }

    /** Добавляет свежую партию, сохраняя даты ранее поступивших упаковок. */
    void replenish(int quantity, int current_day){
        if(quantity <= 0 || quantity > _initial_quantity - _quantity ||
           current_day < 1 || current_day > 1000 ||
           (!_batches.empty() && current_day < _batches.back().production_day)){
            throw invalid_argument("Некорректные параметры поставки.");
        }
        _batches.push_back({current_day, quantity});
        _quantity += quantity;
    }

    /** Продаёт только годные упаковки, начиная с самых старых партий. */
    Money sell(int quantity, int current_day, int discount){
        if(quantity <= 0 || discount < 0 || discount > 15){
            throw invalid_argument("Некорректные параметры продажи.");
        }
        int available = 0;
        for(Batch const& batch : _batches){
            if(checkExpiry(batch, current_day)){
                available += batch.quantity;
            }
        }
        if(!checkExpiry(current_day) || available < quantity){
            return 0;
        }
        int remaining = quantity;
        for(Batch& batch : _batches){
            if(checkExpiry(batch, current_day)){
                int taken = min(remaining, batch.quantity);
                batch.quantity -= taken;
                remaining -= taken;
            }
            if(remaining == 0){
                break;
            }
        }
        _batches.erase(remove_if(_batches.begin(), _batches.end(),
            [](Batch const& batch){ return batch.quantity == 0; }), _batches.end());
        // Округляем цену одной упаковки до копейки, затем умножаем на количество.
        Money total = ((_price * (100 - discount) + 50) / 100) * quantity;
        _quantity -= quantity;
        _sold += quantity;
        _revenue += total;
        return total;
    }

    /** Выводит общие данные препарата и даты всех его партий. */
    friend ostream& operator<<(ostream& output, MedicalDrug const& drug){
        output << drug._name << " | срок: " << drug._shelf_life
               << " дней | цена: " << moneyText(drug._price)
               << " руб. | остаток: " << drug._quantity << " | партии:";
        if(drug._batches.empty()){
            output << " нет";
        }
        for(Batch const& batch : drug._batches){
            output << " [дата " << batch.production_day << ", " << batch.quantity << " уп.]";
        }
        return output;
    }
};

class Tablets : public MedicalDrug{
private:
    int _dosage;
public:
    /** Создаёт таблетки с дозировкой в миллиграммах. */
    Tablets(string const& name, int life, Money price, int quantity, int day,
            bool for_cold, int dosage)
        : MedicalDrug(name, life, price, quantity, day, for_cold), _dosage(dosage){
        if(dosage <= 0){ throw invalid_argument("Дозировка должна быть положительной."); }
    }
    /** Имитирует применение таблеток без медицинских рекомендаций. */
    string apply() const{ return "Имитация применения таблеток."; }
    /** Возвращает правила хранения таблеток в учебной модели. */
    string storageRules() const{ return "Сухое место, 15-25 C."; }
    /** Выводит данные таблеток и их особые параметры. */
    friend ostream& operator<<(ostream& output, Tablets const& drug){
        return output << "Таблетки | " << static_cast<MedicalDrug const&>(drug)
                      << " | дозировка: " << drug._dosage << " мг | " << drug.storageRules();
    }
};

class Syrup : public MedicalDrug{
private:
    int _volume;
public:
    /** Создаёт сироп с объёмом в миллилитрах. */
    Syrup(string const& name, int life, Money price, int quantity, int day,
          bool for_cold, int volume)
        : MedicalDrug(name, life, price, quantity, day, for_cold), _volume(volume){
        if(volume <= 0){ throw invalid_argument("Объём должен быть положительным."); }
    }
    /** Имитирует применение сиропа. */
    string apply() const{ return "Имитация применения сиропа."; }
    /** Возвращает правила хранения сиропа в учебной модели. */
    string storageRules() const{ return "В защищённом от света месте, 8-15 C."; }
    /** Выводит данные сиропа и его особые параметры. */
    friend ostream& operator<<(ostream& output, Syrup const& drug){
        return output << "Сироп | " << static_cast<MedicalDrug const&>(drug)
                      << " | объём: " << drug._volume << " мл | " << drug.storageRules();
    }
};

class Ointment : public MedicalDrug{
private:
    int _weight;
public:
    /** Создаёт мазь с весом в граммах. */
    Ointment(string const& name, int life, Money price, int quantity, int day,
             bool for_cold, int weight)
        : MedicalDrug(name, life, price, quantity, day, for_cold), _weight(weight){
        if(weight <= 0){ throw invalid_argument("Вес должен быть положительным."); }
    }
    /** Имитирует применение мази. */
    string apply() const{ return "Имитация применения мази."; }
    /** Возвращает правила хранения мази в учебной модели. */
    string storageRules() const{ return "В прохладном месте, 2-8 C."; }
    /** Выводит данные мази и её особые параметры. */
    friend ostream& operator<<(ostream& output, Ointment const& drug){
        return output << "Мазь | " << static_cast<MedicalDrug const&>(drug)
                      << " | вес: " << drug._weight << " г | " << drug.storageRules();
    }
};

struct Sale{
    int day;
    string name;
    int quantity;
    int discount;
    Money total;
};

class Pharmacy{
private:
    vector<Tablets> _tablets;
    vector<Syrup> _syrups;
    vector<Ointment> _ointments;
    vector<Sale> _history;
    mt19937 _generator;
    int _next_day;
    Money _balance = 5000000;
    Money _purchases = 0;
    Money _revenue = 0;

    /** Обрабатывает списание и утреннюю поставку для одного типа препаратов. */
    template<class T>
    void prepareStock(vector<T>& drugs, int day){
        for(T& drug : drugs){
            int expired = drug.writeOffExpired(day);
            if(expired > 0){
                cout << "Списано: " << drug.getName() << " — " << expired << " уп.\n";
            }
            if(drug.getQuantity() * 10 < drug.getInitialQuantity()){
                int quantity = drug.getInitialQuantity() - drug.getQuantity();
                Money cost = drug.getPurchasePrice() * quantity;
                if(cost <= _balance){
                    drug.replenish(quantity, day);
                    _balance -= cost;
                    _purchases += cost;
                    cout << "Поставка: " << drug.getName() << " — " << quantity
                         << " уп., " << moneyText(cost) << " руб.\n";
                } else {
                    cout << "Недостаточно средств для поставки: " << drug.getName() << '\n';
                }
            }
        }
    }

    /** Печатает остатки через перегруженный оператор конкретного типа. */
    template<class T>
    void printStock(vector<T> const& drugs) const{
        for(T const& drug : drugs){
            cout << drug << '\n';
        }
    }

    /** Собирает временный список ссылок на препараты без передачи владения. */
    vector<MedicalDrug*> allDrugs(){
        vector<MedicalDrug*> drugs;
        for(Tablets& drug : _tablets){ drugs.push_back(&drug); }
        for(Syrup& drug : _syrups){ drugs.push_back(&drug); }
        for(Ointment& drug : _ointments){ drugs.push_back(&drug); }
        return drugs;
    }

    /** Выводит прибыль с учётом закупочной стоимости продаж и списаний. */
    template<class T>
    void printProfitability(vector<T> const& drugs) const{
        for(T const& drug : drugs){
            Money costs = drug.getPurchasePrice() * (drug.getSold() + drug.getWrittenOff());
            Money profit = drug.getRevenue() - costs;
            cout << drug.getName() << " | продано: " << drug.getSold()
                 << " | списано: " << drug.getWrittenOff()
                 << " | выручка: " << moneyText(drug.getRevenue())
                 << " | затраты: " << moneyText(costs)
                 << " | прибыль: " << moneyText(profit) << " руб. | рентабельность: ";
            if(costs == 0){
                cout << "не определена (нет затрат)";
            } else {
                cout << fixed << setprecision(2) << 100.0 * profit / costs << '%';
            }
            cout << '\n';
        }
    }

public:
    /** Создаёт случайные начальные запасы: 15-30 наименований каждого типа. */
    Pharmacy(int current_day, unsigned int seed) : _generator(seed), _next_day(current_day){
        if(current_day < 1 || current_day > 1000){
            throw invalid_argument("Дата должна быть от 1 до 1000.");
        }
        int tablets_count = randomInt(_generator, 15, 30);
        int syrups_count = randomInt(_generator, 15, 30);
        int ointments_count = randomInt(_generator, 15, 30);
        // По одному новому объекту за итерацию: диапазоны одинаковы для всех типов.
        for(int type = 0; type < 3; type++){
            int count = type == 0 ? tablets_count : (type == 1 ? syrups_count : ointments_count);
            for(int i = 0; i < count; i++){
                int life = randomInt(_generator, 30, 365);
                Money price = randomInt(_generator, 10000, 80000);
                int quantity = randomInt(_generator, 20, 60);
                int day = randomInt(_generator, 1, current_day);
                bool for_cold = i % 2 == 0;
                string name = (for_cold ? "Простуда-" : "Препарат-");
                name += (type == 0 ? "Т" : (type == 1 ? "С" : "М"));
                name += to_string(i + 1);
                int parameter = randomInt(_generator, 1, 10);
                if(type == 0){
                    _tablets.emplace_back(name, life, price, quantity, day, for_cold, parameter * 50);
                } else if(type == 1){
                    _syrups.emplace_back(name, life, price, quantity, day, for_cold, parameter * 25);
                } else {
                    _ointments.emplace_back(name, life, price, quantity, day, for_cold, parameter * 10);
                }
            }
        }
    }

    /** Контейнеры автоматически освобождают все препараты и историю. */
    ~Pharmacy() = default;

    /** Демонстрирует методы трёх наследников через статический полиморфизм. */
    template<class T>
    void demonstrate(T const& drug) const{
        cout << drug.apply() << " Хранение: " << drug.storageRules() << '\n';
    }

    /** Показывает начальные данные и демонстрацию разных типов. */
    void printInitialStock() const{
        cout << "\nНачальный баланс: " << moneyText(_balance) << " руб.\n"
             << "Наименований: таблетки — " << _tablets.size()
             << ", сиропы — " << _syrups.size() << ", мази — " << _ointments.size() << '\n';
        demonstrate(_tablets.front());
        demonstrate(_syrups.front());
        demonstrate(_ointments.front());
        printStock(_tablets);
        printStock(_syrups);
        printStock(_ointments);
    }

    /** Моделирует день: списание, пополнение, запросы покупателей и отчёт. */
    void simulateDay(int day){
        if(day < 1 || day > 1000 || day != _next_day){
            throw invalid_argument("Дни моделирования должны идти подряд в диапазоне 1-1000.");
        }
        int calendar_day = (day - 1) % 365 + 1;
        bool winter = calendar_day <= 59 || calendar_day >= 335;
        int discount = randomInt(_generator, 1, 100) <= 30 ? randomInt(_generator, 5, 15) : 0;
        cout << "\n=== День " << day << " | " << (winter ? "зима" : "обычный спрос")
             << " | скидка: " << discount << "% ===\nСписание и пополнение:\n";
        prepareStock(_tablets, day);
        prepareStock(_syrups, day);
        prepareStock(_ointments, day);

        vector<MedicalDrug*> drugs = allDrugs();
        vector<int> weights;
        for(MedicalDrug const* drug : drugs){
            weights.push_back(winter && drug->isForCold() ? 3 : 1);
        }
        discrete_distribution<size_t> select(weights.begin(), weights.end());
        int customers = randomInt(_generator, 5, 20);
        int refused = 0;
        Money daily_revenue = 0;
        size_t history_start = _history.size();
        for(int customer = 0; customer < customers; customer++){
            int requests = randomInt(_generator, 1, 3);
            // Один покупатель запрашивает разные наименования.
            vector<size_t> selected;
            for(int request = 0; request < requests; request++){
                size_t index = select(_generator);
                while(find(selected.begin(), selected.end(), index) != selected.end()){
                    index = select(_generator);
                }
                selected.push_back(index);
                MedicalDrug& drug = *drugs[index];
                int quantity = randomInt(_generator, 1, 3);
                Money total = drug.sell(quantity, day, discount);
                if(total == 0){
                    refused++;
                    continue;
                }
                _history.push_back({day, drug.getName(), quantity, discount, total});
                daily_revenue += total;
                _balance += total;
                _revenue += total;
            }
        }
        cout << "Покупателей: " << customers << ", отказов по запросам: " << refused
             << "\nПроданные препараты:\n";
        if(history_start == _history.size()){
            cout << "Продаж нет.\n";
        }
        for(size_t i = history_start; i < _history.size(); i++){
            Sale const& sale = _history[i];
            cout << "День " << sale.day << " | " << sale.name << " | " << sale.quantity
                 << " уп. | скидка " << sale.discount << "% | " << moneyText(sale.total) << " руб.\n";
        }
        cout << "Выручка за день: " << moneyText(daily_revenue)
             << " руб.\nБаланс: " << moneyText(_balance) << " руб.\nОстатки на складе:\n";
        printStock(_tablets);
        printStock(_syrups);
        printStock(_ointments);
        _next_day++;
    }

    /** Выводит итоги периода и рентабельность каждого наименования. */
    void printSummary() const{
        cout << "\n=== Итоги периода ===\nПродаж в истории: " << _history.size()
             << "\nОбщая выручка: " << moneyText(_revenue)
             << " руб.\nОплаченные поставки: " << moneyText(_purchases)
             << " руб.\nКонечный баланс: " << moneyText(_balance)
             << " руб.\nРентабельность по препаратам:\n";
        printProfitability(_tablets);
        printProfitability(_syrups);
        printProfitability(_ointments);
    }
};

/** Читает целое число, повторяя запрос при ошибке; EOF завершает ввод. */
bool readInteger(string const& prompt, int minimum, int maximum, int& result){
    string line;
    while(true){
        cout << prompt;
        if(!getline(cin, line)){
            return false;
        }
        istringstream input(line);
        int value = 0;
        char extra = '\0';
        if(input >> value && !(input >> extra) && value >= minimum && value <= maximum){
            result = value;
            return true;
        }
        cout << "Введите целое число от " << minimum << " до " << maximum << ".\n";
    }
}

/** Запускает консольную симуляцию на выбранное число дней, включая 90 дней. */
int main(){
    try{
        cout << "Лабораторная работа 5. Вариант 5. Аптечная система учёта.\n";
        int day = 0;
        int days = 0;
        int seed = 0;
        if(!readInteger("Начальная дата (1-1000): ", 1, 1000, day)){
            return 0;
        }
        int max_days = min(90, 1001 - day);
        if(!readInteger("Дней моделирования (1-" + to_string(max_days) + "): ", 1, max_days, days) ||
           !readInteger("Seed (0 — случайный): ", 0, numeric_limits<int>::max(), seed)){
            return 0;
        }
        unsigned int actual_seed = seed == 0
            ? random_device{}() % static_cast<unsigned int>(numeric_limits<int>::max()) + 1
            : static_cast<unsigned int>(seed);
        cout << "Seed: " << actual_seed << '\n';
        Pharmacy pharmacy(day, actual_seed);
        pharmacy.printInitialStock();
        for(int offset = 0; offset < days; offset++){
            pharmacy.simulateDay(day + offset);
        }
        pharmacy.printSummary();
    } catch(exception const& error){
        cerr << "Ошибка: " << error.what() << '\n';
        return 1;
    }
    return 0;
}
