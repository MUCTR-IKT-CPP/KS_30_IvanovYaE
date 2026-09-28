#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>

using namespace std;

struct SocialMediaProfile {
    string username;
    int age;
    int number_of_friends;
    int registration_year;
    bool is_premium;
    int last_login;
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

/**
 * Приводит латинскую букву к нижнему регистру.
 *
 * @param symbol исходный символ.
 * @return символ в нижнем регистре, если это латиница, иначе исходный символ.
 */
char toLowerChar(char symbol) {
    if(symbol >= 'A' && symbol <= 'Z') {
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
int compareIgnoreCase(string const& a, string const& b) {
    int i = 0;
    int size_a = (int)a.size();
    int size_b = (int)b.size();
    while(i < size_a && i < size_b) {
        char ca = toLowerChar(a[i]);
        char cb = toLowerChar(b[i]);
        if(ca < cb) {
            return -1;
        }
        if(ca > cb) {
            return 1;
        }
        i++;
    }
    if(size_a < size_b) {
        return -1;
    }
    if(size_a > size_b) {
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
void printProfile(SocialMediaProfile const& profile) {
    cout << profile.username
         << " | возраст: " << profile.age
         << " | друзья: " << profile.number_of_friends
         << " | год: " << profile.registration_year
         << " | премиум: ";
    if(profile.is_premium) {
        cout << "да";
    } else {
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
void printProfiles(SocialMediaProfile const* p_profiles, int n) {
    if(n == 0) {
        cout << "Список пуст." << endl;
        return;
    }
    for(int i = 0; i < n; i++) {
        cout << i << ". ";
        printProfile(p_profiles[i]);
    }
}

/**
 * Генерирует уникальное имя пользователя.
 *
 * @param index порядковый номер профиля.
 * @return строка с именем.
 */
string makeUsername(int index) {
    char const* bases[10] = {
        "Alice", "bob", "Charlie", "diana",
        "Eve", "frank", "Grace", "henry",
        "Ivy", "jack"
    };
    string name = bases[index % 10];
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
void fillRandomProfiles(SocialMediaProfile* p_profiles, int n) {
    for(int i = 0; i < n; i++) {
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
) {
    int count = 0;
    for(int i = 0; i < n; i++) {
        if(p_profiles[i].last_login > x) {
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
) {
    *p_avg_age = 0;
    *p_avg_friends = 0;
    *p_premium_count = 0;
    if(n <= 0) {
        return;
    }

    long sum_age = 0;
    long sum_friends = 0;
    int premium_count = 0;
    for(int i = 0; i < n; i++) {
        sum_age += p_profiles[i].age;
        sum_friends += p_profiles[i].number_of_friends;
        if(p_profiles[i].is_premium) {
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
void sortByFriends(SocialMediaProfile* p_profiles, int n) {
    for(int i = 0; i < n - 1; i++) {
        for(int j = 0; j < n - 1 - i; j++) {
            if(p_profiles[j].number_of_friends < p_profiles[j + 1].number_of_friends) {
                SocialMediaProfile temp = p_profiles[j];
                p_profiles[j] = p_profiles[j + 1];
                p_profiles[j + 1] = temp;
            }
        }
    }
}


void someFunc(int* p2, int const n){
    while(n = 1)
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
) {
    int count = 0;
    for(int i = 0; i < n; i++) {
        if(p_profiles[i].registration_year <= year) {
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
void sortByUsername(SocialMediaProfile* p_profiles, int n) {
    for(int i = 0; i < n - 1; i++) {
        for(int j = 0; j < n - 1 - i; j++) {
            if(compareIgnoreCase(p_profiles[j].username, p_profiles[j + 1].username) > 0) {
                SocialMediaProfile temp = p_profiles[j];
                p_profiles[j] = p_profiles[j + 1];
                p_profiles[j + 1] = temp;
            }
        }
    }
}

/**
 * Имитирует отправку уведомления пользователям,
 * которые не заходили больше 30 дней.
 *
 * @param p_profiles указатель на массив профилей.
 * @param n количество элементов.
 * @return число пользователей, которым «отправлено» уведомление.
 */
int sendNotifications(SocialMediaProfile const* p_profiles, int n) {
    int count = 0;
    cout << "Уведомления пользователям без входа более "
         << NOTIFY_DAYS << " дней:" << endl;
    for(int i = 0; i < n; i++) {
        if(p_profiles[i].last_login > NOTIFY_DAYS) {
            cout << "Отправлено @" << p_profiles[i].username
                 << " (не был(а) " << p_profiles[i].last_login << " дн.)" << endl;
            count++;
        }
    }
    if(count == 0) {
        cout << "Подходящих пользователей нет." << endl;
    }
    return count;
}

/**
 * Печатает меню операций.
 *
 * @return ничего не возвращает.
 */
void printMenu() {
    cout << endl;
    cout << "Выберите действие:" << endl;
    cout << "1. Показать всех пользователей" << endl;
    cout << "2. Поиск по активности (не заходили более X дней)" << endl;
    cout << "3. Анализ аудитории" << endl;
    cout << "4. Поиск старожилов" << endl;
    cout << "5. Сортировка по имени (без учёта регистра)" << endl;
    cout << "6. Отправка уведомлений (не заходили более 30 дней)" << endl;
    cout << "0. Выход" << endl;
    cout << "Ваш выбор: ";
}

int main() {
    srand((unsigned int)time(0));

    int n = 0;
    cout << "Введите N (число пользователей): ";
    cin >> n;
    if(n <= 0) {
        cout << "N должно быть положительным." << endl;
        return 1;
    }

    vector<SocialMediaProfile> profiles(n);
    fillRandomProfiles(&profiles[0], n);

    cout << endl << "Сгенерировано пользователей: " << n << endl;
    printProfiles(&profiles[0], n);

    int choice = -1;
    while(true) {
        printMenu();
        cin >> choice;

        if(choice == 0) {
            break;
        } else if(choice == 1) {
            printProfiles(&profiles[0], n);
        } else if(choice == 2) {
            int x = 0;
            cout << "Введите X (дней): ";
            cin >> x;
            vector<SocialMediaProfile> found(n);
            int count = findInactiveUsers(&profiles[0], n, x, &found[0]);
            cout << "Найдено пользователей: " << count << endl;
            printProfiles(&found[0], count);
        } else if(choice == 3) {
            double avg_age = 0;
            double avg_friends = 0;
            int premium_count = 0;
            analyzeAudience(&profiles[0], n, &avg_age, &avg_friends, &premium_count);
            cout << "Средний возраст: " << avg_age << endl;
            cout << "Среднее число друзей: " << avg_friends << endl;
            cout << "Премиум-пользователей: " << premium_count << endl;
        } else if(choice == 4) {
            int year = 0;
            cout << "Введите год (зарегистрированы не позднее): ";
            cin >> year;
            vector<SocialMediaProfile> old_timers(n);
            int count = findOldTimers(&profiles[0], n, year, &old_timers[0]);
            cout << "Найдено старожилов: " << count << endl;
            printProfiles(&old_timers[0], count);
        } else if(choice == 5) {
            sortByUsername(&profiles[0], n);
            cout << "Массив отсортирован по имени." << endl;
            printProfiles(&profiles[0], n);
        } else if(choice == 6) {
            int sent = sendNotifications(&profiles[0], n);
            cout << "Всего уведомлений: " << sent << endl;
        } else {
            cout << "Неизвестный пункт меню." << endl;
        }
    }

    return 0;
}
