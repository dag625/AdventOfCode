//
// Created by Daniel Garcia on 12/5/2024.
//

#ifndef ADVENTOFCODE_OPENSSL_HASH_H
#define ADVENTOFCODE_OPENSSL_HASH_H

#include <openssl/evp.h>
#include <vector>
#include <cstdint>
#include <string>

namespace aoc {

    class openssl_hash {
        EVP_MD_CTX* m_ctxt = EVP_MD_CTX_new();

        void add(const void* data, std::size_t len);

    public:
        explicit openssl_hash(const EVP_MD* digest);
        openssl_hash(const openssl_hash&) = delete;
        openssl_hash(openssl_hash&&) = delete;
        ~openssl_hash();

        void reinit();

        template <typename T>
        void add(const std::vector<T>& data) {
            add(reinterpret_cast<const void*>(data.data()), data.size());
        }

        void add(const std::string& data) {
            add(reinterpret_cast<const void*>(data.data()), data.size());
        }

        std::vector<uint8_t> finalize();
        std::string finalizeHex();
    };

} // aoc

#endif //ADVENTOFCODE_OPENSSL_HASH_H
