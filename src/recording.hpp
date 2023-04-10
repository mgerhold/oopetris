#pragma once

#include "input_event.hpp"
#include "random.hpp"
#include "types.hpp"
#include "utils.hpp"
#include <filesystem>
#include <fstream>
#include <spdlog/spdlog.h>
#include <tl/expected.hpp>
#include <tl/optional.hpp>
#include <vector>

struct RecordingError : public std::exception { };

struct Recording {
public:
    struct TetrionHeader final {
        Random::Seed seed;
        i32 starting_level;
    };

protected:
    struct Record final {
        u8 tetrion_index;
        u64 simulation_step_index;
        InputEvent event;
    };

    std::vector<TetrionHeader> m_tetrion_headers;

protected:
    Recording() = default;
    explicit Recording(std::vector<TetrionHeader> tetrion_headers) : m_tetrion_headers{ std::move(tetrion_headers) } { }

public:
    virtual ~Recording() = default;

    [[nodiscard]] const std::vector<TetrionHeader>& tetrion_headers() const {
        return m_tetrion_headers;
    }
};

struct RecordingReader : public Recording {
    std::vector<Record> m_records;

public:
    explicit RecordingReader(const std::filesystem::path& path) {
        std::ifstream file{ path, std::ios::in | std::ios::binary };
        if (not file) {
            spdlog::error("unable to load recording from file \"{}\"", path.string());
            throw RecordingError{};
        }
        const auto num_tetrions = read_integral_from_file<u8>(file);
        if (not num_tetrions.has_value()) {
            spdlog::error("unable to read number of tetrions from recorded game");
            throw RecordingError{};
        }

        m_tetrion_headers.reserve(*num_tetrions);
        for (u8 i = 0; i < *num_tetrions; ++i) {
            auto header = read_tetrion_header_from_file(file);
            if (not header) {
                spdlog::error("failed to read tetrion header from recorded game");
                throw RecordingError{};
            }
            m_tetrion_headers.push_back(*header);
        }

        while (true) {
            const auto record = read_record_from_file(file);
            if (not record.has_value()) {
                if (record.error() == ReadError::EndOfFile) {
                    // finished reading
                    break;
                }
                spdlog::error("invalid record while reading recorded game");
                throw RecordingError{};
            }
            m_records.push_back(*record);
        }
    }

    [[nodiscard]] const Record& at(const usize index) const {
        return m_records.at(index);
    }

    [[nodiscard]] usize num_records() const {
        return m_records.size();
    }

    [[nodiscard]] auto begin() const {
        return m_records.cbegin();
    }

    [[nodiscard]] auto end() const {
        return m_records.cend();
    }

private:
    enum class ReadError {
        EndOfFile,
        Incomplete,
        InvalidStream,
    };

    template<typename Result>
    using ReadResult = tl::expected<Result, ReadError>;

    template<std::integral Integral>
    [[nodiscard]] static ReadResult<Integral> read_integral_from_file(std::ifstream& file) {
        if (not file) {
            spdlog::error("failed to read data from file");
            return tl::unexpected{ ReadError::InvalidStream };
        }

        Integral little_endian_data;
        file.read(reinterpret_cast<char*>(&little_endian_data), sizeof(little_endian_data));
        if (not file) {
            return tl::unexpected{ ReadError::Incomplete };
        }
        return utils::from_little_endian(little_endian_data);
    }

    [[nodiscard]] static ReadResult<TetrionHeader> read_tetrion_header_from_file(std::ifstream& file) {
        if (not file) {
            spdlog::error("failed to read data from file");
            return tl::unexpected{ ReadError::InvalidStream };
        }

        const auto seed = read_integral_from_file<decltype(TetrionHeader::seed)>(file);

        if (not seed.has_value()) {
            return tl::unexpected{ ReadError::Incomplete };
        }

        const auto starting_level = read_integral_from_file<decltype(TetrionHeader::starting_level)>(file);
        if (not starting_level.has_value()) {
            return tl::unexpected{ ReadError::Incomplete };
        }

        return TetrionHeader{ .seed{ *seed }, .starting_level{ *starting_level } };
    }

    [[nodiscard]] static ReadResult<Record> read_record_from_file(std::ifstream& file) {
        if (not file) {
            spdlog::error("invalid input file stream while trying to read record");
            return tl::unexpected{ ReadError::InvalidStream };
        }

        const auto tetrion_index = read_integral_from_file<decltype(Record::tetrion_index)>(file);
        if (not tetrion_index.has_value()) {
            return tl::unexpected{ ReadError::EndOfFile };
        }

        const auto simulation_step_index = read_integral_from_file<decltype(Record::simulation_step_index)>(file);
        if (not simulation_step_index.has_value()) {
            return tl::unexpected{ ReadError::Incomplete };
        }

        const auto event = read_integral_from_file<u8>(file);
        if (not file) {
            return tl::unexpected{ ReadError::Incomplete };
        }

        return Record{
            .tetrion_index{ *tetrion_index },
            .simulation_step_index{ *simulation_step_index },
            .event{ static_cast<InputEvent>(*event) }, // todo: validation
        };
    }
};

struct RecordingWriter : public Recording {
private:
    std::ofstream m_output_file;

public:
    explicit RecordingWriter(const std::filesystem::path& path, std::vector<TetrionHeader> tetrion_headers)
        : Recording{ std::move(tetrion_headers) },
          m_output_file{ path, std::ios::out | std::ios::binary } {
        if (not m_output_file) {
            spdlog::error("failed to open output file \"{}\"", path.string());
            throw RecordingError{};
        }
        write_integral_to_file(m_output_file, static_cast<u8>(m_tetrion_headers.size()));

        for (const auto& header : m_tetrion_headers) {
            write_tetrion_header_to_file(m_output_file, header);
        }
    }

    void add_event(const u8 tetrion_index, const u64 simulation_step_index, const InputEvent event) {
        assert(tetrion_index < m_tetrion_headers.size());
        write(tetrion_index);
        write(simulation_step_index);
        write(static_cast<u8>(event));
    }

private:
    static void write_integral_to_file(std::ofstream& file, const std::integral auto data) {
        if (not file) {
            spdlog::error("failed to write data \"{}\"", data);
            return;
        }

        const auto little_endian_data = utils::to_little_endian(data);
        file.write(reinterpret_cast<const char*>(&little_endian_data), sizeof(little_endian_data));
    }

    static void write_tetrion_header_to_file(std::ofstream& file, const TetrionHeader& header) {
        static_assert(sizeof(Random::Seed) == 8);
        write_integral_to_file(file, header.seed);
        write_integral_to_file(file, header.starting_level);
    }

    void write(const std::integral auto data) {
        write_integral_to_file(m_output_file, data);
    }
};
