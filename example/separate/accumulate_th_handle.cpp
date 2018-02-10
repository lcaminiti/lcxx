
#include <thread>
#include <chrono>
#include <vector>
#include <algorithm>
#include <numeric>
#include <functional>
#include <iterator>
#include <iostream>
#include <cassert>

template<typename Iter, typename T>
struct accumulator {
    void operator()(Iter first, Iter last, T& result) {
        result = std::accumulate(first, last, result);
    }
};

template<typename Iter, typename T>
T concurrent_accumulate(Iter first, Iter last, T init) {
    unsigned long const len = std::distance(first, last);
    if(!len) return init;

    unsigned long const min_per_block = 25;
    unsigned long const max_blocks = (len + min_per_block - 1) / min_per_block;
    unsigned long const hw_conc = std::thread::hardware_concurrency();
    unsigned long const num_blocks = std::min(hw_conc != 0 ? hw_conc : 2,
            max_blocks);
    unsigned long const block_size = len / num_blocks;

    std::vector<T> results(num_blocks);
    std::vector<std::thread> ths(num_blocks - 1);

    Iter block_start = first;
    for(unsigned long i = 0; i < (num_blocks - 1); ++i) {
        Iter block_end = block_start;
        std::advance(block_end, block_size);
        ths[i] = std::thread(accumulator<Iter, T>(), block_start, block_end,
                std::ref(results[i]));
        block_start = block_end;
    }
    accumulator<Iter, T>()(block_start, last, results[num_blocks - 1]);

    std::for_each(ths.begin(), ths.end(), std::mem_fn(&std::thread::join));
    return std::accumulate(results.begin(), results.end(), init);
}

int main() {
    std::vector<int> v(1e6, 1);

    std::chrono::high_resolution_clock::time_point s =
            std::chrono::high_resolution_clock::now();
    int r = std::accumulate(v.begin(), v.end(), 10);
    std::chrono::high_resolution_clock::time_point e =
            std::chrono::high_resolution_clock::now();
    auto d = std::chrono::duration_cast<std::chrono::microseconds>(
            e - s).count();
    std::cout << "result: " << r << std::endl;
    std::cout << "duration: " << d << " microsec" << std::endl;

    std::cout << std::endl;

    s = std::chrono::high_resolution_clock::now();
    int conc_r = concurrent_accumulate(v.begin(), v.end(), 10);
    e = std::chrono::high_resolution_clock::now();
    auto conc_d = std::chrono::duration_cast<std::chrono::microseconds>(
            e - s).count();
    std::cout << "concurrent result: " << conc_r << std::endl;
    std::cout << "concurrent duration: " << conc_d << " microsec" << std::endl;
    
    assert(conc_r == r);
    // In average, expect conc_d < d.
    return 0;
}

