#include <iostream>
#include <cstdlib>
#include <ctime>

using namespace std;

int const MAX_N = 1000;

/*
 * Заполняет массив случайными целыми числами от 0 до 9.
 *
 * @param p_arr указатель на массив.
 * @param n количество элементов.
 * @return ничего не возвращает.
 */
void fillRandom(int* p_arr, int const n) {
    for(int i = 0; i < n; i++) {
        p_arr[i] = rand() % 10;
    }
}

/*
 * Считывает массив с клавиатуры.
 *
 * @param p_arr указатель на массив.
 * @param n количество элементов.
 * @return ничего не возвращает.
 */
void fillManual(int* p_arr, int const n) {
    cout << "Введите " << n << " чисел: ";
    for(int i = 0; i < n; i++) {
        cin >> p_arr[i];
    }
}

/*
 * Печатает массив в терминал.
 *
 * @param p_arr указатель на массив.
 * @param n количество элементов.
 * @return ничего не возвращает.
 */
void printArray(int const* p_arr, int const n) {
    cout << "[";
    for(int i = 0; i < n; i++) {
        cout << p_arr[i];
        if(i + 1 < n) {
            cout << ", ";
        }
    }
    cout << "]" << endl;
}

/*
 * Проверяет, является ли массив палиндромом.
 * Сравнение идёт с концов к центру через локальные индексы.
 *
 * @param p_arr указатель на массив.
 * @param n количество элементов.
 * @return 1 если массив палиндром, иначе 0.
 */
int isPalindrome(int const* p_arr, int const n) {
    int left = 0;
    int right = n - 1;
    while(left < right) {
        if(p_arr[left] != p_arr[right]) {
            return 0;
        }
        left++;
        right--;
    }
    return 1;
}

/*
 * Находит максимальную разницу соседних элементов.
 * Разница берётся по модулю.
 *
 * @param p_arr указатель на массив.
 * @param n количество элементов.
 * @return максимальная разница соседних элементов.
 */
int findMaxDiff(int const* p_arr, int const n) {
    int max_diff = 0;
    if(n < 2) {
        return max_diff;
    }
    max_diff = p_arr[1] - p_arr[0];
    if(max_diff < 0) {
        max_diff = -max_diff;
    }
    for(int i = 1; i < n - 1; i++) {
        int diff = p_arr[i + 1] - p_arr[i];
        if(diff < 0) {
            diff = -diff;
        }
        if(diff > max_diff) {
            max_diff = diff;
        }
    }
    return max_diff;
}

/*
 * Находит минимальную разницу соседних элементов.
 * Разница берётся по модулю.
 *
 * @param p_arr указатель на массив.
 * @param n количество элементов.
 * @return минимальная разница соседних элементов.
 */
int findMinDiff(int const* p_arr, int const n) {
    int min_diff = 0;
    if(n < 2) {
        return min_diff;
    }
    min_diff = p_arr[1] - p_arr[0];
    if(min_diff < 0) {
        min_diff = -min_diff;
    }
    for(int i = 1; i < n - 1; i++) {
        int diff = p_arr[i + 1] - p_arr[i];
        if(diff < 0) {
            diff = -diff;
        }
        if(diff < min_diff) {
            min_diff = diff;
        }
    }
    return min_diff;
}

int main() {
    srand((unsigned int)time(0));

    int n = 0;
    cout << "Введите N: ";
    cin >> n;

    if(n <= 0 || n > MAX_N) {
        cout << "N должно быть от 1 до " << MAX_N << "." << endl;
        return 1;
    }

    int arr[MAX_N];
    int* p_arr = arr;

    int fill_mode = 1;
    cout << "Заполнить случайно (1) или вручную (2): ";
    cin >> fill_mode;

    if(fill_mode == 2) {
        fillManual(p_arr, n);
    } else {
        fillRandom(p_arr, n);
    }

    cout << "Ввод: ";
    printArray(p_arr, n);

    if(isPalindrome(p_arr, n) == 1) {
        cout << "Вывод: Палиндром" << endl;
    } else {
        cout << "Вывод: Не палиндром" << endl;
    }

    cout << "Макс. разница: " << findMaxDiff(p_arr, n) << endl;
    cout << "Мин. разница: " << findMinDiff(p_arr, n) << endl;

    return 0;
}
