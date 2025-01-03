//
// Created by Dan on 12/19/2021.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <vector>
#include <array>
#include <utility>
#include <set>

#include "utilities.h"
#include "ranges.h"
#include "parse.h"
#include "point_nd.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    constexpr int MIN_BEACONS_TO_MATCH = 12;

    using vector_3d = point<3>;

    class matrix_3d {
    public:
        std::array<int, 9> data{};

        [[nodiscard]] constexpr vector_3d mul_left(const vector_3d& rhs) const noexcept {
            return {(data[0] * rhs[0] + data[1] * rhs[1] + data[2] * rhs[2]),
                    (data[3] * rhs[0] + data[4] * rhs[1] + data[5] * rhs[2]),
                    (data[6] * rhs[0] + data[7] * rhs[1] + data[8] * rhs[2])};
        }

        [[nodiscard]] constexpr vector_3d mul_right(const vector_3d& lhs) const noexcept {
            return {(data[0] * lhs[0] + data[3] * lhs[1] + data[6] * lhs[2]),
                    (data[1] * lhs[0] + data[4] * lhs[1] + data[7] * lhs[2]),
                    (data[2] * lhs[0] + data[5] * lhs[1] + data[8] * lhs[2])};
        }

        [[nodiscard]] constexpr matrix_3d mul_left(const matrix_3d& rhs) const noexcept {
            return {
                    (data[0] * rhs.data[0] + data[1] * rhs.data[3] + data[2] * rhs.data[6]),
                    (data[0] * rhs.data[1] + data[1] * rhs.data[4] + data[2] * rhs.data[7]),
                    (data[0] * rhs.data[2] + data[1] * rhs.data[5] + data[2] * rhs.data[8]),

                    (data[3] * rhs.data[0] + data[4] * rhs.data[3] + data[5] * rhs.data[6]),
                    (data[3] * rhs.data[1] + data[4] * rhs.data[4] + data[5] * rhs.data[7]),
                    (data[3] * rhs.data[2] + data[4] * rhs.data[5] + data[5] * rhs.data[8]),

                    (data[6] * rhs.data[0] + data[7] * rhs.data[3] + data[8] * rhs.data[6]),
                    (data[6] * rhs.data[1] + data[7] * rhs.data[4] + data[8] * rhs.data[7]),
                    (data[6] * rhs.data[2] + data[7] * rhs.data[5] + data[8] * rhs.data[8])
            };
        }

        [[nodiscard]] constexpr matrix_3d mul_right(const matrix_3d& lhs) const noexcept {
            return lhs.mul_left(*this);
        }

        constexpr matrix_3d operator-() const noexcept {
            return {-data[0], -data[1], -data[2], -data[3], -data[4], -data[5], -data[6], -data[7], -data[8]};
        }

        constexpr bool operator==(const matrix_3d& rhs) const noexcept {
            return std::ranges::equal(data, rhs.data);
        }

        constexpr bool operator<(const matrix_3d& rhs) const noexcept {
            const auto res = std::ranges::mismatch(data, rhs.data);
            if (res.in1 == data.end()) {
                return false;
            }
            else {
                return *res.in1 < *res.in2;
            }
        }

        [[nodiscard]] std::string display(int field_width = 3, bool one_line = false) const {
            if (one_line) {
                return fmt::format("[[{}, {}, {}], [{}, {}, {}], [{}, {}, {}]]",
                                   data[0],
                                   data[1],
                                   data[2],
                                   data[3],
                                   data[4],
                                   data[5],
                                   data[6],
                                   data[7],
                                   data[8]);
            }
            else {
                return fmt::format("{1:>{0}}{2:>{0}}{3:>{0}}\n{4:>{0}}{5:>{0}}{6:>{0}}\n{7:>{0}}{8:>{0}}{9:>{0}}",
                                   field_width,
                                   data[0],
                                   data[1],
                                   data[2],
                                   data[3],
                                   data[4],
                                   data[5],
                                   data[6],
                                   data[7],
                                   data[8]);
            }
        }
    };

    constexpr matrix_3d operator*(const matrix_3d& lhs, const matrix_3d& rhs) noexcept {
        return lhs.mul_left(rhs);
    }

    constexpr vector_3d operator*(const vector_3d& lhs, const matrix_3d& rhs) noexcept {
        return rhs.mul_right(lhs);
    }

    constexpr vector_3d operator*(const matrix_3d& lhs, const vector_3d& rhs) noexcept {
        return lhs.mul_left(rhs);
    }

    constexpr vector_3d operator+(const vector_3d& lhs, const vector_3d& rhs) noexcept {
        return {lhs[0] + rhs[0], lhs[1] + rhs[1], lhs[2] + rhs[2]};
    }

    constexpr vector_3d operator-(const vector_3d& lhs, const vector_3d& rhs) noexcept {
        return {lhs[0] - rhs[0], lhs[1] - rhs[1], lhs[2] - rhs[2]};
    }

    constexpr int operator*(const vector_3d& lhs, const vector_3d& rhs) noexcept {
        return lhs[0] * rhs[0] + lhs[1] * rhs[1] + lhs[2] * rhs[2];
    }

    constexpr matrix_3d operator+(const matrix_3d& lhs, const matrix_3d& rhs) noexcept {
        return {lhs.data[0] + rhs.data[0], lhs.data[1] + rhs.data[1], lhs.data[2] + rhs.data[2],
                lhs.data[3] + rhs.data[3], lhs.data[4] + rhs.data[4], lhs.data[5] + rhs.data[5],
                lhs.data[6] + rhs.data[6], lhs.data[7] + rhs.data[7], lhs.data[8] + rhs.data[8]};
    }

    constexpr matrix_3d operator-(const matrix_3d& lhs, const matrix_3d& rhs) noexcept {
        return {lhs.data[0] - rhs.data[0], lhs.data[1] - rhs.data[1], lhs.data[2] - rhs.data[2],
                lhs.data[3] - rhs.data[3], lhs.data[4] - rhs.data[4], lhs.data[5] - rhs.data[5],
                lhs.data[6] - rhs.data[6], lhs.data[7] - rhs.data[7], lhs.data[8] - rhs.data[8]};
    }

    template <typename T>
    constexpr bool operator==(const vector_3d& lhs, const vector_3d& rhs) noexcept {
        return lhs[0] == rhs[0] && lhs[1] == rhs[1] && lhs[2] == rhs[2];
    }

    template <typename T>
    constexpr bool operator<(const vector_3d& lhs, const vector_3d& rhs) noexcept {
        const auto res = std::ranges::mismatch(lhs, rhs);
        if (res.in1 == lhs.end()) {
            return false;
        }
        else {
            return *res.in1 < *res.in2;
        }
    }

    consteval auto calculate_rot_and_refl_combos() {
        constexpr matrix_3d rotations[] = {
                { 1, 0, 0,   0, 1, 0,   0, 0, 1},

                //Around Z:
                {0, 1, 0,  -1, 0, 0,   0, 0, 1},
                {-1, 0, 0,  0,-1, 0,   0, 0, 1},
                {0,-1, 0,   1, 0, 0,   0, 0, 1},

                //Around X:
                {1, 0, 0,   0, 0, 1,   0,-1, 0},
                {1, 0, 0,   0,-1, 0,   0, 0,-1},
                {1, 0, 0,   0, 0,-1,   0, 1, 0},

                //Around Y:
                {0, 0,-1,   0, 1, 0,   1, 0, 0},
                {-1, 0, 0,  0, 1, 0,   0, 0,-1},
                {0, 0, 1,   0, 1, 0,  -1, 0, 0}
        };

        constexpr matrix_3d reflections[] = {
                //Single:
                {-1, 0, 0,   0, 1, 0,   0, 0, 1},
                { 1, 0, 0,   0,-1, 0,   0, 0, 1},
                { 1, 0, 0,   0, 1, 0,   0, 0,-1},

                //Double:
                { 1, 0, 0,   0,-1, 0,   0, 0,-1},
                {-1, 0, 0,   0, 1, 0,   0, 0,-1},
                {-1, 0, 0,   0,-1, 0,   0, 0, 1},

                //Triple:
                {-1, 0, 0,   0,-1, 0,   0, 0, -1}
        };

        std::vector<matrix_3d> scratch;
        for (const auto& rot1 : rotations) {
            for (const auto& rot2 : rotations) {
                for (const auto &refl: reflections) {
                    scratch.push_back(rot1 * rot2 * refl);
                    scratch.push_back(refl * rot1 * rot2);
                }
            }
        }
        std::sort(scratch.begin(), scratch.end());
        scratch.erase(std::unique(scratch.begin(), scratch.end()), scratch.end());
        std::array<matrix_3d, 48> retval;
        if (scratch.size() != retval.size()) {
            //This will cause a compiler error if the size of the array doesn't match the size of the scratch vector.
            //It's a bit of a hack (you need to know how big things are before you have this be a compile-time only
            //function) but it's a useful optimization, and there's no way to return a dynamically sized (at compile
            //time) buffer/array/list/vector as of C++20.
            throw std::exception{};
        }
        std::copy_n(scratch.begin(), retval.size(), retval.begin());
        return retval;
    }

    struct fingerprint {
        uint64_t fp;
        int idx_a;
        int idx_b;

        fingerprint(uint64_t f, int a, int b) : fp{f}, idx_a{a}, idx_b{b} {}

        bool operator<(const fingerprint& rhs) const noexcept {
            return fp < rhs.fp;
        }
    };

    struct scanner {
        int id = -1;
        std::vector<vector_3d> beacons;
        std::vector<fingerprint> fingerprints;
        std::optional<vector_3d> offset;
        std::optional<matrix_3d> transform;
    };

    uint64_t get_fingerprint(const vector_3d& a, const vector_3d& b) noexcept {
        const auto d = a - b;
        return d[0] * d[0] + d[1] * d[1] + d[2] * d[2];
    }

    std::vector<fingerprint> fingerprints_from_beacons(const std::vector<vector_3d>& beacons) {
        const auto bsize = beacons.size();
        std::vector<fingerprint> fps;
        fps.reserve(bsize * (bsize - 1) / 2);
        for (int i = 0; i < bsize; ++i) {
            for (int j = i + 1; j < bsize; ++j) {
                fps.emplace_back(get_fingerprint(beacons[i], beacons[j]), i, j);
            }
        }
        std::sort(fps.begin(), fps.end());
        return fps;
    }

    scanner parse_scanner(int id, std::vector<std::string>::const_iterator current, const std::vector<std::string>::const_iterator& last) {
        std::vector<vector_3d> beacons;
        beacons.reserve(std::distance(current, last));
        for (; current != last; ++current) {
            const auto parts = split(*current, ',');
            const auto nums = parts |
                    std::views::transform([](std::string_view s){ return parse<int>(s); }) |
                    to<std::vector<int>>();
            beacons.push_back({nums[0], nums[1], nums[2]});
        }
        auto fps = fingerprints_from_beacons(beacons);
        return {id, std::move(beacons), std::move(fps)};
    }

    std::vector<scanner> get_input(const std::vector<std::string>& lines) {
        const auto end = lines.end();
        std::vector<scanner> retval;
        retval.reserve(32);
        for (auto current = lines.begin(); current != end; ++current) {
            if (current->starts_with("--- scanner ")) {
                const auto num_str = std::string_view{ *current }.substr(12);
                auto last = current;
                while (last != end && !last->empty()) { ++last; }
                retval.push_back(parse_scanner(parse<int>(num_str), current + 1, last));
            }
        }
        return retval;
    }

    struct offset_info {
        vector_3d offset;
        int matches = 1;

        offset_info(vector_3d off) : offset{off} {}

        bool operator<(const offset_info& rhs) const noexcept {
            return offset < rhs.offset;
        }
    };

    bool operator<(const offset_info& a, const vector_3d& b) noexcept {
        return a.offset < b;
    }

    bool operator<(const vector_3d& a, const offset_info& b) noexcept {
        return a < b.offset;
    }

    std::vector<vector_3d> multiply_all(const matrix_3d& m, const std::vector<vector_3d>& orig) {
        std::vector<vector_3d> retval;
        retval.reserve(orig.size());
        std::transform(orig.begin(), orig.end(), std::back_inserter(retval), [&m](const vector_3d& v) { return m * v; });
        return retval;
    }

    /**
     * Return an offset O and transformation T such that:
     * p_s0 = T*p_s + O
     * We are given a ref with it's own T_r and O_r, and we are looking for T_s and O_s such that:
     * p_r = T_s*p_s + O_s
     * p_s0 = T_r*(T_s*p_s + O_s) + O_r
     * p_s0 = (T_r*T_s)*p_s + (O_r + T_r*O_s)
     * So we want to return T_r*T_s and O_r+T_r*O_s.
     */
    std::optional<std::pair<vector_3d, matrix_3d>> find_relationship(const scanner& ref, const scanner& s) {
        constexpr auto ops = calculate_rot_and_refl_combos();
        std::vector<offset_info> info;
        info.reserve(ref.beacons.size() * s.beacons.size());
        for (const auto& op : ops) {
            info.clear();
            const auto oped = multiply_all(op, s.beacons);
            for (const auto& rp : ref.beacons) {
                for (const auto& sp : oped) {
                    const auto offset = rp - sp;
                    auto found = std::lower_bound(info.begin(), info.end(), offset);
                    if (found != info.end() && found->offset == offset) {
                        if (++found->matches >= MIN_BEACONS_TO_MATCH) {
                            return std::pair{*ref.offset + *ref.transform * found->offset, *ref.transform * op};
                        }
                    }
                    else {
                        info.emplace(found, offset);
                    }
                }
            }
        }
        return std::nullopt;
    }

    matrix_3d rotation_from_vectors(const vector_3d& from, const vector_3d& to) noexcept
    {
        constexpr auto ops = calculate_rot_and_refl_combos();
        for (const auto& op : ops) {
            const auto tr = op * from;
            if (tr == to) {
                return op;
            }
        }
        return {1, 0, 0,    0, 1, 0,    0, 0, 1};
    }

    /**
     * Alternate solution suggested by https://www.reddit.com/r/adventofcode/comments/rjpf7f/2021_day_19_solution
     * to use the differences between all the points for a scanner as a fingerprint.  The distances between two
     * beacons will be the same despite any translations, reflections, or rotations.  We can use this to find two
     * scanners which share points (though we are assuming that the distances are fairly unique!).
     *
     * From that point we can reconstruct the rotation/reflection matrix and then calculate the offset.
     */
    std::optional<std::pair<vector_3d, matrix_3d>> find_relationship_fp(const scanner& ref, const scanner& s) {
        int count = 0;
        std::vector<vector_3d> rm, sm;
        rm.reserve(ref.beacons.size());
        sm.reserve(s.beacons.size());
        for (int ri = 0, si = 0; ri < ref.fingerprints.size() && si < s.fingerprints.size();) {
            if (ref.fingerprints[ri].fp == s.fingerprints[si].fp) {
                rm.push_back(ref.beacons[ref.fingerprints[ri].idx_a]);
                rm.push_back(ref.beacons[ref.fingerprints[ri].idx_b]);
                sm.push_back(s.beacons[s.fingerprints[si].idx_a]);
                sm.push_back(s.beacons[s.fingerprints[si].idx_b]);
                ++count;
                ++ri;
                ++si;
            }
            else if (ref.fingerprints[ri].fp < s.fingerprints[si].fp) {
                ++ri;
            }
            else {
                ++si;
            }
        }
        if (count < (MIN_BEACONS_TO_MATCH * (MIN_BEACONS_TO_MATCH - 1)) / 2) {
            return std::nullopt;
        }
        std::sort(rm.begin(), rm.end());
        rm.erase(std::unique(rm.begin(), rm.end()), rm.end());
        std::sort(sm.begin(), sm.end());
        sm.erase(std::unique(sm.begin(), sm.end()), sm.end());

        constexpr auto ops = calculate_rot_and_refl_combos();
        std::vector<offset_info> info;
        info.reserve(rm.size() * sm.size());
        for (const auto& op : ops) {
            info.clear();
            const auto oped = multiply_all(op, sm);
            for (const auto& rp : rm) {
                for (const auto& sp : oped) {
                    const auto offset = rp - sp;
                    auto found = std::lower_bound(info.begin(), info.end(), offset);
                    if (found != info.end() && found->offset == offset) {
                        if (++found->matches >= MIN_BEACONS_TO_MATCH) {
                            return std::pair{*ref.offset + *ref.transform * found->offset, *ref.transform * op};
                        }
                    }
                    else {
                        info.emplace(found, offset);
                    }
                }
            }
        }
        return std::nullopt;
    }

    /*
    --- Day 19: Beacon Scanner ---
    As your probe drifted down through this area, it released an assortment of beacons and scanners into the water. It's difficult to navigate in the pitch black open waters of the ocean trench, but if you can build a map of the trench using data from the scanners, you should be able to safely reach the bottom.

    The beacons and scanners float motionless in the water; they're designed to maintain the same position for long periods of time. Each scanner is capable of detecting all beacons in a large cube centered on the scanner; beacons that are at most 1000 units away from the scanner in each of the three axes (x, y, and z) have their precise position determined relative to the scanner. However, scanners cannot detect other scanners. The submarine has automatically summarized the relative positions of beacons detected by each scanner (your puzzle input).

    For example, if a scanner is at x,y,z coordinates 500,0,-500 and there are beacons at -500,1000,-1500 and 1501,0,-500, the scanner could report that the first beacon is at -1000,1000,-1000 (relative to the scanner) but would not detect the second beacon at all.

    Unfortunately, while each scanner can report the positions of all detected beacons relative to itself, the scanners do not know their own position. You'll need to determine the positions of the beacons and scanners yourself.

    The scanners and beacons map a single contiguous 3d region. This region can be reconstructed by finding pairs of scanners that have overlapping detection regions such that there are at least 12 beacons that both scanners detect within the overlap. By establishing 12 common beacons, you can precisely determine where the scanners are relative to each other, allowing you to reconstruct the beacon map one scanner at a time.

    For a moment, consider only two dimensions. Suppose you have the following scanner reports:

    --- scanner 0 ---
    0,2
    4,1
    3,3

    --- scanner 1 ---
    -1,-1
    -5,0
    -2,1
    Drawing x increasing rightward, y increasing upward, scanners as S, and beacons as B, scanner 0 detects this:

    ...B.
    B....
    ....B
    S....
    Scanner 1 detects this:

    ...B..
    B....S
    ....B.
    For this example, assume scanners only need 3 overlapping beacons. Then, the beacons visible to both scanners overlap to produce the following complete map:

    ...B..
    B....S
    ....B.
    S.....
    Unfortunately, there's a second problem: the scanners also don't know their rotation or facing direction. Due to magnetic alignment, each scanner is rotated some integer number of 90-degree turns around all of the x, y, and z axes. That is, one scanner might call a direction positive x, while another scanner might call that direction negative y. Or, two scanners might agree on which direction is positive x, but one scanner might be upside-down from the perspective of the other scanner. In total, each scanner could be in any of 24 different orientations: facing positive or negative x, y, or z, and considering any of four directions "up" from that facing.

    For example, here is an arrangement of beacons as seen from a scanner in the same position but in different orientations:

    --- scanner 0 ---
    -1,-1,1
    -2,-2,2
    -3,-3,3
    -2,-3,1
    5,6,-4
    8,0,7

    --- scanner 0 ---
    1,-1,1
    2,-2,2
    3,-3,3
    2,-1,3
    -5,4,-6
    -8,-7,0

    --- scanner 0 ---
    -1,-1,-1
    -2,-2,-2
    -3,-3,-3
    -1,-3,-2
    4,6,5
    -7,0,8

    --- scanner 0 ---
    1,1,-1
    2,2,-2
    3,3,-3
    1,3,-2
    -4,-6,5
    7,0,8

    --- scanner 0 ---
    1,1,1
    2,2,2
    3,3,3
    3,1,2
    -6,-4,-5
    0,7,-8
    By finding pairs of scanners that both see at least 12 of the same beacons, you can assemble the entire map. For example, consider the following report:

    --- scanner 0 ---
    404,-588,-901
    528,-643,409
    -838,591,734
    390,-675,-793
    -537,-823,-458
    -485,-357,347
    -345,-311,381
    -661,-816,-575
    -876,649,763
    -618,-824,-621
    553,345,-567
    474,580,667
    -447,-329,318
    -584,868,-557
    544,-627,-890
    564,392,-477
    455,729,728
    -892,524,684
    -689,845,-530
    423,-701,434
    7,-33,-71
    630,319,-379
    443,580,662
    -789,900,-551
    459,-707,401

    --- scanner 1 ---
    686,422,578
    605,423,415
    515,917,-361
    -336,658,858
    95,138,22
    -476,619,847
    -340,-569,-846
    567,-361,727
    -460,603,-452
    669,-402,600
    729,430,532
    -500,-761,534
    -322,571,750
    -466,-666,-811
    -429,-592,574
    -355,545,-477
    703,-491,-529
    -328,-685,520
    413,935,-424
    -391,539,-444
    586,-435,557
    -364,-763,-893
    807,-499,-711
    755,-354,-619
    553,889,-390

    --- scanner 2 ---
    649,640,665
    682,-795,504
    -784,533,-524
    -644,584,-595
    -588,-843,648
    -30,6,44
    -674,560,763
    500,723,-460
    609,671,-379
    -555,-800,653
    -675,-892,-343
    697,-426,-610
    578,704,681
    493,664,-388
    -671,-858,530
    -667,343,800
    571,-461,-707
    -138,-166,112
    -889,563,-600
    646,-828,498
    640,759,510
    -630,509,768
    -681,-892,-333
    673,-379,-804
    -742,-814,-386
    577,-820,562

    --- scanner 3 ---
    -589,542,597
    605,-692,669
    -500,565,-823
    -660,373,557
    -458,-679,-417
    -488,449,543
    -626,468,-788
    338,-750,-386
    528,-832,-391
    562,-778,733
    -938,-730,414
    543,643,-506
    -524,371,-870
    407,773,750
    -104,29,83
    378,-903,-323
    -778,-728,485
    426,699,580
    -438,-605,-362
    -469,-447,-387
    509,732,623
    647,635,-688
    -868,-804,481
    614,-800,639
    595,780,-596

    --- scanner 4 ---
    727,592,562
    -293,-554,779
    441,611,-461
    -714,465,-776
    -743,427,-804
    -660,-479,-426
    832,-632,460
    927,-485,-438
    408,393,-506
    466,436,-512
    110,16,151
    -258,-428,682
    -393,719,612
    -211,-452,876
    808,-476,-593
    -575,615,604
    -485,667,467
    -680,325,-822
    -627,-443,-432
    872,-547,-609
    833,512,582
    807,604,487
    839,-516,451
    891,-625,532
    -652,-548,-490
    30,-46,-14
    Because all coordinates are relative, in this example, all "absolute" positions will be expressed relative to scanner 0 (using the orientation of scanner 0 and as if scanner 0 is at coordinates 0,0,0).

    Scanners 0 and 1 have overlapping detection cubes; the 12 beacons they both detect (relative to scanner 0) are at the following coordinates:

    -618,-824,-621
    -537,-823,-458
    -447,-329,318
    404,-588,-901
    544,-627,-890
    528,-643,409
    -661,-816,-575
    390,-675,-793
    423,-701,434
    -345,-311,381
    459,-707,401
    -485,-357,347
    These same 12 beacons (in the same order) but from the perspective of scanner 1 are:

    686,422,578
    605,423,415
    515,917,-361
    -336,658,858
    -476,619,847
    -460,603,-452
    729,430,532
    -322,571,750
    -355,545,-477
    413,935,-424
    -391,539,-444
    553,889,-390
    Because of this, scanner 1 must be at 68,-1246,-43 (relative to scanner 0).

    Scanner 4 overlaps with scanner 1; the 12 beacons they both detect (relative to scanner 0) are:

    459,-707,401
    -739,-1745,668
    -485,-357,347
    432,-2009,850
    528,-643,409
    423,-701,434
    -345,-311,381
    408,-1815,803
    534,-1912,768
    -687,-1600,576
    -447,-329,318
    -635,-1737,486
    So, scanner 4 is at -20,-1133,1061 (relative to scanner 0).

    Following this process, scanner 2 must be at 1105,-1205,1229 (relative to scanner 0) and scanner 3 must be at -92,-2380,-20 (relative to scanner 0).

    The full list of beacons (relative to scanner 0) is:

    -892,524,684
    -876,649,763
    -838,591,734
    -789,900,-551
    -739,-1745,668
    -706,-3180,-659
    -697,-3072,-689
    -689,845,-530
    -687,-1600,576
    -661,-816,-575
    -654,-3158,-753
    -635,-1737,486
    -631,-672,1502
    -624,-1620,1868
    -620,-3212,371
    -618,-824,-621
    -612,-1695,1788
    -601,-1648,-643
    -584,868,-557
    -537,-823,-458
    -532,-1715,1894
    -518,-1681,-600
    -499,-1607,-770
    -485,-357,347
    -470,-3283,303
    -456,-621,1527
    -447,-329,318
    -430,-3130,366
    -413,-627,1469
    -345,-311,381
    -36,-1284,1171
    -27,-1108,-65
    7,-33,-71
    12,-2351,-103
    26,-1119,1091
    346,-2985,342
    366,-3059,397
    377,-2827,367
    390,-675,-793
    396,-1931,-563
    404,-588,-901
    408,-1815,803
    423,-701,434
    432,-2009,850
    443,580,662
    455,729,728
    456,-540,1869
    459,-707,401
    465,-695,1988
    474,580,667
    496,-1584,1900
    497,-1838,-617
    527,-524,1933
    528,-643,409
    534,-1912,768
    544,-627,-890
    553,345,-567
    564,392,-477
    568,-2007,-577
    605,-1665,1952
    612,-1593,1893
    630,319,-379
    686,-3108,-505
    776,-3184,-501
    846,-3110,-434
    1135,-1161,1235
    1243,-1093,1063
    1660,-552,429
    1693,-557,386
    1735,-437,1738
    1749,-1800,1813
    1772,-405,1572
    1776,-675,371
    1779,-442,1789
    1780,-1548,337
    1786,-1538,337
    1847,-1591,415
    1889,-1729,1762
    1994,-1805,1792
    In total, there are 79 beacons.

    Assemble the full map of beacons. How many beacons are there?
    */
    std::string part_1(const std::vector<std::string>& lines) {
        auto input = get_input(lines);
        input.front().offset = {0, 0, 0};
        input.front().transform = { 1, 0, 0,   0, 1, 0,   0, 0, 1};
        auto points = input.front().beacons | to<std::set<vector_3d>>();
        while (std::any_of(input.begin(), input.end(), [](const scanner& s){ return !s.offset.has_value(); })) {
            const auto start_t = std::chrono::system_clock::now();
            for (auto& scan : input | std::views::filter([](const scanner& s){ return !s.offset; })) {
                for (const auto& ref : input | std::views::filter([](const scanner& s){ return s.offset.has_value(); })) {
                    const auto diff = find_relationship_fp(ref, scan);
                    if (diff) {
                        scan.offset = diff->first;
                        scan.transform = diff->second;
                        for (const auto& p : scan.beacons) {
                            points.insert(*scan.transform * p + *scan.offset);
                        }
                        break;
                    }
                }
            }
        }
        return std::to_string(points.size());
    }

    /*
    --- Part Two ---
    Sometimes, it's a good idea to appreciate just how big the ocean is. Using the Manhattan distance, how far apart do the scanners get?

    In the above example, scanners 2 (1105,-1205,1229) and 3 (-92,-2380,-20) are the largest Manhattan distance apart. In total, they are 1197 + 1175 + 1249 = 3621 units apart.

    What is the largest Manhattan distance between any two scanners?
    */
    std::string part_2(const std::vector<std::string>& lines) {
        auto input = get_input(lines);
        input.front().offset = {0, 0, 0};
        input.front().transform = { 1, 0, 0,   0, 1, 0,   0, 0, 1};
        while (std::any_of(input.begin(), input.end(), [](const scanner& s){ return !s.offset.has_value(); })) {
            const auto start_t = std::chrono::system_clock::now();
            for (auto& scan : input | std::views::filter([](const scanner& s){ return !s.offset; })) {
                for (const auto& ref : input | std::views::filter([](const scanner& s){ return s.offset.has_value(); })) {
                    const auto diff = find_relationship_fp(ref, scan);
                    if (diff) {
                        scan.offset = diff->first;
                        scan.transform = diff->second;
                        break;
                    }
                }
            }
        }
        int max = std::numeric_limits<int>::min();
        for (int i = 0; i < input.size(); ++i) {
            for (int j = i + 1; j < input.size(); ++j) {
                const auto diff = *input[i].offset - *input[j].offset;
                const auto md = std::abs(diff[0]) + std::abs(diff[1]) + std::abs(diff[2]);
                if (md > max) {
                    max = md;
                }
            }
        }
        return std::to_string(max);
    }

    aoc::registration r {2021, 19, part_1, part_2};

    TEST_SUITE("2021_day19") {
        TEST_CASE("2021_day19:example") {
            std::vector<scanner> input {
                    {0, {
                            {404,-588,-901},
                            {528,-643,409},
                            {-838,591,734},
                            {390,-675,-793},
                            {-537,-823,-458},
                            {-485,-357,347},
                            {-345,-311,381},
                            {-661,-816,-575},
                            {-876,649,763},
                            {-618,-824,-621},
                            {553,345,-567},
                            {474,580,667},
                            {-447,-329,318},
                            {-584,868,-557},
                            {544,-627,-890},
                            {564,392,-477},
                            {455,729,728},
                            {-892,524,684},
                            {-689,845,-530},
                            {423,-701,434},
                            {7,-33,-71},
                            {630,319,-379},
                            {443,580,662},
                            {-789,900,-551},
                            {459,-707,401}
                    }},
                    {1, {
                            {686,422,578},
                            {605,423,415},
                            {515,917,-361},
                            {-336,658,858},
                            {95,138,22},
                            {-476,619,847},
                            {-340,-569,-846},
                            {567,-361,727},
                            {-460,603,-452},
                            {669,-402,600},
                            {729,430,532},
                            {-500,-761,534},
                            {-322,571,750},
                            {-466,-666,-811},
                            {-429,-592,574},
                            {-355,545,-477},
                            {703,-491,-529},
                            {-328,-685,520},
                            {413,935,-424},
                            {-391,539,-444},
                            {586,-435,557},
                            {-364,-763,-893},
                            {807,-499,-711},
                            {755,-354,-619},
                            {553,889,-390}
                    }}
            };
            input.front().fingerprints = fingerprints_from_beacons(input.front().beacons);
            input.back().fingerprints = fingerprints_from_beacons(input.back().beacons);
            input.front().offset = {0, 0, 0};
            input.front().transform = { 1, 0, 0,   0, 1, 0,   0, 0, 1};
            auto points = input.front().beacons | to<std::set<vector_3d>>();
            const auto diff = find_relationship_fp(input.front(), input.back());
            if (diff) {
                input.back().offset = diff->first;
                input.back().transform = diff->second;
                for (const auto& p : input.back().beacons) {
                    points.insert(*input.back().transform * p + *input.back().offset);
                }
            }
            REQUIRE(input.back().offset.has_value());
            REQUIRE_EQ(*input.back().offset, vector_3d{68,-1246,-43});
        }
        TEST_CASE("2021_day19:find_matrix") {
            matrix_3d rot1 = {0,-1, 0,   1, 0, 0,   0, 0, 1},
                rot2 = {1, 0, 0,   0, 0, 1,   0,-1, 0},
                refl = { 1, 0, 0,   0,-1, 0,   0, 0, 1},
                exp = rot1 * rot2 * refl;
            vector_3d x1 = {10, -6, 37},
                    x2 = {-1, 12, 9},
                    x3 = {-5, -6, -7},
                    y1 = exp * x1,
                    y2 = exp * x2,
                    y3 = exp * x3;
            const auto act1 = rotation_from_vectors(x1, y1);
            const auto act2 = rotation_from_vectors(x2, y2);
            const auto act3 = rotation_from_vectors(x3, y3);
            fmt::print("Actual:\n{}\nExpected:\n{}\n", act1.display(6), exp.display(6));
            fmt::print("Actual:\n{}\nExpected:\n{}\n", act2.display(6), exp.display(6));
            fmt::print("Actual:\n{}\nExpected:\n{}\n", act3.display(6), exp.display(6));
            REQUIRE_EQ(act1, act2);
        }
    }

}