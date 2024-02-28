#include "game/tetrion_snapshot.hpp"

#include <fmt/format.h>
#include <magic_enum.hpp>
#include <spdlog/spdlog.h>
#include <sstream>
#include <string_view>


TetrionSnapshot::TetrionSnapshot(
        u8 tetrion_index, // NOLINT(bugprone-easily-swappable-parameters)
        Level level,
        Score score,
        LineCount lines_cleared,
        SimulationStep simulation_step_index,
        const MinoStack& mino_stack
)
    : m_tetrion_index{ tetrion_index },
      m_level{ level },
      m_score{ score },
      m_lines_cleared{ lines_cleared },
      m_simulation_step_index{ simulation_step_index },
      m_mino_stack{ mino_stack } { }

TetrionSnapshot::TetrionSnapshot(std::istream& istream) {
    const auto tetrion_index = read_from_istream<u8>(istream);
    if (not tetrion_index.has_value()) {
        throw std::runtime_error{ "unable to read tetrion index from snapshot" };
    }
    m_tetrion_index = tetrion_index.value();

    const auto level = read_from_istream<Level>(istream);
    if (not level.has_value()) {
        throw std::runtime_error{ "unable to read level from snapshot" };
    }
    m_level = level.value();

    const auto score = read_from_istream<Score>(istream);
    if (not score.has_value()) {
        throw std::runtime_error{ "unable to read score from snapshot" };
    }
    m_score = score.value();

    const auto lines_cleared = read_from_istream<LineCount>(istream);
    if (not lines_cleared.has_value()) {
        throw std::runtime_error{ "unable to read lines cleared from snapshot" };
    }
    m_lines_cleared = lines_cleared.value();

    const auto simulation_step_index = read_from_istream<SimulationStep>(istream);
    if (not simulation_step_index.has_value()) {
        throw std::runtime_error{ "unable to read simulation step index from snapshot" };
    }
    m_simulation_step_index = simulation_step_index.value();

    const auto num_minos = read_from_istream<MinoCount>(istream);
    if (not num_minos.has_value()) {
        throw std::runtime_error{ "unable to read number of minos from snapshot" };
    }

    for (MinoCount i = 0; i < num_minos.value(); ++i) {
        const auto x = read_from_istream<Coordinate>(istream);
        if (not x.has_value()) {
            throw std::runtime_error{ "unable to read x coordinate of mino from snapshot" };
        }

        const auto y = read_from_istream<Coordinate>(istream);
        if (not y.has_value()) {
            throw std::runtime_error{ "unable to read y coordinate of mino from snapshot" };
        }

        const auto type = read_from_istream<std::underlying_type_t<TetrominoType>>(istream);
        if (not type.has_value()) {
            throw std::runtime_error{ "unable to read tetromino type of mino from snapshot" };
        }

        const auto maybe_type = magic_enum::enum_cast<TetrominoType>(type.value());
        if (not maybe_type.has_value()) {
            throw std::runtime_error{ fmt::format("got invalid enum value for TetrominoType: {}", type.value()) };
        }

        m_mino_stack.set(shapes::AbstractPoint<u8>(x.value(), y.value()), maybe_type.value());
    }
}

TetrionSnapshot::TetrionSnapshot(const Tetrion& tetrion, const SimulationStep simulation_step_index)
    : TetrionSnapshot{ tetrion.tetrion_index(), tetrion.level(),       tetrion.score(),
                       tetrion.lines_cleared(), simulation_step_index, tetrion.mino_stack() } { }

[[nodiscard]] u8 TetrionSnapshot::tetrion_index() const {
    return m_tetrion_index;
}

[[nodiscard]] u64 TetrionSnapshot::simulation_step_index() const {
    return m_simulation_step_index;
}

[[nodiscard]] std::vector<char> TetrionSnapshot::to_bytes() const {
    auto bytes = std::vector<char>{};

    static_assert(sizeof(decltype(m_tetrion_index)) == 1);
    append(bytes, m_tetrion_index);

    static_assert(sizeof(decltype(m_level)) == 4);
    append(bytes, m_level);

    static_assert(sizeof(decltype(m_score)) == 8);
    append(bytes, m_score);

    static_assert(sizeof(decltype(m_lines_cleared)) == 4);
    append(bytes, m_lines_cleared);

    static_assert(sizeof(decltype(m_simulation_step_index)) == 8);
    append(bytes, m_simulation_step_index);
    const auto num_minos = static_cast<MinoCount>(m_mino_stack.num_minos());

    static_assert(sizeof(decltype(num_minos)) == 8);
    append(bytes, num_minos);

    for (const auto& mino : m_mino_stack.minos()) {
        static_assert(sizeof(Coordinate) == 1);

        static_assert(sizeof(decltype(mino.position().x)) == 1);
        append(bytes, mino.position().x);

        static_assert(sizeof(decltype(mino.position().y)) == 1);
        append(bytes, mino.position().y);

        static_assert(sizeof(std::underlying_type_t<TetrominoType>) == 1);
        append(bytes, std::to_underlying(mino.type()));
    }
    return bytes;
}


namespace {

    template<typename Value>
    void compare_values(
            const std::string_view name,
            const Value& this_value,
            const Value& other_value,
            const bool log_result,
            bool& result
    ) {
        if (this_value != other_value) {
            if (log_result) {
                spdlog::error("{} do not match ({} vs. {})", name, this_value, other_value);
            }
            result = false;
        }
    }
} // namespace

bool TetrionSnapshot::compare_to(const TetrionSnapshot& other, const bool log_result) const {
    bool snapshots_are_equal = true;

    compare_values("tetrion indices", m_tetrion_index, other.m_tetrion_index, log_result, snapshots_are_equal);
    compare_values("levels", m_level, other.m_level, log_result, snapshots_are_equal);
    compare_values("scores", m_score, other.m_score, log_result, snapshots_are_equal);
    compare_values("numbers of lines cleared", m_lines_cleared, other.m_lines_cleared, log_result, snapshots_are_equal);
    compare_values(
            "simulation step indices", m_simulation_step_index, other.m_simulation_step_index, log_result,
            snapshots_are_equal
    );

    const auto mino_stacks_are_equal = (m_mino_stack == other.m_mino_stack);
    if (mino_stacks_are_equal) {
        if (log_result) {
            std::stringstream ss;
            ss << m_mino_stack;
        }
    } else {
        if (log_result) {
            std::stringstream ss;
            ss << m_mino_stack << " vs. " << other.m_mino_stack;
            spdlog::error("mino stacks do not match ({})", ss.str());
        }
        snapshots_are_equal = false;
    }

    return snapshots_are_equal;
}
