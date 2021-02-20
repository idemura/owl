#include "foundation/call_stack.h"
#include "owl/hash_map.h"
#include "owl/tree_map.h"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <map>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

static long skey_compare(const skey_t *a, const skey_t *b)
{
    return a->nk - b->nk;
}

static uint64_t skey_hash(const skey_t *a)
{
    return a->nk;
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

constexpr size_t N = 250'000;
constexpr size_t CYCLES = 20;

void benchmark_tree_map()
{
    std::cout << "Testing tree_map\n";

    std::vector<long> time_owl;
    std::vector<long> time_std;

    for (int c = 0; c < CYCLES; c++) {
        const auto start_time = std::chrono::high_resolution_clock::now();

        std::map<long, int> m;
        std::minstd_rand rg;
        for (size_t i = 0; i < N; i++) {
            m[rg()] = i;
        }

        std::chrono::duration<double, std::micro> duration =
                std::chrono::high_resolution_clock::now() - start_time;
        time_std.push_back(duration.count());
    }

    for (int c = 0; c < CYCLES; c++) {
        const auto start_time = std::chrono::high_resolution_clock::now();

        tree_map m = tree_map_new(skey_compare, get_memmgr(), nullptr, sizeof(int));
        std::minstd_rand rg;
        for (size_t i = 0; i < N; i++) {
            *(int*) tree_map_put(&m, skey_number(rg())) = i;
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

        std::unordered_map<long, int> m;
        std::minstd_rand rg;
        for (size_t i = 0; i < N; i++) {
            m[rg()] = i;
        }

        std::chrono::duration<double, std::micro> duration =
                std::chrono::high_resolution_clock::now() - start_time;
        time_std.push_back(duration.count());
    }

    for (int c = 0; c < CYCLES; c++) {
        const auto start_time = std::chrono::high_resolution_clock::now();

        hash_map m = hash_map_new(
            skey_compare, skey_hash, get_memmgr(), nullptr, sizeof(int), 0);
        std::minstd_rand rg;
        for (size_t i = 0; i < N; i++) {
            *(int*) hash_map_put(&m, skey_number(rg())) = i;
        }

        std::chrono::duration<double, std::micro> duration =
                std::chrono::high_resolution_clock::now() - start_time;
        time_owl.push_back(duration.count());

        hash_map_destroy(&m);
    }

    std::cout << "std: " << name_duration(get_average(time_std)) << "\n";
    std::cout << "owl hash_map: " << name_duration(get_average(time_owl)) << "\n";
}

int main(int argc, char **argv)
{
    call_stack_init(argv[0]);

    benchmark_tree_map();
    std::cout<<"\n";

    benchmark_hash_map();
    std::cout<<"\n";

    return 0;
}
