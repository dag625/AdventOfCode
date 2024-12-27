//
// Created by Dan on 12/16/2021.
//

#include "registration.h"

#include <doctest/doctest.h>

#include <vector>
#include <charconv>
#include <span>

#include "utilities.h"
#include "ranges.h"
#include "sum.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;
    using namespace std::string_view_literals;

    constexpr auto INPUT = "005532447836402684AC7AB3801A800021F0961146B1007A1147C89440294D005C12D2A7BC992D3F4E50C72CDF29EECFD0ACD5CC016962099194002CE31C5D3005F401296CAF4B656A46B2DE5588015C913D8653A3A001B9C3C93D7AC672F4FF78C136532E6E0007FCDFA975A3004B002E69EC4FD2D32CDF3FFDDAF01C91FCA7B41700263818025A00B48DEF3DFB89D26C3281A200F4C5AF57582527BC1890042DE00B4B324DBA4FAFCE473EF7CC0802B59DA28580212B3BD99A78C8004EC300761DC128EE40086C4F8E50F0C01882D0FE29900A01C01C2C96F38FCBB3E18C96F38FCBB3E1BCC57E2AA0154EDEC45096712A64A2520C6401A9E80213D98562653D98562612A06C0143CB03C529B5D9FD87CBA64F88CA439EC5BB299718023800D3CE7A935F9EA884F5EFAE9E10079125AF39E80212330F93EC7DAD7A9D5C4002A24A806A0062019B6600730173640575A0147C60070011FCA005000F7080385800CBEE006800A30C023520077A401840004BAC00D7A001FB31AAD10CC016923DA00686769E019DA780D0022394854167C2A56FB75200D33801F696D5B922F98B68B64E02460054CAE900949401BB80021D0562344E00042A16C6B8253000600B78020200E44386B068401E8391661C4E14B804D3B6B27CFE98E73BCF55B65762C402768803F09620419100661EC2A8CE0008741A83917CC024970D9E718DD341640259D80200008444D8F713C401D88310E2EC9F20F3330E059009118019A8803F12A0FC6E1006E3744183D27312200D4AC01693F5A131C93F5A131C970D6008867379CD3221289B13D402492EE377917CACEDB3695AD61C939C7C10082597E3740E857396499EA31980293F4FD206B40123CEE27CFB64D5E57B9ACC7F993D9495444001C998E66B50896B0B90050D34DF3295289128E73070E00A4E7A389224323005E801049351952694C000"sv;

    struct packet {
        uint8_t version = 0;
        uint8_t type_id = 0;

        packet() = default;
        packet(uint8_t v, uint8_t id) : version{v}, type_id{id} {}

        virtual ~packet() = default;
        [[nodiscard]] virtual uint64_t value() const = 0;
        [[nodiscard]] virtual uint64_t sum_of_versions() const = 0;
    };

    using packet_ptr = std::unique_ptr<packet>;

    struct literal : public packet {
        uint64_t lit_value;

        literal() = default;
        literal(uint8_t v, uint8_t id, uint64_t val) : packet{v, id}, lit_value{val} {}

        [[nodiscard]] uint64_t value() const override { return lit_value; }
        [[nodiscard]] uint64_t sum_of_versions() const override { return version; }
    };

    struct op_packet : public packet {
        std::vector<packet_ptr> packets;

        op_packet() = default;
        op_packet(uint8_t v, uint8_t id, std::vector<packet_ptr> list) : packet{v, id}, packets{std::move(list)} {}

        [[nodiscard]] uint64_t value() const override {
            uint64_t retval = 0;
            switch (type_id) {
                case 0://Sum
                    retval = sum(packets, [](const packet_ptr& p){ return p->value(); });
                    break;
                case 1://Product
                    retval = 1;
                    for (const auto& p : packets) {
                        retval *= p->value();
                    }
                    break;
                case 2://Min
                    retval = (*std::min_element(packets.begin(), packets.end(), [](const packet_ptr& a, const packet_ptr& b){ return a->value() < b->value(); }))->value();
                    break;
                case 3://Max
                    retval = (*std::max_element(packets.begin(), packets.end(), [](const packet_ptr& a, const packet_ptr& b){ return a->value() < b->value(); }))->value();
                    break;
                //Boolean Ops on 2 Subs
                case 5://Greater Than
                    retval = packets[0]->value() > packets[1]->value() ? 1 : 0;
                    break;
                case 6://Less Than
                    retval = packets[0]->value() < packets[1]->value() ? 1 : 0;
                    break;
                case 7://Equal
                    retval = packets[0]->value() == packets[1]->value() ? 1 : 0;
                    break;
                default:
                    break;
            }
            return retval;
        }
        [[nodiscard]] uint64_t sum_of_versions() const override { return version + sum(packets, [](const packet_ptr& p){ return p->sum_of_versions(); }); }
    };

    std::vector<uint8_t> to_bytes(const std::string_view hex_str) {
        std::vector<uint8_t> retval;
        retval.reserve(hex_str.size() / 2 + 1);
        for (std::size_t idx = 0u; idx < hex_str.size(); idx += 2) {
            std::size_t end = std::min(idx + 2, hex_str.size());
            uint8_t val = 0;
            const auto res = std::from_chars(hex_str.data() + idx, hex_str.data() + end, val, 16);
            if (const auto ec = std::make_error_code(res.ec); ec) {
                throw std::system_error{ec};
            }
            retval.push_back(val);
        }
        return retval;
    }

    uint8_t create_mask(std::size_t ex_front, std::size_t ex_back) {
        uint8_t retval = 0u;
        for (auto i = ex_back; i < 8 - ex_front; ++i) {
            retval |= 0x1u << i;
        }
        return retval;
    }

    uint16_t consume_bits(std::span<const uint8_t>& bytes, std::size_t& bit_offset, std::size_t count) {
        const auto num_bits = bit_offset + count;
        const auto num_bytes = num_bits / 8 + (num_bits % 8 == 0 ? 0 : 1);
        if (bytes.size() < num_bytes) {
            return 0;
        }
        uint16_t retval = 0;
        while (true) {
            const auto available = 8 - bit_offset;
            if (count <= available) {
                const auto remaining = available - count;
                const auto mask = create_mask(bit_offset, remaining);
                retval <<= (8u - bit_offset - remaining);
                retval |= ((bytes.front() & mask) >> remaining);
                if (remaining) {
                    bit_offset = 8u - remaining;
                }
                else {
                    bytes = bytes.subspan(1);
                    bit_offset = 0;
                }
                break;
            }
            else {
                const auto mask = create_mask(bit_offset, 0);
                retval <<= (8u - bit_offset);
                retval |= (bytes.front() & mask);
                count -= available;
                bytes = bytes.subspan(1);
                bit_offset = 0;
            }
        }
        return retval;
    }

    packet_ptr parse_packet(std::span<const uint8_t>& bytes, std::size_t& bit_offset) {
        const auto ver = static_cast<uint8_t>(consume_bits(bytes, bit_offset, 3));
        const auto id = static_cast<uint8_t>(consume_bits(bytes, bit_offset, 3));
        if (id == 4) {
            uint64_t val = 0;
            while (true) {
                const auto part = consume_bits(bytes, bit_offset, 5);
                val <<= 4;
                val |= (part & 0x000fu);
                if (!(part & 0x0010u)) {
                    break;
                }
            }
            return std::make_unique<literal>(ver, id, val);
        }
        else {
            const auto len_type = consume_bits(bytes, bit_offset, 1);
            if (len_type) {
                const auto num_sub_packets = consume_bits(bytes, bit_offset, 11);
                std::vector<packet_ptr> list;
                list.reserve(num_sub_packets);
                for (int i = 0; i < num_sub_packets; ++i) {
                    list.push_back(parse_packet(bytes, bit_offset));
                }
                return std::make_unique<op_packet>(ver, id, std::move(list));
            }
            else {
                const auto num_sub_bits = consume_bits(bytes, bit_offset, 15);
                const uint8_t* start = bytes.data();
                const auto starting_bit = bit_offset;
                const auto ending_bit = starting_bit + num_sub_bits;
                const auto post_starting_bit = ending_bit % 8;
                const auto num_bytes = ending_bit / 8 + (post_starting_bit == 0 ? 0 : 1);
                const uint8_t* end = start + num_bytes - 1;//A bit of a fudge so we don't have to track the bits, and a valid packet is at least 2 bytes.
                std::vector<packet_ptr> list;
                list.reserve(num_bytes);    //This is an overestimate.
                while (bytes.data() < end) {    //Don't care about the bit because no packet can be less than a byte long.
                    list.push_back(parse_packet(bytes, bit_offset));
                }
                return std::make_unique<op_packet>(ver, id, std::move(list));
            }
        }
    }

    /*
    --- Day 16: Packet Decoder ---
    As you leave the cave and reach open waters, you receive a transmission from the Elves back on the ship.

    The transmission was sent using the Buoyancy Interchange Transmission System (BITS), a method of packing numeric expressions into a binary sequence. Your submarine's computer has saved the transmission in hexadecimal (your puzzle input).

    The first step of decoding the message is to convert the hexadecimal representation into binary. Each character of hexadecimal corresponds to four bits of binary data:

    0 = 0000
    1 = 0001
    2 = 0010
    3 = 0011
    4 = 0100
    5 = 0101
    6 = 0110
    7 = 0111
    8 = 1000
    9 = 1001
    A = 1010
    B = 1011
    C = 1100
    D = 1101
    E = 1110
    F = 1111
    The BITS transmission contains a single packet at its outermost layer which itself contains many other packets. The hexadecimal representation of this packet might encode a few extra 0 bits at the end; these are not part of the transmission and should be ignored.

    Every packet begins with a standard header: the first three bits encode the packet version, and the next three bits encode the packet type ID. These two values are numbers; all numbers encoded in any packet are represented as binary with the most significant bit first. For example, a version encoded as the binary sequence 100 represents the number 4.

    Packets with type ID 4 represent a literal value. Literal value packets encode a single binary number. To do this, the binary number is padded with leading zeroes until its length is a multiple of four bits, and then it is broken into groups of four bits. Each group is prefixed by a 1 bit except the last group, which is prefixed by a 0 bit. These groups of five bits immediately follow the packet header. For example, the hexadecimal string D2FE28 becomes:

    110100101111111000101000
    VVVTTTAAAAABBBBBCCCCC
    Below each bit is a label indicating its purpose:

    The three bits labeled V (110) are the packet version, 6.
    The three bits labeled T (100) are the packet type ID, 4, which means the packet is a literal value.
    The five bits labeled A (10111) start with a 1 (not the last group, keep reading) and contain the first four bits of the number, 0111.
    The five bits labeled B (11110) start with a 1 (not the last group, keep reading) and contain four more bits of the number, 1110.
    The five bits labeled C (00101) start with a 0 (last group, end of packet) and contain the last four bits of the number, 0101.
    The three unlabeled 0 bits at the end are extra due to the hexadecimal representation and should be ignored.
    So, this packet represents a literal value with binary representation 011111100101, which is 2021 in decimal.

    Every other type of packet (any packet with a type ID other than 4) represent an operator that performs some calculation on one or more sub-packets contained within. Right now, the specific operations aren't important; focus on parsing the hierarchy of sub-packets.

    An operator packet contains one or more packets. To indicate which subsequent binary data represents its sub-packets, an operator packet can use one of two modes indicated by the bit immediately after the packet header; this is called the length type ID:

    If the length type ID is 0, then the next 15 bits are a number that represents the total length in bits of the sub-packets contained by this packet.
    If the length type ID is 1, then the next 11 bits are a number that represents the number of sub-packets immediately contained by this packet.
    Finally, after the length type ID bit and the 15-bit or 11-bit field, the sub-packets appear.

    For example, here is an operator packet (hexadecimal string 38006F45291200) with length type ID 0 that contains two sub-packets:

    00111000000000000110111101000101001010010001001000000000
    VVVTTTILLLLLLLLLLLLLLLAAAAAAAAAAABBBBBBBBBBBBBBBB
    The three bits labeled V (001) are the packet version, 1.
    The three bits labeled T (110) are the packet type ID, 6, which means the packet is an operator.
    The bit labeled I (0) is the length type ID, which indicates that the length is a 15-bit number representing the number of bits in the sub-packets.
    The 15 bits labeled L (000000000011011) contain the length of the sub-packets in bits, 27.
    The 11 bits labeled A contain the first sub-packet, a literal value representing the number 10.
    The 16 bits labeled B contain the second sub-packet, a literal value representing the number 20.
    After reading 11 and 16 bits of sub-packet data, the total length indicated in L (27) is reached, and so parsing of this packet stops.

    As another example, here is an operator packet (hexadecimal string EE00D40C823060) with length type ID 1 that contains three sub-packets:

    11101110000000001101010000001100100000100011000001100000
    VVVTTTILLLLLLLLLLLAAAAAAAAAAABBBBBBBBBBBCCCCCCCCCCC
    The three bits labeled V (111) are the packet version, 7.
    The three bits labeled T (011) are the packet type ID, 3, which means the packet is an operator.
    The bit labeled I (1) is the length type ID, which indicates that the length is a 11-bit number representing the number of sub-packets.
    The 11 bits labeled L (00000000011) contain the number of sub-packets, 3.
    The 11 bits labeled A contain the first sub-packet, a literal value representing the number 1.
    The 11 bits labeled B contain the second sub-packet, a literal value representing the number 2.
    The 11 bits labeled C contain the third sub-packet, a literal value representing the number 3.
    After reading 3 complete sub-packets, the number of sub-packets indicated in L (3) is reached, and so parsing of this packet stops.

    For now, parse the hierarchy of the packets throughout the transmission and add up all of the version numbers.

    Here are a few more examples of hexadecimal-encoded transmissions:

    8A004A801A8002F478 represents an operator packet (version 4) which contains an operator packet (version 1) which contains an operator packet (version 5) which contains a literal value (version 6); this packet has a version sum of 16.
    620080001611562C8802118E34 represents an operator packet (version 3) which contains two sub-packets; each sub-packet is an operator packet that contains two literal values. This packet has a version sum of 12.
    C0015000016115A2E0802F182340 has the same structure as the previous example, but the outermost packet uses a different length type ID. This packet has a version sum of 23.
    A0016C880162017C3686B18A3D4780 is an operator packet that contains an operator packet that contains an operator packet that contains five literal values; it has a version sum of 31.
    Decode the structure of your hexadecimal-encoded BITS transmission; what do you get if you add up the version numbers in all packets?
    */
    std::string part_1(const std::vector<std::string>& lines) {
        const auto input = to_bytes(INPUT);
        std::span view {input};
        std::size_t offset = 0;
        const auto packet = parse_packet(view, offset);
        return std::to_string(packet->sum_of_versions());
    }

    /*
    --- Part Two ---
    Now that you have the structure of your transmission decoded, you can calculate the value of the expression it represents.

    Literal values (type ID 4) represent a single number as described above. The remaining type IDs are more interesting:

    Packets with type ID 0 are sum packets - their value is the sum of the values of their sub-packets. If they only have a single sub-packet, their value is the value of the sub-packet.
    Packets with type ID 1 are product packets - their value is the result of multiplying together the values of their sub-packets. If they only have a single sub-packet, their value is the value of the sub-packet.
    Packets with type ID 2 are minimum packets - their value is the minimum of the values of their sub-packets.
    Packets with type ID 3 are maximum packets - their value is the maximum of the values of their sub-packets.
    Packets with type ID 5 are greater than packets - their value is 1 if the value of the first sub-packet is greater than the value of the second sub-packet; otherwise, their value is 0. These packets always have exactly two sub-packets.
    Packets with type ID 6 are less than packets - their value is 1 if the value of the first sub-packet is less than the value of the second sub-packet; otherwise, their value is 0. These packets always have exactly two sub-packets.
    Packets with type ID 7 are equal to packets - their value is 1 if the value of the first sub-packet is equal to the value of the second sub-packet; otherwise, their value is 0. These packets always have exactly two sub-packets.
    Using these rules, you can now work out the value of the outermost packet in your BITS transmission.

    For example:

    C200B40A82 finds the sum of 1 and 2, resulting in the value 3.
    04005AC33890 finds the product of 6 and 9, resulting in the value 54.
    880086C3E88112 finds the minimum of 7, 8, and 9, resulting in the value 7.
    CE00C43D881120 finds the maximum of 7, 8, and 9, resulting in the value 9.
    D8005AC2A8F0 produces 1, because 5 is less than 15.
    F600BC2D8F produces 0, because 5 is not greater than 15.
    9C005AC2F8F0 produces 0, because 5 is not equal to 15.
    9C0141080250320F1802104A08 produces 1, because 1 + 3 = 2 * 2.
    What do you get if you evaluate the expression represented by your hexadecimal-encoded BITS transmission?
    */
    std::string part_2(const std::vector<std::string>& lines) {
        const auto input = to_bytes(INPUT);
        std::span view {input};
        std::size_t offset = 0;
        const auto packet = parse_packet(view, offset);
        return std::to_string(packet->value());
    }

    aoc::registration r {2021, 16, part_1, part_2};

    TEST_SUITE("2021_day16") {
        TEST_CASE("2021_day16:parse_literal_0") {
            const std::vector<uint8_t> data = { 0b10110000u, 0b00000000u };
            std::span view {data};
            std::size_t offset = 0;
            const auto result = parse_packet(view, offset);
            REQUIRE_NE(result.get(), nullptr);
            REQUIRE_EQ(result->type_id, 4);
            REQUIRE_EQ(result->version, 5);
            REQUIRE_EQ(result->value(), 0);
        }
        TEST_CASE("2021_day16:parse_literal_7") {
            const std::vector<uint8_t> data = { 0b01010000u, 0b11100000u };
            std::span view {data};
            std::size_t offset = 0;
            const auto result = parse_packet(view, offset);
            REQUIRE_NE(result.get(), nullptr);
            REQUIRE_EQ(result->type_id, 4);
            REQUIRE_EQ(result->version, 2);
            REQUIRE_EQ(result->value(), 7);
        }
        TEST_CASE("2021_day16:parse_literal_19") {
            const std::vector<uint8_t> data = { 0b11110010u, 0b00100011u };
            std::span view {data};
            std::size_t offset = 0;
            const auto result = parse_packet(view, offset);
            REQUIRE_NE(result.get(), nullptr);
            REQUIRE_EQ(result->type_id, 4);
            REQUIRE_EQ(result->version, 7);
            REQUIRE_EQ(result->value(), 19);
        }
        TEST_CASE("2021_day16:parse_list_type_1") {
            const std::vector<uint8_t> data = { 0b10101010u, 0b00000000u, 0b11101100u, 0b00000010u, 0b10000111u, 0b11110010u, 0b00100011u };
            std::span view {data};
            std::size_t offset = 0;
            const auto result = parse_packet(view, offset);
            const auto* lp = dynamic_cast<const op_packet*>(result.get());
            REQUIRE_NE(lp, nullptr);
            REQUIRE_EQ(result->type_id, 2);
            REQUIRE_EQ(result->version, 5);
            REQUIRE_EQ(lp->packets.size(), 3);
            REQUIRE_EQ(lp->packets[0]->value(), 0);
            REQUIRE_EQ(lp->packets[1]->value(), 7);
            REQUIRE_EQ(lp->packets[2]->value(), 19);
        }
        TEST_CASE("2021_day16:parse_list_type_0") {
            const std::vector<uint8_t> data = { 0b10101000u, 0b00000000u, 0b10011010u, 0b11000000u, 0b00101000u, 0b01111111u, 0b00100010u, 0b00110000u };
            std::span view {data};
            std::size_t offset = 0;
            const auto result = parse_packet(view, offset);
            const auto* lp = dynamic_cast<const op_packet*>(result.get());
            REQUIRE_NE(lp, nullptr);
            REQUIRE_EQ(result->type_id, 2);
            REQUIRE_EQ(result->version, 5);
            REQUIRE_EQ(lp->packets.size(), 3);
            REQUIRE_EQ(lp->packets[0]->value(), 0);
            REQUIRE_EQ(lp->packets[1]->value(), 7);
            REQUIRE_EQ(lp->packets[2]->value(), 19);
        }
//        TEST_CASE("2021_day16:example") {
//
//        }
    }

}