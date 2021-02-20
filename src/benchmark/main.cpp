#include "owl/tree_map.h"

#include <chrono>
#include <iostream>
#include <map>
#include <random>

static long skey_compare(const skey_t *a, const skey_t *b)
{
    return a->nk - b->nk;
}

long get_average(const long *a, size_t a_n)
{
    double sum = 0;
    for (size_t i = 0; i < a_n; i++) {
        sum += a[i];
    }
    return sum / a_n;
}

void benchmark_tree_map()
{
    constexpr size_t N = 100'000;
    constexpr size_t CYCLES = 10;

    long time_tree_map[CYCLES] = {};
    long time_std[CYCLES] = {};

    for (int c = 0; c < CYCLES; c++) {
        {
            const auto start_time = std::chrono::high_resolution_clock::now();
            tree_map t = tree_map_new(skey_compare, get_memmgr(), nullptr, sizeof(int));
            std::minstd_rand rg;
            for (size_t i = 0; i < N; i++) {
                *(int*) tree_map_put(&t, skey_number(rg())) = i;
            }
            std::chrono::duration<double, std::micro> duration =
                    std::chrono::high_resolution_clock::now() - start_time;
            time_tree_map[c] = duration.count();
            tree_map_destroy(&t);
        }
        {
            const auto start_time = std::chrono::high_resolution_clock::now();
            std::map<long, int> m;
            std::minstd_rand rg;
            for (size_t i = 0; i < N; i++) {
                m[rg()] = i;
            }
            std::chrono::duration<double, std::micro> duration =
                    std::chrono::high_resolution_clock::now() - start_time;
            time_std[c] = duration.count();
        }
    }

    std::cout << "tree_map: " << get_average(time_tree_map, CYCLES) / 1000.0 << " ms\n";
    std::cout << "std: " << get_average(time_std, CYCLES) / 1000.0 << " ms\n";
}

int main(int argc, char **argv)
{
    benchmark_tree_map();
    return 0;
}
