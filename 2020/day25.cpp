//
// Created by Daniel Garcia on 12/25/20.
//

#include "registration.h"

#include <doctest/doctest.h>

#include <filesystem>
#include <array>

namespace {

    constexpr auto PUBLIC_KEY_1 = 10212254ull;
    constexpr auto PUBLIC_KEY_2 = 12577395ull;

    constexpr auto MODULUS = 20201227ull;

    uint64_t encrypt_transform(uint64_t val, const uint64_t subj_num) {
        val *= subj_num;
        val %= MODULUS;
        return val;
    }

    uint64_t encrypt(uint64_t val, const uint64_t loop_size) {
        const auto subj_num = val;
        val = 1;
        for (uint64_t i = 0; i < loop_size; ++i) {
            val = encrypt_transform(val, subj_num);
        }
        return val;
    }

    std::array<uint64_t, 2> find_loop_sizes(const uint64_t pub_key_1, const uint64_t pub_key_2) {
        uint64_t val = 1ull;
        std::array<uint64_t, 2> loop_sizes{};
        for (uint64_t ls = 1ull; loop_sizes[0] == 0 || loop_sizes[1] == 0; ++ls) {
            val = encrypt_transform(val, 7);
            if (val == pub_key_1) {
                loop_sizes[0] = ls;
            }
            if (val == pub_key_2) {
                loop_sizes[1] = ls;
            }
        }
        return loop_sizes;
    }

    /*
    You finally reach the check-in desk. Unfortunately, their registration systems are currently offline, and they cannot check you in. Noticing the look on your face, they quickly add that tech support is already on the way! They even created all the room keys this morning; you can take yours now and give them your room deposit once the registration system comes back online.

    The room key is a small RFID card. Your room is on the 25th floor and the elevators are also temporarily out of service, so it takes what little energy you have left to even climb the stairs and navigate the halls. You finally reach the door to your room, swipe your card, and - beep - the light turns red.

    Examining the card more closely, you discover a phone number for tech support.

    "Hello! How can we help you today?" You explain the situation.

    "Well, it sounds like the card isn't sending the right command to unlock the door. If you go back to the check-in desk, surely someone there can reset it for you." Still catching your breath, you describe the status of the elevator and the exact number of stairs you just had to climb.

    "I see! Well, your only other option would be to reverse-engineer the cryptographic handshake the card does with the door and then inject your own commands into the data stream, but that's definitely impossible." You thank them for their time.

    Unfortunately for the door, you know a thing or two about cryptographic handshakes.

    The handshake used by the card and the door involves an operation that transforms a subject number. To transform a subject number, start with the value 1. Then, a number of times called the loop size, perform the following steps:

    Set the value to itself multiplied by the subject number.
    Set the value to the remainder after dividing the value by 20201227.
    The card always uses a specific, secret loop size when it transforms a subject number. The door always uses a different, secret loop size.

    The cryptographic handshake works like this:

    The card transforms the subject number of 7 according to the card's secret loop size. The result is called the card's public key.
    The door transforms the subject number of 7 according to the door's secret loop size. The result is called the door's public key.
    The card and door use the wireless RFID signal to transmit the two public keys (your puzzle input) to the other device. Now, the card has the door's public key, and the door has the card's public key. Because you can eavesdrop on the signal, you have both public keys, but neither device's loop size.
    The card transforms the subject number of the door's public key according to the card's loop size. The result is the encryption key.
    The door transforms the subject number of the card's public key according to the door's loop size. The result is the same encryption key as the card calculated.
    If you can use the two public keys to determine each device's loop size, you will have enough information to calculate the secret encryption key that the card and door use to communicate; this would let you send the unlock command directly to the door!

    For example, suppose you know that the card's public key is 5764801. With a little trial and error, you can work out that the card's loop size must be 8, because transforming the initial subject number of 7 with a loop size of 8 produces 5764801.

    Then, suppose you know that the door's public key is 17807724. By the same process, you can determine that the door's loop size is 11, because transforming the initial subject number of 7 with a loop size of 11 produces 17807724.

    At this point, you can use either device's loop size with the other device's public key to calculate the encryption key. Transforming the subject number of 17807724 (the door's public key) with a loop size of 8 (the card's loop size) produces the encryption key, 14897079. (Transforming the subject number of 5764801 (the card's public key) with a loop size of 11 (the door's loop size) produces the same encryption key: 14897079.)

    What encryption key is the handshake trying to establish?
    */
    std::string part_1(const std::filesystem::path&) {
        const auto loop_sizes = find_loop_sizes(PUBLIC_KEY_1, PUBLIC_KEY_2);
        return std::to_string(encrypt(PUBLIC_KEY_2, loop_sizes[0]));
    }

    //Part 2 was completing all the other challenges... :(

    aoc::registration r {2020, 25, part_1};

    TEST_SUITE("day25" * doctest::description("Tests for the challenges for day 25.")) {
        using namespace std::string_view_literals;
        TEST_CASE("day25:example" * doctest::description("Testing my solution on the example given in the challenge.")) {
            constexpr auto CARD_PUBLIC_KEY = 5764801ull;
            constexpr auto DOOR_PUBLIC_KEY = 17807724ull;

            const auto loop_sizes = find_loop_sizes(CARD_PUBLIC_KEY, DOOR_PUBLIC_KEY);
            REQUIRE_EQ(loop_sizes[0], 8);
            REQUIRE_EQ(loop_sizes[1], 11);

            CHECK_EQ(encrypt(DOOR_PUBLIC_KEY, loop_sizes[0]), 14897079ull);
            CHECK_EQ(encrypt(CARD_PUBLIC_KEY, loop_sizes[1]), 14897079ull);
        }
    }

} /* namespace */