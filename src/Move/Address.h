// Copyright © 2017-2023 Trust Wallet.
//
// This file is part of Trust. The full Trust copyright notice, including
// terms governing use, modification, and redistribution, is contained in the
// file LICENSE at the root of the source code distribution tree.

#pragma once

#include "Data.h"
#include "HexCoding.h"
#include "PublicKey.h"
#include <array>

namespace TW::Move {
template <typename Derived, std::size_t N>
class Address {
private:
    static std::string normalize(const std::string& string, std::size_t hexLen) {
        std::string hexStr((size * 2) - hexLen, '0');
        hexStr.append(string);
        return hexStr;
    }

    /// Determines whether a collection of bytes makes a valid address.
    static bool isValid(const Data& data) { return data.size() == size; }
public:
    static constexpr int size = N;
    std::array<int, N> bytes;

    /// Determines whether a string makes a valid address.
    static bool isValid(const std::string& string) {
        auto address = string;
        if (address.starts_with("0x")) {
            address = address.substr(2);
            if (std::size_t hexLen = address.size(); hexLen < Address::size * 2) {
                address = normalize(address, hexLen);
            }
        }
        if (address.size() != 2 * Address::size) {
            return false;
        }
        const auto data = parse_hex(address);
        return isValid(data);
    };

    Address() noexcept = default;

    Address(const std::string& string) {
        if (!isValid(string)) {
            throw std::invalid_argument("Invalid address string");
        }
        auto hexFunctor = [&string]() {
            if (std::size_t hexLen = string.size() - 2; string.starts_with("0x") && hexLen < size * 2) {
                //! We have specific address like 0x1, padding it.
                return parse_hex(normalize(string.substr(2), hexLen));
            } else {
                return parse_hex(string);
            }
        };

        const auto data = hexFunctor();
        std::copy(data.begin(), data.end(), bytes.begin());
    }

    Address(const Data& data) {
        if (!isValid(data)) {
            throw std::invalid_argument("Invalid address data");
        }
        std::copy_n(data.begin(), size, bytes.begin());
    }

    Address(const PublicKey& publicKey) {
        if (publicKey.type != TWPublicKeyTypeED25519) {
            throw std::invalid_argument("Invalid public key type");
        }
        auto digest = static_cast<Derived*>(this)->getDigest(publicKey);
        const auto data = Hash::sha3_256(digest);
        std::copy_n(data.begin(), Address::size, bytes.begin());
    }

    static Derived zero() {
        return Derived("0x0");
    }

    static Derived one() {
        return Derived("0x1");
    }

    static Derived three() {
        return Derived("0x3");
    }

    /// Returns a string representation of the address.
    [[nodiscard]] std::string string(bool withPrefix = true) const {
        std::string output = withPrefix ? "0x" : "";
        return output + hex(bytes);
    };

    /// Returns a short string representation of the address. E.G 0x1;
    [[nodiscard]] std::string shortString() const {
        std::string s = hex(bytes);
        s.erase(0, s.find_first_not_of('0'));
        return s;
    };
};
} // namespace TW::Move
