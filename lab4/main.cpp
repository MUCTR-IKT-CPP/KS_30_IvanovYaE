#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <stdexcept>
#include <utility>

using namespace std;

// Все источники фиктивные: хранят только количество элементов.
struct CSVData{
    int rows = 0;
};

struct JSONData{
    int items = 0;
};

struct XMLData{
    int nodes = 0;
};

enum class DataSourceType { CSV, JSON, XML };

/** Возвращает название формата; неизвестный тип считается ошибкой. */
string typeName(DataSourceType type){
    switch(type){
        case DataSourceType::CSV: return "CSV";
        case DataSourceType::JSON: return "JSON";
        case DataSourceType::XML: return "XML";
    }
    throw invalid_argument("Неизвестный тип источника.");
}

// Класс единолично владеет объектом через void*.
// Тип определяет, к какой структуре приводить указатель перед чтением и delete.
class DataSource{
private:
    void* _p_source_data = nullptr;
    DataSourceType _type = DataSourceType::CSV;
    string _name = "";

    /** Создаёт объект нужного типа, проверяя количество элементов. */
    static void* _createData(DataSourceType type, int count){
        if(count < 0){
            throw invalid_argument("Количество элементов не может быть отрицательным.");
        }
        switch(type){
            case DataSourceType::CSV: return new CSVData{count};
            case DataSourceType::JSON: return new JSONData{count};
            case DataSourceType::XML: return new XMLData{count};
        }
        throw invalid_argument("Неизвестный тип источника.");
    }

    /** Освобождает память через исходный тип объекта, а не через void*. */
    void _releaseData() noexcept{
        switch(_type){
            case DataSourceType::CSV:
                delete static_cast<CSVData*>(_p_source_data);
                break;
            case DataSourceType::JSON:
                delete static_cast<JSONData*>(_p_source_data);
                break;
            case DataSourceType::XML:
                delete static_cast<XMLData*>(_p_source_data);
                break;
        }
        _p_source_data = nullptr;
    }

    /** Обменивает состояние без выделения памяти. */
    void _swap(DataSource& other) noexcept{
        swap(_p_source_data, other._p_source_data);
        swap(_type, other._type);
        _name.swap(other._name);
    }

public:
    /** Создаёт пустой, ещё не загруженный источник. */
    DataSource() = default;

    /** Создаёт именованный источник заданного типа и размера. */
    DataSource(DataSourceType type, string name, int count)
        : _type(type), _name(std::move(name)){
        if(_name.empty()){
            throw invalid_argument("Имя источника не должно быть пустым.");
        }
        _p_source_data = _createData(_type, count);
    }

    /** Глубокое копирование: копия получает собственный объект в памяти. */
    DataSource(DataSource const& other)
        : _type(other._type), _name(other._name){
        if(other.isLoaded()){
            _p_source_data = _createData(_type, other.getCount());
        }
    }

    /** Копирует через временный объект; ошибка не изменяет текущий источник. */
    DataSource& operator=(DataSource const& other){
        if(this != &other){
            DataSource copy(other);
            _swap(copy);
        }
        return *this;
    }

    /** Передаёт владение и оставляет исходный объект пустым. */
    DataSource(DataSource&& other) noexcept
        : _p_source_data(other._p_source_data), _type(other._type), _name(std::move(other._name)){
        other._p_source_data = nullptr;
        other._type = DataSourceType::CSV;
        other._name.clear();
    }

    /** Освобождает прежний объект и принимает владение от другого источника. */
    DataSource& operator=(DataSource&& other) noexcept{
        if(this != &other){
            _releaseData();
            _p_source_data = other._p_source_data;
            _type = other._type;
            _name = std::move(other._name);
            other._p_source_data = nullptr;
            other._type = DataSourceType::CSV;
            other._name.clear();
        }
        return *this;
    }

    /** Освобождает принадлежащий источнику объект. */
    ~DataSource(){
        _releaseData();
    }

    /** Проверяет, загружены ли фиктивные данные. */
    bool isLoaded() const{
        return _p_source_data != nullptr;
    }

    /** Возвращает формат источника. */
    DataSourceType getType() const{
        return _type;
    }

    /** Возвращает имя источника без копирования строки. */
    string const& getName() const{
        return _name;
    }

    /** Читает количество элементов через соответствующий тип структуры. */
    int getCount() const{
        if(!isLoaded()){
            return 0;
        }
        switch(_type){
            case DataSourceType::CSV: return static_cast<CSVData const*>(_p_source_data)->rows;
            case DataSourceType::JSON: return static_cast<JSONData const*>(_p_source_data)->items;
            case DataSourceType::XML: return static_cast<XMLData const*>(_p_source_data)->nodes;
        }
        throw logic_error("Нарушен тип загруженного источника.");
    }

    /** Меняет фиктивный формат, сохраняя имя и количество элементов. */
    void convertTo(DataSourceType type){
        if(!isLoaded()){
            throw logic_error("Нельзя конвертировать незагруженный источник.");
        }
        // Сначала выделяем новую память: при ошибке старые данные сохранятся.
        void* p_new_data = _createData(type, getCount());
        _releaseData();
        _p_source_data = p_new_data;
        _type = type;
    }
};

class DataAnalyzer{
private:
    vector<DataSource> _sources = {};

public:
    /** Создаёт пустой каталог. */
    DataAnalyzer() = default;

    /** Создаёт каталог из источников с непустыми уникальными именами. */
    explicit DataAnalyzer(vector<DataSource> const& sources){
        for(DataSource const& source : sources){
            if(!source.isLoaded()){
                throw invalid_argument("В каталог можно добавлять только загруженные источники.");
            }
            loadSource(source.getType(), source.getName(), source.getCount());
        }
    }

    /** Копирует каталог; DataSource выполняет глубокое копирование. */
    DataAnalyzer(DataAnalyzer const& other) : _sources(other._sources){}

    /** Копирует каталог с сохранением старого состояния при ошибке. */
    DataAnalyzer& operator=(DataAnalyzer const& other){
        if(this != &other){
            DataAnalyzer copy(other);
            _sources.swap(copy._sources);
        }
        return *this;
    }

    /** Переносит каталог, оставляя исходный менеджер пустым. */
    DataAnalyzer(DataAnalyzer&& other) noexcept : _sources(std::move(other._sources)){
        other._sources.clear();
    }

    /** Переносит каталог и освобождает прежние источники. */
    DataAnalyzer& operator=(DataAnalyzer&& other) noexcept{
        if(this != &other){
            _sources = std::move(other._sources);
            other._sources.clear();
        }
        return *this;
    }

    /** vector вызывает деструктор каждого принадлежащего ему DataSource. */
    ~DataAnalyzer() = default;

    /** Возвращает число загруженных источников. */
    size_t getSize() const{
        return _sources.size();
    }

    /** Находит источник по точному имени; возвращает nullptr, если его нет. */
    DataSource const* findSource(string const& name) const{
        for(DataSource const& source : _sources){
            if(source.getName() == name){
                return &source;
            }
        }
        return nullptr;
    }

    /** Загружает фиктивный источник; одинаковые имена запрещены. */
    void loadSource(DataSourceType type, string const& name, int count){
        if(findSource(name) != nullptr){
            throw invalid_argument("Источник с таким именем уже существует.");
        }
        _sources.emplace_back(type, name, count);
    }

    /** Конвертирует существующий источник и выводит прежний и новый форматы. */
    bool convertSource(string const& name, DataSourceType type, ostream& output = cout){
        for(DataSource& source : _sources){
            if(source.getName() == name){
                string const OLD_TYPE = typeName(source.getType());
                source.convertTo(type);
                output << name << ": " << OLD_TYPE << " -> " << typeName(type) << endl;
                return true;
            }
        }
        return false;
    }

    /** Сравнивает количество элементов двух источников. */
    bool compareSources(string const& first, string const& second, ostream& output = cout) const{
        DataSource const* p_first = findSource(first);
        DataSource const* p_second = findSource(second);
        if(p_first == nullptr || p_second == nullptr){
            return false;
        }
        int const FIRST_COUNT = p_first->getCount();
        int const SECOND_COUNT = p_second->getCount();
        output << first << ": " << FIRST_COUNT << "; " << second << ": " << SECOND_COUNT << endl;
        if(FIRST_COUNT == SECOND_COUNT){
            output << "Количество элементов одинаковое." << endl;
        } else if(FIRST_COUNT > SECOND_COUNT){
            output << "Больше элементов в источнике " << first << "." << endl;
        } else{
            output << "Больше элементов в источнике " << second << "." << endl;
        }
        return true;
    }

    /** Удаляет источник по имени и освобождает принадлежащую ему память. */
    bool removeSource(string const& name){
        for(auto it = _sources.begin(); it != _sources.end(); ++it){
            if(it->getName() == name){
                _sources.erase(it);
                return true;
            }
        }
        return false;
    }

    /** Выводит имена, форматы и количество элементов всех источников. */
    void printReport(ostream& output = cout) const{
        output << "Источников: " << _sources.size() << endl;
        if(_sources.empty()){
            output << "Список пуст." << endl;
        }
        for(DataSource const& source : _sources){
            output << source.getName() << " | " << typeName(source.getType())
                   << " | элементов: " << source.getCount() << endl;
        }
    }
};

/** Читает целое число в диапазоне; EOF завершает запрос. */
bool readInteger(string const& prompt, int min_value, int max_value, int* p_value){
    string line = "";
    while(true){
        cout << prompt;
        if(!getline(cin, line)){
            return false;
        }
        istringstream input(line);
        int value = 0;
        if(input >> value){
            input >> ws;
            if(input.eof() && value >= min_value && value <= max_value){
                *p_value = value;
                return true;
            }
        }
        cout << "Введите целое число от " << min_value << " до " << max_value << "." << endl;
    }
}

/** Читает непустое имя, допускающее пробелы внутри строки. */
bool readName(string const& prompt, string* p_name){
    string line = "";
    while(true){
        cout << prompt;
        if(!getline(cin, line)){
            return false;
        }
        size_t const FIRST = line.find_first_not_of(" \t\r");
        if(FIRST != string::npos){
            size_t const LAST = line.find_last_not_of(" \t\r");
            *p_name = line.substr(FIRST, LAST - FIRST + 1);
            return true;
        }
        cout << "Имя не должно быть пустым." << endl;
    }
}

/** Читает один из трёх форматов. */
bool readType(DataSourceType* p_type){
    int choice = 0;
    if(!readInteger("Тип (1 — CSV, 2 — JSON, 3 — XML): ", 1, 3, &choice)){
        return false;
    }
    *p_type = static_cast<DataSourceType>(choice - 1);
    return true;
}

/** Показывает независимость копий и передачу владения при перемещении. */
void demonstrateCopies(){
    DataAnalyzer original(vector<DataSource>{
        DataSource(DataSourceType::CSV, "example", 10)
    });
    DataAnalyzer copied(original);
    DataAnalyzer assigned;
    assigned = original;
    copied.convertSource("example", DataSourceType::JSON);
    cout << "Оригинал после изменения копии:" << endl;
    original.printReport();
    cout << "Изменённая копия:" << endl;
    copied.printReport();
    DataAnalyzer moved(std::move(copied));
    DataAnalyzer move_assigned;
    move_assigned = std::move(assigned);
    cout << "Размеры после перемещения: исходные = " << copied.getSize()
         << ", " << assigned.getSize() << "; получатели = "
         << moved.getSize() << ", " << move_assigned.getSize() << endl;
}

/** Запускает меню работы с фиктивными источниками данных. */
#ifndef LAB4_TESTING
int main(){
    DataAnalyzer analyzer;
    int const MAX_ELEMENTS = 1000000;
    while(true){
        cout << "\n1. Загрузить источник\n2. Конвертировать источник"
             << "\n3. Сравнить источники\n4. Удалить источник"
             << "\n5. Отчёт\n6. Демонстрация копирования и перемещения\n0. Выход\n";
        int choice = 0;
        if(!readInteger("Ваш выбор: ", 0, 6, &choice) || choice == 0){
            break;
        }
        try{
            string name = "";
            DataSourceType type = DataSourceType::CSV;
            if(choice == 1){
                int count = 0;
                if(!readName("Имя источника: ", &name) || !readType(&type) ||
                   !readInteger("Количество элементов: ", 0, MAX_ELEMENTS, &count)){
                    break;
                }
                analyzer.loadSource(type, name, count);
                cout << "Источник загружен." << endl;
            } else if(choice == 2){
                if(!readName("Имя источника: ", &name) || !readType(&type)){
                    break;
                }
                if(!analyzer.convertSource(name, type)){
                    cout << "Источник не найден." << endl;
                }
            } else if(choice == 3){
                string second = "";
                if(!readName("Первый источник: ", &name) || !readName("Второй источник: ", &second)){
                    break;
                }
                if(!analyzer.compareSources(name, second)){
                    cout << "Один или оба источника не найдены." << endl;
                }
            } else if(choice == 4){
                if(!readName("Имя источника: ", &name)){
                    break;
                }
                cout << (analyzer.removeSource(name) ? "Источник удалён." : "Источник не найден.") << endl;
            } else if(choice == 5){
                analyzer.printReport();
            } else if(choice == 6){
                demonstrateCopies();
            }
        } catch(exception const& error){
            cout << "Ошибка: " << error.what() << endl;
        }
    }
    return 0;
}
#endif
