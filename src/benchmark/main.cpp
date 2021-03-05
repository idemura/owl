#include "foundation/hash_map.h"
#include "foundation/tree_map.h"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <map>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

struct key_value {
    int k = 0;
    int v = 0;

    key_value(int k_, int v_): k{k_}, v{v_} {}
};

static long skey_compare(skey_t a, skey_t b)
{
    return *((const int *) a.ptr) - *((const int *) b.ptr);
}

static uint64_t skey_hash(skey_t a)
{
    return *((const int *) a.ptr);
}

static long get_average(std::vector<long> &times)
{
    std::sort(times.begin(), times.end());

    size_t margin = times.size() >= 6 ? 1 : 0;

    double sum = 0;
    for (size_t i = margin; i < times.size() - margin; i++) {
        sum += times[i];
    }

    return sum / (times.size() - 2 * margin);
}

static std::string name_duration(long us)
{
    if (us < 20'000) {
        return std::to_string(us) + " us";
    }
    us /= 1000;
    if (us < 20'000) {
        return std::to_string(us) + " ms";
    }
    us /= 1000;
    return std::to_string(us) + " s";
}

constexpr int N = 250'000;
constexpr int CYCLES = 20;

void benchmark_tree_map()
{
    std::cout << "Testing tree_map\n";

    std::vector<long> time_owl;
    std::vector<long> time_std;

    for (int c = 0; c < CYCLES; c++) {
        const auto start_time = std::chrono::high_resolution_clock::now();

        std::map<int, int> m;
        std::minstd_rand rg0;
        for (int i = 0; i < N; i++) {
            m[rg0()] = i;
        }
        std::minstd_rand rg1;
        for (int i = 0; i < N; i++) {
            m.erase(rg1());
        }
        if (m.size() != 0) {
            die("std map not empty");
        }

        std::chrono::duration<double, std::micro> duration =
                std::chrono::high_resolution_clock::now() - start_time;
        time_std.push_back(duration.count());
    }

    for (int c = 0; c < CYCLES; c++) {
        const auto start_time = std::chrono::high_resolution_clock::now();

        tree_map m = tree_map_new_default(skey_compare, sizeof(key_value));
        std::minstd_rand rg0;
        for (int i = 0; i < N; i++) {
            tree_map_put_v(&m, key_value(rg0(), i));
        }
        std::minstd_rand rg1;
        for (int i = 0; i < N; i++) {
            tree_map_del_v(&m, key_value(rg1(), 0));
        }
        if (tree_map_size(&m) != 0) {
            die("owl map not empty");
        }

        std::chrono::duration<double, std::micro> duration =
                std::chrono::high_resolution_clock::now() - start_time;
        time_owl.push_back(duration.count());

        tree_map_destroy(&m);
    }

    std::cout << "std: " << name_duration(get_average(time_std)) << "\n";
    std::cout << "owl tree_map: " << name_duration(get_average(time_owl)) << "\n";
}

void benchmark_hash_map()
{
    std::cout << "Testing hash_map\n";

    std::vector<long> time_owl;
    std::vector<long> time_std;

    for (int c = 0; c < CYCLES; c++) {
        const auto start_time = std::chrono::high_resolution_clock::now();

        std::unordered_map<int, int> m;
        std::minstd_rand rg;
        for (int i = 0; i < N; i++) {
            m[rg()] = i;
        }

        std::chrono::duration<double, std::micro> duration =
                std::chrono::high_resolution_clock::now() - start_time;
        time_std.push_back(duration.count());
    }

    for (int c = 0; c < CYCLES; c++) {
        const auto start_time = std::chrono::high_resolution_clock::now();

        hash_map m = hash_map_new_default(skey_compare, skey_hash, sizeof(key_value));
        std::minstd_rand rg0;
        for (int i = 0; i < N; i++) {
            hash_map_put_v(&m, key_value(rg0(), i));
        }

        std::chrono::duration<double, std::micro> duration =
                std::chrono::high_resolution_clock::now() - start_time;
        time_owl.push_back(duration.count());

        if (c == 0) {
            std::minstd_rand rg1;
            for (int i = 0; i < N; i++) {
                auto *kv = (key_value *) hash_map_get_v(&m, rg1());
                if (i != kv->v) {
                    die("Wrong value: %d vs %d", i, kv->v);
                }
            }
        }

        hash_map_destroy(&m);
    }

    std::cout << "std: " << name_duration(get_average(time_std)) << "\n";
    std::cout << "owl hash_map: " << name_duration(get_average(time_owl)) << "\n";
}

int main(int argc, char **argv)
{
    benchmark_tree_map();
    std::cout << "\n";

    benchmark_hash_map();
    std::cout << "\n";

    return 0;
}
