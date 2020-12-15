//
// Created by Daniel Garcia on 12/14/20.
//

#include "day14.h"
#include "utilities.h"

#include <utility>
#include <variant>
#include <string_view>
#include <charconv>
#include <vector>
#include <iostream>
#include <unordered_map>
#include <numeric>
#include <cmath>

namespace fs = std::filesystem;

namespace aoc2020 {

    namespace {

        constexpr uint64_t WORD_BITS = 0x0fffffffff;

        using memory_map = std::unordered_map<std::size_t, uint64_t>;

        struct mask {
            uint64_t clear = WORD_BITS;
            uint64_t set = 0;
            std::vector<uint64_t> floating;

            mask() = default;
            mask(uint64_t c, uint64_t s, std::vector<uint64_t>&& f) : clear{c}, set{s}, floating{std::move(f)} {}

            [[nodiscard]] uint64_t apply(uint64_t value) const {
                return (value | set) & clear;
            }

            static void apply_floating_array(memory_map& memory, const std::size_t base, const int64_t val, const uint64_t* masks, std::size_t size) {
                if (size == 0) {
                    memory[base] = val;
                }
                else {
                    apply_floating_array(memory, base | *masks, val, masks + 1, size - 1);
                    apply_floating_array(memory, base, val, masks + 1, size - 1);
                }
            }

            void apply_floating(memory_map& memory, const std::size_t base, const int64_t val) const {
                const auto set_base = (base | set) & (~clear | set);
                apply_floating_array(memory, set_base, val, floating.data(), floating.size());
            }
        };

        struct set {
            std::size_t address = 0;
            uint64_t value = 0;
        };

        using instruction = std::variant<mask, set>;

        bool starts_with(std::string_view str, std::string_view prefix) {
            if (prefix.size() <= str.size()) {
                const auto end = prefix.end();
                auto p = prefix.begin(), s = str.begin();
                while (p != end) {
                    if (*p != *s) {
                        return false;
                    }
                    ++p;
                    ++s;
                }
                return true;
            }
            else {
                return false;
            }
        }

        mask parse_mask(std::string_view data) {
            auto start = data.find_first_not_of(" =");
            if (start == std::string_view::npos) {
                throw std::runtime_error{"Invalid mask instruction."};
            }
            uint64_t clear = 0, set = 0;
            std::vector<uint64_t> floating_bits;
            floating_bits.reserve(36);//Worst case
            data.remove_prefix(start);
            const auto begin = data.begin(), end = data.end();
            for (auto p = begin; p != end; ++p) {
                clear <<= 1;
                set <<= 1;
                if (*p == '1') {
                    set |= 1;
                }
                else if (*p == '0') {
                    clear |= 1;
                }
                else if (*p == 'X') {
                    floating_bits.push_back(1ull << (35 - (p - begin)));
                }
                else {
                    throw std::runtime_error{"Invalid mask instruction bit."};
                }
            }
            return {~clear & WORD_BITS, set, std::move(floating_bits)};
        }

        set parse_set(std::string_view data) {
            if (data.empty()) {
                throw std::runtime_error{"Invalid empty memory instruction."};
            }
            else if (*data.begin() != '[') {
                throw std::runtime_error{"Invalid memory instruction address begin."};
            }
            data.remove_prefix(1);
            std::size_t address = 0;
            auto res = std::from_chars(data.data(), data.data() + data.size(), address);
            if (res.ec != std::errc{}) {
                throw std::system_error{std::make_error_code(res.ec)};
            }
            else if (*res.ptr != ']') {
                throw std::runtime_error{"Invalid memory instruction address end."};
            }
            data.remove_prefix(res.ptr - data.data() + 1);
            auto start = data.find_first_not_of(" =");
            if (start == std::string_view::npos) {
                throw std::runtime_error{"Invalid memory instruction."};
            }
            data.remove_prefix(start);
            uint64_t value = 0;
            res = std::from_chars(data.data(), data.data() + data.size(), value);
            if (res.ec != std::errc{}) {
                throw std::system_error{std::make_error_code(res.ec)};
            }
            return {address, value};
        }

        instruction parse_instruction(std::string_view line) {
            using namespace std::string_view_literals;
            if (starts_with(line, "mask"sv)) {
                line.remove_prefix(4);
                return parse_mask(line);
            }
            else if (starts_with(line, "mem"sv)) {
                line.remove_prefix(3);
                return parse_set(line);
            }
            else {
                throw std::runtime_error{"Unknown instruction."};
            }
        }

        std::vector<instruction> get_input(const fs::path &input_dir) {
            auto lines = aoc::read_file_lines(input_dir / "2020" / "day_14_input.txt");
            std::vector<instruction> retval;
            retval.reserve(lines.size());
            std::transform(lines.begin(), lines.end(), std::back_inserter(retval), parse_instruction);
            return retval;
        }

        struct decoder_v1 {
            mask current;
            memory_map memory;

            void operator()(mask ins) { current = std::move(ins); }
            void operator()(set ins) { memory[ins.address] = current.apply(ins.value); }

            void apply(const instruction& ins) {
                std::visit(*this, ins);
            }
        };

        struct decoder_v2 {
            mask current;
            memory_map memory;

            void operator()(mask ins) { current = std::move(ins); }
            void operator()(set ins) { current.apply_floating(memory, ins.address, ins.value); }

            void apply(const instruction& ins) {
                std::visit(*this, ins);
            }
        };

    }

    /*
    As your ferry approaches the sea port, the captain asks for your help again. The computer system that runs this port isn't compatible with the docking program on the ferry, so the docking parameters aren't being correctly initialized in the docking program's memory.

    After a brief inspection, you discover that the sea port's computer system uses a strange bitmask system in its initialization program. Although you don't have the correct decoder chip handy, you can emulate it in software!

    The initialization program (your puzzle input) can either update the bitmask or write a value to memory. Values and memory addresses are both 36-bit unsigned integers. For example, ignoring bitmasks for a moment, a line like mem[8] = 11 would write the value 11 to memory address 8.

    The bitmask is always given as a string of 36 bits, written with the most significant bit (representing 2^35) on the left and the least significant bit (2^0, that is, the 1s bit) on the right. The current bitmask is applied to values immediately before they are written to memory: a 0 or 1 overwrites the corresponding bit in the value, while an X leaves the bit in the value unchanged.

    For example, consider the following program:

    mask = XXXXXXXXXXXXXXXXXXXXXXXXXXXXX1XXXX0X
    mem[8] = 11
    mem[7] = 101
    mem[8] = 0
    This program starts by specifying a bitmask (mask = ....). The mask it specifies will overwrite two bits in every written value: the 2s bit is overwritten with 0, and the 64s bit is overwritten with 1.

    The program then attempts to write the value 11 to memory address 8. By expanding everything out to individual bits, the mask is applied as follows:

    value:  000000000000000000000000000000001011  (decimal 11)
    mask:   XXXXXXXXXXXXXXXXXXXXXXXXXXXXX1XXXX0X
    result: 000000000000000000000000000001001001  (decimal 73)
    So, because of the mask, the value 73 is written to memory address 8 instead. Then, the program tries to write 101 to address 7:

    value:  000000000000000000000000000001100101  (decimal 101)
    mask:   XXXXXXXXXXXXXXXXXXXXXXXXXXXXX1XXXX0X
    result: 000000000000000000000000000001100101  (decimal 101)
    This time, the mask has no effect, as the bits it overwrote were already the values the mask tried to set. Finally, the program tries to write 0 to address 8:

    value:  000000000000000000000000000000000000  (decimal 0)
    mask:   XXXXXXXXXXXXXXXXXXXXXXXXXXXXX1XXXX0X
    result: 000000000000000000000000000001000000  (decimal 64)
    64 is written to address 8 instead, overwriting the value that was there previously.

    To initialize your ferry's docking program, you need the sum of all values left in memory after the initialization program completes. (The entire 36-bit address space begins initialized to the value 0 at every address.) In the above example, only two values in memory are not zero - 101 (at address 7) and 64 (at address 8) - producing a sum of 165.

    Execute the initialization program. What is the sum of all values left in memory after it completes?
    */
    void solve_day_14_1(const std::filesystem::path& input_dir) {
        auto instructions = get_input(input_dir);
        decoder_v1 s;
        for (const auto& ins : instructions) {
            s.apply(ins);
        }
        uint64_t sum = std::accumulate(s.memory.begin(), s.memory.end(), 0ULL,
                                       [](uint64_t acc, const std::pair<std::size_t, uint64_t>& v){ return acc + v.second; });
        std::cout << '\t' << sum << '\n';
    }

    /*
    For some reason, the sea port's computer system still can't communicate with your ferry's docking program. It must be using version 2 of the decoder chip!

    A version 2 decoder chip doesn't modify the values being written at all. Instead, it acts as a memory address decoder. Immediately before a value is written to memory, each bit in the bitmask modifies the corresponding bit of the destination memory address in the following way:

    If the bitmask bit is 0, the corresponding memory address bit is unchanged.
    If the bitmask bit is 1, the corresponding memory address bit is overwritten with 1.
    If the bitmask bit is X, the corresponding memory address bit is floating.
    A floating bit is not connected to anything and instead fluctuates unpredictably. In practice, this means the floating bits will take on all possible values, potentially causing many memory addresses to be written all at once!

    For example, consider the following program:

    mask = 000000000000000000000000000000X1001X
    mem[42] = 100
    mask = 00000000000000000000000000000000X0XX
    mem[26] = 1
    When this program goes to write to memory address 42, it first applies the bitmask:

    address: 000000000000000000000000000000101010  (decimal 42)
    mask:    000000000000000000000000000000X1001X
    result:  000000000000000000000000000000X1101X
    After applying the mask, four bits are overwritten, three of which are different, and two of which are floating. Floating bits take on every possible combination of values; with two floating bits, four actual memory addresses are written:

    000000000000000000000000000000011010  (decimal 26)
    000000000000000000000000000000011011  (decimal 27)
    000000000000000000000000000000111010  (decimal 58)
    000000000000000000000000000000111011  (decimal 59)
    Next, the program is about to write to memory address 26 with a different bitmask:

    address: 000000000000000000000000000000011010  (decimal 26)
    mask:    00000000000000000000000000000000X0XX
    result:  00000000000000000000000000000001X0XX
    This results in an address with three floating bits, causing writes to eight memory addresses:

    000000000000000000000000000000010000  (decimal 16)
    000000000000000000000000000000010001  (decimal 17)
    000000000000000000000000000000010010  (decimal 18)
    000000000000000000000000000000010011  (decimal 19)
    000000000000000000000000000000011000  (decimal 24)
    000000000000000000000000000000011001  (decimal 25)
    000000000000000000000000000000011010  (decimal 26)
    000000000000000000000000000000011011  (decimal 27)
    The entire 36-bit address space still begins initialized to the value 0 at every address, and you still need the sum of all values left in memory at the end of the program. In this example, the sum is 208.

    Execute the initialization program using an emulator for a version 2 decoder chip. What is the sum of all values left in memory after it completes?
    */
    void solve_day_14_2(const std::filesystem::path& input_dir) {
        auto instructions = get_input(input_dir);
        decoder_v2 s;
        for (const auto& ins : instructions) {
            s.apply(ins);
        }
        uint64_t sum = std::accumulate(s.memory.begin(), s.memory.end(), 0ULL,
                                       [](uint64_t acc, const std::pair<std::size_t, uint64_t>& v){ return acc + v.second; });
        std::cout << '\t' << sum << '\n';
    }

} /* namespace aoc2020 */