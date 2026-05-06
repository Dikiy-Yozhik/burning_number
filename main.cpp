#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>

using namespace std;

const int N = 20; // Количество вершин
const uint32_t TARGET_MASK = (1 << N) - 1; //  (все 20 бит равны 1)
const int INF = 100; // условное верхнее значение при подсчете расстояний в графе

vector<vector<int>> graph6_parser(const string& line) {
    vector<vector<int>> res(N);
    int bit_count = 0;

    for (int col = 1; col < N; col++) {
        for (int row = 0; row < col; row++) {
            int ch_idx = bit_count/6 + 1;
            int bit_mask = 5 - bit_count%6;

            if (((line[ch_idx] - 63) & (1 << bit_mask)) != 0) {
                res[col].push_back(row);
                res[row].push_back(col); // так как матрица смежности симметрична
            }
            bit_count++;
        }
    }

    return res;
}

void make_adj_table(vector<vector<int>>& dist, vector<vector<int>>& adj) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            dist[i][j] = (i==j ? 0 : INF);
        }
    }

    for (int i = 0; i < N; i++) {
        for (int val : adj[i]) {
            dist[i][val] = 1;
        }
    }
}

void floyd_warshall(vector<vector<int>>& dist) {
    for (int x = 0; x < N; x++) {
        for (int y = 0; y < N; y++) {
            for (int z = 0; z < N; z++) {
                if (dist[x][z] + dist[z][y] < dist[x][y]) {
                    dist[x][y] = dist[x][z] + dist[z][y];
                }
            }
        }
    }
}

void burn_mask(vector<vector<uint32_t>>& masks, vector<vector<int>>& dist) {
    // row - индекс изначально подожженного узла
    // col - количество ходов
    // masks[row][col] - число, кодирующее сожженные вершины к концу ходов
    //      например 496 - 00111110000 - будут выжжены вершины 2, 3, 4, 5, 6
    for (int row = 0; row < N; row++) {
        for (int col = 0; col < N; col++) {
            uint32_t mask = 0;
            for (int val = 0; val < N; val++) {
                if (dist[row][val] <= col) {
                    mask |= (1 << val);
                }
            }
            masks[row][col] = mask;
        }
    }
}

bool can_burn(int b, int step, vector<vector<uint32_t>>& masks, uint32_t current_mask) {
    if (current_mask == TARGET_MASK) return true;
    if (step > b) return false;

    int radius = b - step;
    for (int node = 0; node < N; node++) {
        uint32_t next_mask = current_mask | masks[node][radius];
        if (current_mask == next_mask) continue;

        if (can_burn(b, step+1, masks, next_mask)) return true;
    }

    return false;
}

int burning_number(vector<vector<int>> adj) {
    // считаем расстояния
    vector<vector<int>> dist (N, vector<int>(N));
    make_adj_table(dist, adj);
    floyd_warshall(dist);

    //считаем маску
    vector<vector<uint32_t>> masks(N, vector<uint32_t>(N, 0));
    burn_mask(masks, dist);

    // основной цикл подсчета
    for (int b = 0; b < N; b++) {
        if (can_burn(b, 1, masks, 0)) {
            return b;
        }
    }

    return N-1; // если вдруг что-то пошло не так
}

int main() {
    // Открываем файл с деревьями 
    ifstream infile("trees20.g6");
    if (!infile.is_open()) {
        cerr << "Error: can't open file trees20.g6!" << endl;
        return 1;
    }

    string line;
    int stats[N] = {0}; // Массив для сбора статистики: индексы - это число выжигания
    int total_trees = 0;

    cout << "Starting to count ..." << endl;
    auto start_time = chrono::high_resolution_clock::now();

    // Читаем файл построчно
    while (getline(infile, line)) {
        if (line.empty()) continue; 

        vector<vector<int>> adj = graph6_parser(line);
        int b = burning_number(adj);
        
        stats[b]++;
        total_trees++;

        // Для отслеживания прогресса выводим каждые 50 000 деревьев
        if (total_trees % 50000 == 0) {
            auto current_time = chrono::high_resolution_clock::now();
            chrono::duration<double> elapsed = current_time - start_time;
            cout << "Processed: " << total_trees << " trees";
            cout << " | Time: " << elapsed.count() << " sec" << endl;
        }
    }

    auto end_time = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed = end_time - start_time;

    // Вывод результатов
    cout << "\n=== Results ===" << endl;
    cout << "The trees amount: " << total_trees << endl;
    cout << "Time spend: " << elapsed.count() << " seconds" << endl;
    cout << "Burning count statistics:" << endl;
    for (int i = 2; i <= 5; ++i) {
        if (stats[i] > 0) {
            cout << "b(T) = " << i << ": " << stats[i] << " trees" << endl;
        }
    }

    return 0;
}