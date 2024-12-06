//
// Created by Daniel Garcia on 12/5/2024.
//

#include "openssl_hash.h"
#include <fmt/core.h>
#include <fmt/ranges.h>
#include <stdexcept>

namespace aoc {


    openssl_hash::openssl_hash(const EVP_MD* digest) {
        const int res = EVP_DigestInit_ex(m_ctxt, digest, nullptr);
        if (res == 0) {
            throw std::runtime_error{"Failed to initialize digest."};
        }
    }

    openssl_hash::~openssl_hash() {
        EVP_MD_CTX_free(m_ctxt);
    }

    void openssl_hash::reinit() {
        const int res = EVP_DigestInit_ex(m_ctxt, nullptr, nullptr);
        if (res == 0) {
            throw std::runtime_error{"Failed to reinitialize digest."};
        }
    }

    void openssl_hash::add(const void* data, std::size_t len) {
        const int res = EVP_DigestUpdate(m_ctxt, data, len);
        if (res == 0) {
            throw std::runtime_error{"Failed to update digest."};
        }
    }

    std::vector<uint8_t> openssl_hash::finalize() {
        unsigned int len = EVP_MAX_MD_SIZE;
        unsigned char buf[EVP_MAX_MD_SIZE];
        const int res = EVP_DigestFinal_ex(m_ctxt, buf, &len);
        if (res == 0) {
            throw std::runtime_error{"Failed to finalize digest."};
        }
        else {
            std::vector<uint8_t> retval;
            retval.insert(retval.begin(), buf, buf + len);
            return retval;
        }
    }

    std::string openssl_hash::finalizeHex() {
        const auto bytes = finalize();
        return fmt::format("{:02x}", fmt::join(bytes, ""));
    }

} // aoc