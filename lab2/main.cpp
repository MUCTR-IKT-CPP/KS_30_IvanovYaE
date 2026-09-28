#include <iostream>
#include <cstdlib>
#include <ctime>

using namespace std;

/**
 * Выделяет память под квадратную матрицу n x n типа int.
 * @param n размер стороны матрицы
 * @return указатель на массив указателей (динамический двумерный массив)
 */
int** allocateMap(int n){
    int** p_map = new int*[n];
    for(int i = 0; i < n; i++){
        p_map[i] = new int[n];
    }
    return p_map;
}

/**
 * Освобождает память, выделенную под матрицу n x n.
 * @param p_map указатель на матрицу
 * @param n размер стороны матрицы
 */
void freeMap(int** p_map, int n){
    if(p_map == 0){
        return;
    }
    for(int i = 0; i < n; i++){
        delete[] p_map[i];
    }
    delete[] p_map;
}

/**
 * Заполняет матрицу случайными значениями 0 и 1.
 * @param p_map указатель на матрицу
 * @param n размер стороны матрицы
 */
void fillRandom(int** p_map, int n){
    for(int i = 0; i < n; i++){
        for(int j = 0; j < n; j++){
            p_map[i][j] = rand() % 2;
        }
    }
}

/**
 * Заполняет матрицу нулями.
 * @param p_map указатель на матрицу
 * @param n размер стороны матрицы
 */
void fillZeros(int** p_map, int n){
    for(int i = 0; i < n; i++){
        for(int j = 0; j < n; j++){
            p_map[i][j] = 0;
        }
    }
}

/**
 * Выводит карту бухты в терминал.
 * @param p_map указатель на матрицу
 * @param n размер стороны матрицы
 */
void printMap(int** p_map, int n){
    cout << "Карта:" << endl;
    for(int i = 0; i < n; i++){
        for(int j = 0; j < n; j++){
            cout << p_map[i][j];
            if(j + 1 < n){
                cout << " ";
            }
        }
        cout << endl;
    }
}

/**
 * Рекурсивный обход одного корабля (клетки, соседние по стороне).
 * Помечает посещённые клетки в p_visited и возвращает размер корабля.
 * @param p_map указатель на матрицу
 * @param p_visited матрица посещённых клеток
 * @param n размер стороны матрицы
 * @param row текущая строка
 * @param col текущий столбец
 * @return число клеток текущего корабля
 */
int floodSize(int** p_map, int** p_visited, int n, int row, int col){
    if(row < 0 || col < 0 || row >= n || col >= n){
        return 0;
    }
    if(p_map[row][col] == 0 || p_visited[row][col] == 1){
        return 0;
    }

    p_visited[row][col] = 1;
    int size = 1;
    size += floodSize(p_map, p_visited, n, row - 1, col);
    size += floodSize(p_map, p_visited, n, row + 1, col);
    size += floodSize(p_map, p_visited, n, row, col - 1);
    size += floodSize(p_map, p_visited, n, row, col + 1);
    return size;
}

/**
 * Рекурсивно обнуляет все клетки корабля, содержащего точку (row, col).
 * @param p_map указатель на матрицу
 * @param n размер стороны матрицы
 * @param row текущая строка
 * @param col текущий столбец
 */
void floodClear(int** p_map, int n, int row, int col){
    if(row < 0 || col < 0 || row >= n || col >= n){
        return;
    }
    if(p_map[row][col] == 0){
        return;
    }

    p_map[row][col] = 0;
    floodClear(p_map, n, row - 1, col);
    floodClear(p_map, n, row + 1, col);
    floodClear(p_map, n, row, col - 1);
    floodClear(p_map, n, row, col + 1);
}

/**
 * Подсчитывает количество кораблей и их размеры.
 * Кораблём считается связная группа единиц (соседство по вертикали и горизонтали).
 * @param p_map указатель на матрицу
 * @param n размер стороны матрицы
 * @param p_sizes указатель на массив размеров кораблей
 * @param p_ship_count указатель, по которому записывается число кораблей
 */
void countShipsAndSizes(int** p_map, int n, int* p_sizes, int* p_ship_count){
    int** p_visited = allocateMap(n);
    fillZeros(p_visited, n);

    *p_ship_count = 0;
    for(int i = 0; i < n; i++){
        for(int j = 0; j < n; j++){
            if(p_map[i][j] == 1 && p_visited[i][j] == 0){
                int size = floodSize(p_map, p_visited, n, i, j);
                p_sizes[*p_ship_count] = size;
                (*p_ship_count)++;
            }
        }
    }

    freeMap(p_visited, n);
}

/**
 * Отражает карту бухты зеркально относительно вертикальной оси
 * (левая и правая стороны меняются местами).
 * @param p_map указатель на матрицу
 * @param n размер стороны матрицы
 */
void mirrorMap(int** p_map, int n){
    for(int i = 0; i < n; i++){
        for(int j = 0; j < n / 2; j++){
            int temp = p_map[i][j];
            p_map[i][j] = p_map[i][n - 1 - j];
            p_map[i][n - 1 - j] = temp;
        }
    }
}

/**
 * Удаляет корабль по указанным координатам.
 * Параметры n, row и col передаются указателями.
 * Если в клетке нет корабля, карта не изменяется.
 * @param p_map указатель на матрицу
 * @param p_n указатель на размер стороны матрицы
 * @param p_row указатель на номер строки
 * @param p_col указатель на номер столбца
 */
void deleteShip(int** p_map, int* p_n, int* p_row, int* p_col){
    if(p_n == 0 || p_row == 0 || p_col == 0){
        return;
    }
    if(*p_row < 0 || *p_col < 0 || *p_row >= *p_n || *p_col >= *p_n){
        cout << "Координаты выходят за границы карты." << endl;
        return;
    }
    if(p_map[*p_row][*p_col] == 0){
        cout << "В указанной клетке корабля нет." << endl;
        return;
    }

    floodClear(p_map, *p_n, *p_row, *p_col);
    cout << "Корабль удалён." << endl;
}

/**
 * Подсчитывает количество кораблей. Матрица передаётся через void*.
 * @param p_data указатель на матрицу int**, приведённый к void*
 * @param n размер стороны матрицы
 * @return количество кораблей на карте
 */
int countShipsVoid(void* p_data, int n){
    int** p_map = (int**)p_data;
    int* p_sizes = new int[n * n];
    int ship_count = 0;

    countShipsAndSizes(p_map, n, p_sizes, &ship_count);

    delete[] p_sizes;
    return ship_count;
}

/**
 * Печатает меню доступных операций.
 */
void printMenu(){
    cout << endl;
    cout << "Выберите действие:" << endl;
    cout << "1. Подсчитать количество кораблей и их размеры" << endl;
    cout << "2. Отразить карту бухты зеркально" << endl;
    cout << "3. Удалить корабль по указанным координатам" << endl;
    cout << "4. Подсчитать корабли через функцию с параметром void*" << endl;
    cout << "0. Выход" << endl;
    cout << "Ваш выбор: ";
}

int main(){
    srand((unsigned int)time(0));

    int n = 0;
    cout << "Введите N (размер карты N x N): ";
    cin >> n;

    if(n <= 0){
        cout << "Размер карты должен быть положительным числом." << endl;
        return 1;
    }

    int** p_map = allocateMap(n);
    fillRandom(p_map, n);

    cout << endl << "N = " << n << endl;
    printMap(p_map, n);

    int choice = -1;
    while(true){
        printMenu();
        cin >> choice;

        if(choice == 0){
            break;
        } else if(choice == 1){
            int* p_sizes = new int[n * n];
            int ship_count = 0;
            countShipsAndSizes(p_map, n, p_sizes, &ship_count);

            cout << "Количество кораблей = " << ship_count << endl;
            if(ship_count == 0){
                cout << "Размеры: нет кораблей" << endl;
            } else {
                cout << "Размеры: ";
                for(int i = 0; i < ship_count; i++){
                    cout << p_sizes[i];
                    if(i + 1 < ship_count){
                        cout << ", ";
                    }
                }
                cout << endl;
            }
            delete[] p_sizes;
        } else if(choice == 2){
            mirrorMap(p_map, n);
            cout << "Карта отражена зеркально." << endl;
            printMap(p_map, n);
        } else if(choice == 3){
            int row = 0;
            int col = 0;
            cout << "Введите строку и столбец (нумерация с 0): ";
            cin >> row >> col;
            deleteShip(p_map, &n, &row, &col);
            printMap(p_map, n);
        } else if(choice == 4){
            int ship_count = countShipsVoid((void*)p_map, n);
            cout << "Количество кораблей = " << ship_count << endl;
        } else {
            cout << "Неизвестный пункт меню." << endl;
        }
    }

    freeMap(p_map, n);
    return 0;
}
