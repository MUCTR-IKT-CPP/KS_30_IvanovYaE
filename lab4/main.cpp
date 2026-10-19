#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <limits>

using namespace std;

// Все источники фиктивные: хранят только количество элементов.
struct CSVData{
    int rows;
};

struct JSONData{
    int items;
};

struct XMLData{
    int nodes;
};

enum class DataSourceType { CSV, JSON, XML };

// Тип источника определяет, как читать и удалять данные через void*.
class DataSource{
private:
    void* _p_source_data = nullptr;
    DataSourceType _type = DataSourceType::CSV;
    string _name = "";

    static void* _createData(DataSourceType type, int count);

    // Перед delete приводим указатель к типу созданного объекта.
    void _releaseData() noexcept;

    void _swap(DataSource& other) noexcept;

public:
    DataSource();

    DataSource(DataSourceType type, string name, int count);

    // Для копии выделяется отдельная память.
    DataSource(DataSource const& other);

    // Временная копия сохранит старые данные, если выделение памяти не удастся.
    DataSource& operator=(DataSource const& other);

    // При перемещении передаём указатель, а исходный объект оставляем пустым.
    DataSource(DataSource&& other) noexcept;

    DataSource& operator=(DataSource&& other) noexcept;

    ~DataSource();

    bool isLoaded() const;

    DataSourceType getType() const;

    string const& getName() const;

    int getCount() const;

    void convertTo(DataSourceType type);
};

class DataAnalyzer{
private:
    vector<DataSource> _sources = {};

public:
    DataAnalyzer();

    explicit DataAnalyzer(vector<DataSource> const& sources);

    // При копировании vector вызывает конструктор копирования DataSource.
    DataAnalyzer(DataAnalyzer const& other);

    DataAnalyzer& operator=(DataAnalyzer const& other);

    DataAnalyzer(DataAnalyzer&& other) noexcept;

    DataAnalyzer& operator=(DataAnalyzer&& other) noexcept;

    // vector сам вызывает деструкторы источников.
    ~DataAnalyzer();

    size_t getSize() const;

    DataSource const* findSource(string const& name) const;

    void loadSource(DataSourceType type, string const& name, int count);

    bool convertSource(string const& name, DataSourceType type, ostream& output = cout);

    bool compareSources(string const& first, string const& second, ostream& output = cout) const;

    bool removeSource(string const& name);

    void printReport(ostream& output = cout) const;
};

string typeName(DataSourceType type);
bool readInteger(string const& prompt, int min_value, int max_value, int& result);
bool readName(string const& prompt, string& name);
bool readType(DataSourceType& type);
void demonstrateCopies();

int main(){
    DataAnalyzer analyzer = {};
    int const MAX_ELEMENTS = numeric_limits<int>::max();
    while(true){
        cout << "\n1. Загрузить источник\n2. Конвертировать источник"
             << "\n3. Сравнить источники\n4. Удалить источник"
             << "\n5. Отчёт\n6. Демонстрация копирования и перемещения\n0. Выход\n";
        int choice = 0;
        if(!readInteger("Ваш выбор: ", 0, 6, choice) || choice == 0){
            break;
        }
        try{
            string name = "";
            DataSourceType type = DataSourceType::CSV;
            if(choice == 1){
                int count = 0;
                if(!readName("Имя источника: ", name) || !readType(type) ||
                   !readInteger("Количество элементов: ", 0, MAX_ELEMENTS, count)){
                    break;
                }
                analyzer.loadSource(type, name, count);
                cout << "Источник загружен." << endl;
            } else if(choice == 2){
                if(!readName("Имя источника: ", name) || !readType(type)){
                    break;
                }
                if(!analyzer.convertSource(name, type)){
                    cout << "Источник не найден." << endl;
                }
            } else if(choice == 3){
                string second = "";
                if(!readName("Первый источник: ", name) || !readName("Второй источник: ", second)){
                    break;
                }
                if(!analyzer.compareSources(name, second)){
                    cout << "Один или оба источника не найдены." << endl;
                }
            } else if(choice == 4){
                if(!readName("Имя источника: ", name)){
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

string typeName(DataSourceType type){
    switch(type){
        case DataSourceType::CSV: return "CSV";
        case DataSourceType::JSON: return "JSON";
        case DataSourceType::XML: return "XML";
    }
    throw invalid_argument("Неизвестный тип источника.");
}

void* DataSource::_createData(DataSourceType type, int count){
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

void DataSource::_releaseData() noexcept{
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

void DataSource::_swap(DataSource& other) noexcept{
    swap(_p_source_data, other._p_source_data);
    swap(_type, other._type);
    _name.swap(other._name);
}

DataSource::DataSource() = default;

DataSource::DataSource(DataSourceType type, string name, int count)
    : _type(type), _name(std::move(name)){
    if(_name.empty()){
        throw invalid_argument("Имя источника не должно быть пустым.");
    }
    _p_source_data = _createData(_type, count);
}

DataSource::DataSource(DataSource const& other)
    : _type(other._type), _name(other._name){
    if(other.isLoaded()){
        _p_source_data = _createData(_type, other.getCount());
    }
}

DataSource& DataSource::operator=(DataSource const& other){
    if(this != &other){
        DataSource copy(other);
        _swap(copy);
    }
    return *this;
}

DataSource::DataSource(DataSource&& other) noexcept
    : _p_source_data(other._p_source_data), _type(other._type), _name(std::move(other._name)){
    other._p_source_data = nullptr;
    other._type = DataSourceType::CSV;
    other._name.clear();
}

DataSource& DataSource::operator=(DataSource&& other) noexcept{
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

DataSource::~DataSource(){
    _releaseData();
}

bool DataSource::isLoaded() const{
    return _p_source_data != nullptr;
}

DataSourceType DataSource::getType() const{
    return _type;
}

string const& DataSource::getName() const{
    return _name;
}

int DataSource::getCount() const{
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

void DataSource::convertTo(DataSourceType type){
    if(!isLoaded()){
        throw logic_error("Нельзя конвертировать незагруженный источник.");
    }
    if(type == _type){
        return;
    }
    // Сначала выделяем новую память: при ошибке старые данные сохранятся.
    void* const p_NEW_DATA = _createData(type, getCount());
    _releaseData();
    _p_source_data = p_NEW_DATA;
    _type = type;
}

DataAnalyzer::DataAnalyzer() = default;

DataAnalyzer::DataAnalyzer(vector<DataSource> const& sources){
    for(DataSource const& source : sources){
        if(!source.isLoaded()){
            throw invalid_argument("В каталог можно добавлять только загруженные источники.");
        }
        loadSource(source.getType(), source.getName(), source.getCount());
    }
}

DataAnalyzer::DataAnalyzer(DataAnalyzer const& other) : _sources(other._sources){}

DataAnalyzer& DataAnalyzer::operator=(DataAnalyzer const& other){
    if(this != &other){
        DataAnalyzer copy(other);
        _sources.swap(copy._sources);
    }
    return *this;
}

DataAnalyzer::DataAnalyzer(DataAnalyzer&& other) noexcept : _sources(std::move(other._sources)){
    other._sources.clear();
}

DataAnalyzer& DataAnalyzer::operator=(DataAnalyzer&& other) noexcept{
    if(this != &other){
        _sources = std::move(other._sources);
        other._sources.clear();
    }
    return *this;
}

DataAnalyzer::~DataAnalyzer() = default;

size_t DataAnalyzer::getSize() const{
    return _sources.size();
}

DataSource const* DataAnalyzer::findSource(string const& name) const{
    for(DataSource const& source : _sources){
        if(source.getName() == name){
            return &source;
        }
    }
    return nullptr;
}

void DataAnalyzer::loadSource(DataSourceType type, string const& name, int count){
    if(findSource(name) != nullptr){
        throw invalid_argument("Источник с таким именем уже существует.");
    }
    _sources.emplace_back(type, name, count);
}

bool DataAnalyzer::convertSource(string const& name, DataSourceType type, ostream& output){
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

bool DataAnalyzer::compareSources(string const& first, string const& second, ostream& output) const{
    DataSource const* const p_FIRST = findSource(first);
    DataSource const* const p_SECOND = findSource(second);
    if(p_FIRST == nullptr || p_SECOND == nullptr){
        return false;
    }
    int const FIRST_COUNT = p_FIRST->getCount();
    int const SECOND_COUNT = p_SECOND->getCount();
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

bool DataAnalyzer::removeSource(string const& name){
    for(auto it = _sources.begin(); it != _sources.end(); ++it){
        if(it->getName() == name){
            _sources.erase(it);
            return true;
        }
    }
    return false;
}

void DataAnalyzer::printReport(ostream& output) const{
    output << "Источников: " << _sources.size() << endl;
    if(_sources.empty()){
        output << "Список пуст." << endl;
    }
    for(DataSource const& source : _sources){
        output << source.getName() << " | " << typeName(source.getType())
               << " | элементов: " << source.getCount() << endl;
    }
}

// Читаем строку целиком, чтобы отклонять ввод вроде "12abc".
bool readInteger(string const& prompt, int min_value, int max_value, int& result){
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
                result = value;
                return true;
            }
        }
        cout << "Введите целое число от " << min_value << " до " << max_value << "." << endl;
    }
}

bool readName(string const& prompt, string& name){
    string line = "";
    while(true){
        cout << prompt;
        if(!getline(cin, line)){
            return false;
        }
        size_t const FIRST = line.find_first_not_of(" \t\r");
        if(FIRST != string::npos){
            size_t const LAST = line.find_last_not_of(" \t\r");
            name = line.substr(FIRST, LAST - FIRST + 1);
            return true;
        }
        cout << "Имя не должно быть пустым." << endl;
    }
}

bool readType(DataSourceType& type){
    int choice = 0;
    if(!readInteger("Тип (1 — CSV, 2 — JSON, 3 — XML): ", 1, 3, choice)){
        return false;
    }
    type = static_cast<DataSourceType>(choice - 1);
    return true;
}

void demonstrateCopies(){
    DataAnalyzer const ORIGINAL(vector<DataSource>{
        DataSource(DataSourceType::CSV, "example", 10)
    });
    DataAnalyzer copied(ORIGINAL);
    DataAnalyzer assigned = {};
    assigned = ORIGINAL;
    copied.convertSource("example", DataSourceType::JSON);
    cout << "Оригинал после изменения копии:" << endl;
    ORIGINAL.printReport();
    cout << "Изменённая копия:" << endl;
    copied.printReport();
    DataAnalyzer const MOVED(std::move(copied));
    DataAnalyzer move_assigned = {};
    move_assigned = std::move(assigned);
    cout << "Размеры после перемещения: исходные = " << copied.getSize()
         << ", " << assigned.getSize() << "; получатели = "
         << MOVED.getSize() << ", " << move_assigned.getSize() << endl;
}
