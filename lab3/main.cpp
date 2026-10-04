#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <sstream>
#include <limits>

using namespace std;

struct SocialMediaProfile{
    string username = "";
    int age = 0;
    int number_of_friends = 0;
    int registration_year = 0;
    bool is_premium = false;
    int last_login = 0;
};

int const MIN_AGE = 14;
int const MAX_AGE = 80;
int const MIN_FRIENDS = 0;
int const MAX_FRIENDS = 2000;
int const MIN_YEAR = 2010;
int const MAX_YEAR = 2026;
int const MIN_LAST_LOGIN = 0;
int const MAX_LAST_LOGIN = 400;
int const NOTIFY_DAYS = 30;
int const MAX_PROFILES = 100000;

/**
 * Приводит латинскую букву к нижнему регистру.
 *
 * @param symbol исходный символ.
 * @return символ в нижнем регистре, если это латиница, иначе исходный символ.
 */
char toLowerChar(char symbol){
    if(symbol >= 'A' && symbol <= 'Z'){
        return (char)(symbol - 'A' + 'a');
    }
    return symbol;
}

/**
 * Сравнивает две строки без учёта регистра.
 *
 * @param a первая строка.
 * @param b вторая строка.
 * @return -1 если a < b, 1 если a > b, 0 если строки равны.
 */
int compareIgnoreCase(string const& a, string const& b){
    size_t i = 0;
    size_t const SIZE_A = a.size();
    size_t const SIZE_B = b.size();
    while(i < SIZE_A && i < SIZE_B){
        char const CA = toLowerChar(a[i]);
        char const CB = toLowerChar(b[i]);
        if(CA < CB){
            return -1;
        }
        if(CA > CB){
            return 1;
        }
        i++;
    }
    if(SIZE_A < SIZE_B){
        return -1;
    }
    if(SIZE_A > SIZE_B){
        return 1;
    }
    return 0;
}

/**
 * Печатает один профиль пользователя.
 *
 * @param profile профиль для вывода.
 * @return ничего не возвращает.
 */
void printProfile(SocialMediaProfile const& profile){
    cout << profile.username
         << " | возраст: " << profile.age
         << " | друзья: " << profile.number_of_friends
         << " | год: " << profile.registration_year
         << " | премиум: ";
    if(profile.is_premium){
        cout << "да";
    } else{
        cout << "нет";
    }
    cout << " | дней с входа: " << profile.last_login << endl;
}

/**
 * Печатает массив профилей.
 *
 * @param p_profiles указатель на массив профилей.
 * @param n количество элементов.
 * @return ничего не возвращает.
 */
void printProfiles(SocialMediaProfile const* p_profiles, int n){
    if(n == 0){
        cout << "Список пуст." << endl;
        return;
    }
    for(int i = 0; i < n; i++){
        cout << i << ". ";
        printProfile(p_profiles[i]);
    }
}

/**
 * Генерирует случайное имя пользователя с уникальным числовым суффиксом.
 *
 * @param index порядковый номер профиля.
 * @return строка с именем.
 */
string makeUsername(int index){
    char const* const p_bases[10] = {
        "Alice", "bob", "Charlie", "diana",
        "Eve", "frank", "Grace", "henry",
        "Ivy", "jack"
    };
    string name = p_bases[rand() % 10];
    name += "_";
    name += to_string(index);
    return name;
}

/**
 * Заполняет массив профилей случайными данными в допустимых границах.
 *
 * @param p_profiles указатель на массив профилей.
 * @param n количество элементов.
 * @return ничего не возвращает.
 */
void fillRandomProfiles(SocialMediaProfile* p_profiles, int n){
    for(int i = 0; i < n; i++){
        p_profiles[i].username = makeUsername(i);
        p_profiles[i].age = MIN_AGE + rand() % (MAX_AGE - MIN_AGE + 1);
        p_profiles[i].number_of_friends = MIN_FRIENDS + rand() % (MAX_FRIENDS - MIN_FRIENDS + 1);
        p_profiles[i].registration_year = MIN_YEAR + rand() % (MAX_YEAR - MIN_YEAR + 1);
        p_profiles[i].is_premium = (rand() % 4 == 0);
        p_profiles[i].last_login = MIN_LAST_LOGIN + rand() % (MAX_LAST_LOGIN - MIN_LAST_LOGIN + 1);
    }
}

/**
 * Находит пользователей, которые не заходили в систему более x дней.
 *
 * @param p_profiles указатель на исходный массив.
 * @param n количество элементов.
 * @param x порог по дням без входа.
 * @param p_result массив, куда записываются найденные профили.
 * @return число найденных пользователей.
 */
int findInactiveUsers(
    SocialMediaProfile const* p_profiles,
    int n,
    int x,
    SocialMediaProfile* p_result
){
    int count = 0;
    for(int i = 0; i < n; i++){
        if(p_profiles[i].last_login > x){
            p_result[count] = p_profiles[i];
            count++;
        }
    }
    return count;
}

/**
 * Считает средний возраст, среднее число друзей и количество премиум-пользователей.
 *
 * @param p_profiles указатель на массив профилей.
 * @param n количество элементов.
 * @param p_avg_age указатель для среднего возраста.
 * @param p_avg_friends указатель для среднего числа друзей.
 * @param p_premium_count указатель для числа премиум-пользователей.
 * @return ничего не возвращает.
 */
void analyzeAudience(
    SocialMediaProfile const* p_profiles,
    int n,
    double* p_avg_age,
    double* p_avg_friends,
    int* p_premium_count
){
    *p_avg_age = 0;
    *p_avg_friends = 0;
    *p_premium_count = 0;
    if(n <= 0){
        return;
    }

    long long sum_age = 0;
    long long sum_friends = 0;
    int premium_count = 0;
    for(int i = 0; i < n; i++){
        sum_age += p_profiles[i].age;
        sum_friends += p_profiles[i].number_of_friends;
        if(p_profiles[i].is_premium){
            premium_count++;
        }
    }

    *p_avg_age = (double)sum_age / n;
    *p_avg_friends = (double)sum_friends / n;
    *p_premium_count = premium_count;
}

/**
 * Сортирует массив профилей по количеству друзей по убыванию.
 *
 * @param p_profiles указатель на массив профилей.
 * @param n количество элементов.
 * @return ничего не возвращает.
 */
void sortByFriends(SocialMediaProfile* p_profiles, int n){
    if(n < 2){
        return;
    }
    sort(p_profiles, p_profiles + n, [](SocialMediaProfile const& a, SocialMediaProfile const& b){
        return a.number_of_friends > b.number_of_friends;
    });
}

/**
 * Формирует массив старожилов: год регистрации не больше заданного.
 * Затем сортирует результат по числу друзей.
 *
 * @param p_profiles указатель на исходный массив.
 * @param n количество элементов.
 * @param year верхняя граница года регистрации.
 * @param p_result массив для старожилов.
 * @return число найденных старожилов.
 */
int findOldTimers(
    SocialMediaProfile const* p_profiles,
    int n,
    int year,
    SocialMediaProfile* p_result
){
    int count = 0;
    for(int i = 0; i < n; i++){
        if(p_profiles[i].registration_year <= year){
            p_result[count] = p_profiles[i];
            count++;
        }
    }
    sortByFriends(p_result, count);
    return count;
}

/**
 * Сортирует массив профилей по имени без учёта регистра.
 *
 * @param p_profiles указатель на массив профилей.
 * @param n количество элементов.
 * @return ничего не возвращает.
 */
void sortByUsername(SocialMediaProfile* p_profiles, int n){
    if(n < 2){
        return;
    }
    sort(p_profiles, p_profiles + n, [](SocialMediaProfile const& a, SocialMediaProfile const& b){
        return compareIgnoreCase(a.username, b.username) < 0;
    });
}

/**
 * Имитирует отправку уведомления пользователям,
 * которые не заходили больше 30 дней.
 *
 * @param p_profiles указатель на массив профилей.
 * @param n количество элементов.
 * @return число пользователей, которым «отправлено» уведомление.
 */
int sendNotifications(SocialMediaProfile const* p_profiles, int n){
    int count = 0;
    cout << "Уведомления пользователям без входа более "
         << NOTIFY_DAYS << " дней:" << endl;
    for(int i = 0; i < n; i++){
        if(p_profiles[i].last_login > NOTIFY_DAYS){
            cout << "Отправлено @" << p_profiles[i].username
                 << " (не был(а) " << p_profiles[i].last_login << " дн.)" << endl;
            count++;
        }
    }
    if(count == 0){
        cout << "Подходящих пользователей нет." << endl;
    }
    return count;
}

/**
 * Печатает меню операций.
 *
 * @return ничего не возвращает.
 */
void printMenu(){
    cout << endl;
    cout << "Выберите действие:" << endl;
    cout << "1. Показать всех пользователей" << endl;
    cout << "2. Поиск по активности (не заходили более X дней)" << endl;
    cout << "3. Анализ аудитории" << endl;
    cout << "4. Поиск старожилов" << endl;
    cout << "5. Сортировка по имени (без учёта регистра)" << endl;
    cout << "6. Отправка уведомлений (не заходили более 30 дней)" << endl;
    cout << "0. Выход" << endl;
}

/**
 * Читает одну строку с целым числом и проверяет допустимый диапазон.
 * Повторяет запрос при ошибке; при завершении ввода возвращает false.
 *
 * @param prompt текст запроса.
 * @param min_value минимальное допустимое число.
 * @param max_value максимальное допустимое число.
 * @param p_value указатель для результата.
 * @return true при успешном вводе, false при EOF или ошибке потока.
 */
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
        cout << "Введите целое число от " << min_value
             << " до " << max_value << "." << endl;
    }
}

int main(){
    srand((unsigned int)time(0));

    int n = 0;
    if(!readInteger("Введите N (число пользователей): ", 0, MAX_PROFILES, &n)){
        return 0;
    }

    vector<SocialMediaProfile> profiles(n);
    fillRandomProfiles(profiles.data(), n);

    cout << endl << "Сгенерировано пользователей: " << n << endl;
    printProfiles(profiles.data(), n);

    int choice = -1;
    while(true){
        printMenu();
        if(!readInteger("Ваш выбор: ", 0, 6, &choice)){
            break;
        }

        if(choice == 0){
            break;
        } else if(choice == 1){
            printProfiles(profiles.data(), n);
        } else if(choice == 2){
            int x = 0;
            if(!readInteger("Введите X (дней): ", 0, numeric_limits<int>::max(), &x)){
                break;
            }
            vector<SocialMediaProfile> found(n);
            int const COUNT = findInactiveUsers(profiles.data(), n, x, found.data());
            cout << "Найдено пользователей: " << COUNT << endl;
            printProfiles(found.data(), COUNT);
        } else if(choice == 3){
            double avg_age = 0;
            double avg_friends = 0;
            int premium_count = 0;
            analyzeAudience(profiles.data(), n, &avg_age, &avg_friends, &premium_count);
            cout << "Средний возраст: " << avg_age << endl;
            cout << "Среднее число друзей: " << avg_friends << endl;
            cout << "Премиум-пользователей: " << premium_count << endl;
        } else if(choice == 4){
            int year = 0;
            if(!readInteger("Введите год (зарегистрированы не позднее): ", 1, numeric_limits<int>::max(), &year)){
                break;
            }
            vector<SocialMediaProfile> old_timers(n);
            int const COUNT = findOldTimers(profiles.data(), n, year, old_timers.data());
            cout << "Найдено старожилов: " << COUNT << endl;
            printProfiles(old_timers.data(), COUNT);
        } else if(choice == 5){
            sortByUsername(profiles.data(), n);
            cout << "Массив отсортирован по имени." << endl;
            printProfiles(profiles.data(), n);
        } else if(choice == 6){
            int const SENT = sendNotifications(profiles.data(), n);
            cout << "Всего уведомлений: " << SENT << endl;
        } else{
            cout << "Неизвестный пункт меню." << endl;
        }
    }

    return 0;
}
