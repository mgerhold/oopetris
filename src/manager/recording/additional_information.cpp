

#include "additional_information.hpp"
#include "helper.hpp"


helper::expected<std::pair<std::string, recorder::InformationValue>, std::string>
recorder::InformationValue::read_from_istream(std::istream& istream) {

    const auto key = read_string_from_istream(istream);
    if (not key.has_value()) {
        return helper::unexpected<std::string>{ key.error() };
    }

    const auto value = read_value_from_istream(istream);
    if (not value.has_value()) {
        return helper::unexpected<std::string>{ value.error() };
    }

    return std::pair<std::string, recorder::InformationValue>{ key.value(), value.value() };
}

[[nodiscard]] std::vector<char> recorder::InformationValue::to_bytes() const {
    auto bytes = std::vector<char>{};

    if (is<std::string>()) {
        const auto value = string_to_bytes(as<std::string>());
        helper::writer::append_bytes(bytes, value);
        return bytes;
    } else if (is<float>()) {
        //TODO: validate that the float and double conversion works!

        static_assert(sizeof(float) == 4 && sizeof(u32) == 4);
        const auto value = as<float>();
        helper::writer::append_value<u32>(bytes, static_cast<u32>(value));
        return bytes;

    } else if (is<double>()) {
        static_assert(sizeof(double) == 8 && sizeof(u64) == 8);
        const auto value = as<double>();
        helper::writer::append_value<u64>(bytes, static_cast<u64>(value));
        return bytes;
    } else if (is<bool>()) {
        static_assert(sizeof(bool) == 1 && sizeof(u8) == 1);
        const auto value = as<bool>();
        helper::writer::append_value<u8>(bytes, static_cast<u8>(value));
        return bytes;
    } else if (is<u8>()) {
        static_assert(sizeof(u8) == 1);
        const auto value = as<u8>();
        helper::writer::append_value<u8>(bytes, value);
        return bytes;
    } else if (is<i8>()) {
        static_assert(sizeof(i8) == 1);
        const auto value = as<i8>();
        helper::writer::append_value<i8>(bytes, value);
        return bytes;
    } else if (is<u32>()) {
        static_assert(sizeof(u32) == 4);
        const auto value = as<u32>();
        helper::writer::append_value<u32>(bytes, value);
        return bytes;
    } else if (is<i32>()) {
        static_assert(sizeof(i32) == 4);
        const auto value = as<i32>();
        helper::writer::append_value<i32>(bytes, value);
        return bytes;
    } else if (is<u64>()) {
        static_assert(sizeof(u64) == 8);
        const auto value = as<u64>();
        helper::writer::append_value<u64>(bytes, value);
        return bytes;
    } else if (is<i64>()) {
        static_assert(sizeof(i64) == 8);
        const auto value = as<i64>();
        helper::writer::append_value<i64>(bytes, value);
        return bytes;
    } else if (is<std::vector<InformationValue>>()) {
        static_assert(sizeof(u32) == 4);
        const auto& vector_value = as<std::vector<InformationValue>>();
        helper::writer::append_value<u32>(bytes, static_cast<u32>(vector_value.size()));
        for (const auto& value : vector_value) {
            const auto value_bytes = value.to_bytes();
            helper::writer::append_bytes(bytes, value_bytes);
        }
        return bytes;
    } else {
        utils::unreachable();
    }
}

[[nodiscard]] std::vector<char> recorder::InformationValue::string_to_bytes(const std::string& value) {
    auto bytes = std::vector<char>{};

    static_assert(sizeof(u32) == 4);
    helper::writer::append_value<u32>(bytes, static_cast<u32>(value.size()));

    const char* chars = value.c_str();
    for (decltype(value.size()) i = 0; i < value.size(); ++i) {
        helper::writer::append_value<u8>(bytes, chars[i]);
    }

    return bytes;
}

helper::expected<std::string, std::string> recorder::InformationValue::read_string_from_istream(std::istream& istream) {

    static_assert(sizeof(u32) == 4);
    const auto string_size = helper::reader::read_from_istream<u32>(istream);
    if (not string_size.has_value()) {
        return helper::unexpected<std::string>{ "unable to read string size" };
    }

    char* raw_chars = new char[string_size.value()];
    for (u32 i = 0; i < string_size.value(); ++i) {

        const auto local_value = helper::reader::read_from_istream<u8>(istream);
        if (not local_value.has_value()) {
            return helper::unexpected<std::string>{ fmt::format("unable to read char in string at index {}", i) };
        }

        raw_chars[i] = local_value.value();
    }

    std::string result{ raw_chars, string_size.value() };

    return result;
}


helper::expected<recorder::InformationValue, std::string> recorder::InformationValue::read_value_from_istream(
        std::istream& istream
) {
    const auto magic_byte = helper::reader::read_from_istream<std::underlying_type_t<Type>>(istream);
    if (not magic_byte.has_value()) {
        return helper::unexpected<std::string>{ "unable to read magic byte" };
    }

    const auto magic_byte_value = magic_byte.value();

    if (magic_byte_value == utils::to_underlying(Type::String)) {
        const auto value = read_string_from_istream(istream);
        if (not value.has_value()) {
            return helper::unexpected<std::string>{ value.error() };
        }
        return recorder::InformationValue{ value.value() };
    } else if (magic_byte_value == utils::to_underlying(Type::Float)) {
        //TODO: validate that the float and double conversion works!

        static_assert(sizeof(float) == 4 && sizeof(u32) == 4);
        const auto raw_float = helper::reader::read_from_istream<u32>(istream);
        if (not raw_float.has_value()) {
            return helper::unexpected<std::string>{ "unable to read float value" };
        }

        const u32 raw_float_value = raw_float.value();
        const float value = static_cast<float>(raw_float_value);
        return recorder::InformationValue{ value };
    } else if (magic_byte_value == utils::to_underlying(Type::Double)) {

        static_assert(sizeof(double) == 8 && sizeof(u64) == 8);
        const auto raw_double = helper::reader::read_from_istream<u64>(istream);
        if (not raw_double.has_value()) {
            return helper::unexpected<std::string>{ "unable to read double value" };
        }

        const u64 raw_double_value = raw_double.value();
        const double value = static_cast<double>(raw_double_value);
        return recorder::InformationValue{ value };
    } else if (magic_byte_value == utils::to_underlying(Type::Bool)) {

        static_assert(sizeof(bool) == 1 && sizeof(u8) == 1);
        const auto raw_value = helper::reader::read_from_istream<u8>(istream);
        if (not raw_value.has_value()) {
            return helper::unexpected<std::string>{ "unable to read bool value" };
        }
        return recorder::InformationValue{ raw_value.value() != 0 };
    } else if (magic_byte_value == utils::to_underlying(Type::U8)) {

        static_assert(sizeof(u8) == 1);
        const auto raw_value = helper::reader::read_from_istream<u8>(istream);
        if (not raw_value.has_value()) {
            return helper::unexpected<std::string>{ "unable to read u8 value" };
        }
        return recorder::InformationValue{ raw_value.value() };
    } else if (magic_byte_value == utils::to_underlying(Type::I8)) {

        static_assert(sizeof(i8) == 1);
        const auto raw_value = helper::reader::read_from_istream<i8>(istream);
        if (not raw_value.has_value()) {
            return helper::unexpected<std::string>{ "unable to read i8 value" };
        }
        return recorder::InformationValue{ raw_value.value() };
    } else if (magic_byte_value == utils::to_underlying(Type::U32)) {

        static_assert(sizeof(u32) == 4);
        const auto raw_value = helper::reader::read_from_istream<u32>(istream);
        if (not raw_value.has_value()) {
            return helper::unexpected<std::string>{ "unable to read u32 value" };
        }
        return recorder::InformationValue{ raw_value.value() };
    } else if (magic_byte_value == utils::to_underlying(Type::I32)) {

        static_assert(sizeof(i32) == 4);
        const auto raw_value = helper::reader::read_from_istream<i32>(istream);
        if (not raw_value.has_value()) {
            return helper::unexpected<std::string>{ "unable to read i32 value" };
        }
        return recorder::InformationValue{ raw_value.value() };
    } else if (magic_byte_value == utils::to_underlying(Type::U64)) {

        static_assert(sizeof(u64) == 8);
        const auto raw_value = helper::reader::read_from_istream<u64>(istream);
        if (not raw_value.has_value()) {
            return helper::unexpected<std::string>{ "unable to read u64 value" };
        }
        return recorder::InformationValue{ raw_value.value() };
    } else if (magic_byte_value == utils::to_underlying(Type::I64)) {

        static_assert(sizeof(i64) == 8);
        const auto raw_value = helper::reader::read_from_istream<i64>(istream);
        if (not raw_value.has_value()) {
            return helper::unexpected<std::string>{ "unable to read i64 value" };
        }
        return recorder::InformationValue{ raw_value.value() };
    } else if (magic_byte_value == utils::to_underlying(Type::Vector)) {

        static_assert(sizeof(u32) == 4);
        const auto vector_size = helper::reader::read_from_istream<u32>(istream);
        if (not vector_size.has_value()) {
            return helper::unexpected<std::string>{ "unable to read vector size" };
        }

        std::vector<InformationValue> result{};
        result.reserve(vector_size.value());
        for (u32 i = 0; i < vector_size.value(); ++i) {

            const auto local_value = read_value_from_istream(istream);
            if (not local_value.has_value()) {
                return helper::unexpected<std::string>{
                    fmt::format("unable to read value in vector at index {}: {}", i, local_value.error())
                };
            }

            result.push_back(local_value.value());
        }

        return recorder::InformationValue{ result };

    } else {
        return helper::unexpected<std::string>{
            fmt::format("invalid magic byte: {}", static_cast<int>(magic_byte.value()))
        };
    }
}


recorder::AdditionalInformation::AdditionalInformation(std::istream& istream) {

    const auto magic_bytes =
            helper::reader::read_from_istream<decltype(AdditionalInformation::magic_start_byte)>(istream);
    if (not magic_bytes.has_value()) {
        throw std::runtime_error{ "unable to read magic file bytes from recorded game" };
    }
    if (magic_bytes.value() != AdditionalInformation::magic_start_byte) {
        throw std::runtime_error{ "magic start bytes are not correct, the data is probably corrupted" };
    }

    const auto num_pairs = helper::reader::read_from_istream<u32>(istream);
    static_assert(sizeof(decltype(num_pairs.value())) == 4);
    if (not num_pairs.has_value()) {
        throw std::runtime_error{ "unable to read number of pairs" };
    }

    values.reserve(num_pairs.value());
    for (u32 i = 0; i < num_pairs.value(); ++i) {
        auto value_result = InformationValue::read_from_istream(istream);
        if (not value_result.has_value()) {
            throw std::runtime_error{
                fmt::format("failed to read value from AdditionalInformation: {}", value_result.error())
            };
        }

        const auto& [key, value] = value_result.value();

        if (values.contains(key)) {
            throw std::runtime_error{ fmt::format("AdditionalInformation already contains key '{}'", key) };
        }

        values.insert_or_assign(key, value);
    }


    const auto calculated_checksum = get_checksum();

    const auto read_checksum = helper::reader::read_array_from_istream<
            decltype(calculated_checksum)::value_type, Sha256Stream::ChecksumSize>(istream);
    if (not read_checksum.has_value()) {
        throw std::runtime_error{ "unable to read value checksum from AdditionalInformation" };
    }
    if (read_checksum.value() != calculated_checksum) {
        throw std::runtime_error{ fmt::format(
                "value checksum mismatch, the AdditionalInformation was altered: expected {:x} but got {:x}",
                fmt::join(calculated_checksum, ""), fmt::join(read_checksum.value(), "")
        ) };
    }
}


helper::optional<recorder::AdditionalInformation> recorder::AdditionalInformation::from_istream(std::istream& istream) {
    try {
        const auto result = recorder::AdditionalInformation{ istream };
        return result;
    } catch (const std::exception& error) {
        spdlog::error(error.what());
        return helper::nullopt;
    }
}

void recorder::AdditionalInformation::add_value(const std::string& key, const InformationValue& value, bool overwrite) {
    if (values.contains(key) and not overwrite) {
        throw std::runtime_error("Can't overwrite already existing key");
    }

    values.insert_or_assign(key, value);
}

helper::optional<recorder::InformationValue> recorder::AdditionalInformation::get(const std::string& key) const {

    if (not values.contains(key)) {
        return helper::nullopt;
    }

    return values.at(key);
}

[[nodiscard]] std::vector<char> recorder::AdditionalInformation::to_bytes() const {
    auto bytes = std::vector<char>{};

    static_assert(sizeof(decltype(AdditionalInformation::magic_start_byte)) == 4);
    helper::writer::append_value(bytes, AdditionalInformation::magic_start_byte);

    static_assert(sizeof(u32) == 4);
    helper::writer::append_value(bytes, static_cast<u32>(values.size()));

    for (const auto& [key, value] : values) {

        const auto key_bytes = InformationValue::string_to_bytes(key);
        helper::writer::append_bytes(bytes, key_bytes);

        const auto value_bytes = value.to_bytes();
        helper::writer::append_bytes(bytes, value_bytes);
    }

    const auto checksum = get_checksum();
    static_assert(sizeof(decltype(checksum)) == 32);

    for (const auto& checksum_byte : checksum) {
        helper::writer::append_value<u8>(bytes, checksum_byte);
    }

    return bytes;
}

[[nodiscard]] Sha256Stream::Checksum recorder::AdditionalInformation::get_checksum() const {
    Sha256Stream sha256_creator{};

    static_assert(sizeof(u32) == 4);
    sha256_creator << static_cast<u32>(values.size());

    for (const auto& [key, value] : values) {
        sha256_creator << key;
        sha256_creator << value.to_string();
    }

    return sha256_creator.get_hash();
}
