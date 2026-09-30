// Direct Genesis ROM analysis. This is intentionally independent of Ghidra.
// It decodes enough Motorola 68000 instructions to follow the reset path,
// direct subroutine calls, branches, absolute references, and likely pointer
// tables. The output is evidence for the manual C++ reconstruction.

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <deque>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <iostream>
#include <map>
#include <optional>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace fs = std::filesystem;

namespace {

constexpr uint32_t kRomStart = 0x000000;
constexpr uint32_t kHeaderStart = 0x000100;
constexpr uint32_t kVectorEnd = 0x000100;
constexpr uint32_t kGenesisIoStart = 0xA10000;
constexpr uint32_t kVdpData = 0xC00000;
constexpr uint32_t kVdpControl = 0xC00004;
constexpr uint32_t kIndirectDispatchTable = 0x12CCB4;
// 0x1F3C4 dispatches every command below 0xFA through this table.  The
// earlier analysis treated only the first 32 entries as code, which omitted
// source command 0x30/0x31 (Stoker's debt and Jump House branches) and 0xE9
// (the Stoker arrival branch).  0xFA..0xFF are terminal command values and
// belong to the adjacent data, not this dispatch table.
constexpr size_t kIndirectDispatchCount = 0xFA;
constexpr uint32_t kRoutineSelectTable = 0xF3DC6;
constexpr size_t kRoutineSelectCount = 22;
constexpr uint32_t kStateRoutinePairTable = 0xF3E1E;
constexpr size_t kStateRoutinePairCount = 10;
constexpr uint32_t kEntityDispatchTable = 0x490A;
constexpr size_t kEntityDispatchCount = 15;
constexpr uint32_t kIndexedVariantDescriptorTable = 0x4ACC;
constexpr size_t kIndexedVariantDescriptorCount = 4;

std::string hex(uint32_t value, unsigned width = 6) {
    std::ostringstream out;
    out << "0x" << std::uppercase << std::hex << std::setw(static_cast<int>(width))
        << std::setfill('0') << value;
    return out.str();
}

int32_t sign_extend_8(uint8_t value) {
    return static_cast<int8_t>(value);
}

int32_t sign_extend_16(uint16_t value) {
    return static_cast<int16_t>(value);
}

class Rom {
public:
    explicit Rom(std::vector<uint8_t> bytes) : bytes_(std::move(bytes)) {}

    static Rom load(const fs::path& path) {
        std::ifstream input(path, std::ios::binary);
        if (!input) {
            throw std::runtime_error("cannot open ROM: " + path.string());
        }
        input.seekg(0, std::ios::end);
        const auto size = input.tellg();
        input.seekg(0, std::ios::beg);
        std::vector<uint8_t> bytes(static_cast<size_t>(size));
        input.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
        if (!input && !input.eof()) {
            throw std::runtime_error("cannot read ROM: " + path.string());
        }
        return Rom(std::move(bytes));
    }

    size_t size() const { return bytes_.size(); }

    bool contains(uint32_t address, size_t count = 1) const {
        return address <= bytes_.size() && count <= bytes_.size() - address;
    }

    uint8_t u8(uint32_t address) const {
        if (!contains(address)) {
            throw std::out_of_range("ROM byte address outside image");
        }
        return bytes_[address];
    }

    uint16_t u16(uint32_t address) const {
        if (!contains(address, 2)) {
            throw std::out_of_range("ROM word address outside image");
        }
        return static_cast<uint16_t>((static_cast<uint16_t>(bytes_[address]) << 8) |
                                      static_cast<uint16_t>(bytes_[address + 1]));
    }

    uint32_t u32(uint32_t address) const {
        return (static_cast<uint32_t>(u16(address)) << 16) | u16(address + 2);
    }

    const std::vector<uint8_t>& bytes() const { return bytes_; }

private:
    std::vector<uint8_t> bytes_;
};

enum class Flow {
    Fallthrough,
    ConditionalBranch,
    UnconditionalBranch,
    Call,
    Return,
    Stop,
    Illegal
};

struct Decoded {
    uint32_t address = 0;
    uint16_t opcode = 0;
    uint32_t size = 2;
    Flow flow = Flow::Illegal;
    std::optional<uint32_t> target;
    std::vector<uint32_t> directReferences;
    std::string text;
    bool valid = false;
};

struct FunctionInfo {
    uint32_t address = 0;
    std::string label;
    std::set<uint32_t> calls;
    std::vector<uint32_t> instructions;
};

unsigned operand_size_bytes(unsigned size_code) {
    switch (size_code) {
    case 0: return 1;
    case 1: return 2;
    case 2: return 4;
    default: return 0;
    }
}

int ea_extension_words(uint8_t mode, uint8_t reg, unsigned operand_size) {
    switch (mode) {
    case 0: // Dn
    case 1: // An
    case 2: // (An)
    case 3: // (An)+
    case 4: // -(An)
        return 0;
    case 5: // d16(An)
    case 6: // d8(An, Xn)
        return 1;
    case 7:
        switch (reg) {
        case 0: // absolute short
        case 2: // d16(PC)
        case 3: // d8(PC, Xn)
            return 1;
        case 1: // absolute long
            return 2;
        case 4: // immediate
            return operand_size == 4 ? 2 : 1;
        default:
            return -1;
        }
    default:
        return -1;
    }
}

std::optional<uint32_t> effective_address_target(const Rom& rom, uint32_t address,
                                                 uint8_t mode, uint8_t reg,
                                                 unsigned extension_word_offset) {
    if (mode != 7 || (reg != 0 && reg != 1 && reg != 2)) {
        return std::nullopt;
    }
    const uint32_t extension = address + 2 + extension_word_offset * 2;
    if (reg == 0) {
        return static_cast<uint32_t>(sign_extend_16(rom.u16(extension))) & 0x00FFFFFFU;
    }
    if (reg == 1) {
        return rom.u32(extension) & 0x00FFFFFFU;
    }
    return static_cast<uint32_t>(static_cast<int32_t>(extension + 2) +
                                 sign_extend_16(rom.u16(extension))) & 0x00FFFFFFU;
}

std::string flow_name(Flow flow) {
    switch (flow) {
    case Flow::Fallthrough: return "fallthrough";
    case Flow::ConditionalBranch: return "conditional";
    case Flow::UnconditionalBranch: return "branch";
    case Flow::Call: return "call";
    case Flow::Return: return "return";
    case Flow::Stop: return "stop";
    case Flow::Illegal: return "illegal";
    }
    return "unknown";
}

class Analyzer {
public:
    Analyzer(const Rom& rom, fs::path output) : rom_(rom), output_(std::move(output)) {}

    void run() {
        fs::create_directories(output_);
        seed_vectors();
        seed_verified_indirect_tables();
        discover_functions();
        write_disassembly();
        write_report();
        write_functions();
        write_callgraph();
        write_pointer_tables();
        write_verified_tables();
        write_indirect_control_flow();
        write_regions();
        write_strings();
        write_direct_references();
    }

    size_t function_count() const { return functions_.size(); }
    size_t instruction_count() const { return instructions_.size(); }

private:
    const Rom& rom_;
    fs::path output_;
    std::map<uint32_t, Decoded> instructions_;
    std::map<uint32_t, FunctionInfo> functions_;
    std::map<uint32_t, std::string> seed_labels_;
    std::set<uint32_t> function_queue_seen_;
    std::deque<uint32_t> function_queue_;
    std::set<uint32_t> code_bytes_;

    Decoded decode(uint32_t address) const {
        Decoded result;
        result.address = address;
        if (!rom_.contains(address, 2) || (address & 1U) != 0) {
            result.text = "invalid-address";
            return result;
        }

        const uint16_t op = rom_.u16(address);
        result.opcode = op;
        result.flow = Flow::Fallthrough;

        auto finish = [&](uint32_t size, std::string text, Flow flow = Flow::Fallthrough) {
            if (size < 2 || (size & 1U) != 0 || !rom_.contains(address, size)) {
                result.text = "truncated/invalid";
                result.flow = Flow::Illegal;
                return result;
            }
            result.size = size;
            result.text = std::move(text);
            result.flow = flow;
            result.valid = true;
            return result;
        };

        auto add_ea_reference = [&](uint8_t mode, uint8_t reg, unsigned offset_words) {
            if (auto target = effective_address_target(rom_, address, mode, reg, offset_words)) {
                result.directReferences.push_back(*target);
            }
        };

        // Branches and subroutine branches.
        if ((op & 0xF000U) == 0x6000U) {
            const uint8_t condition = static_cast<uint8_t>((op >> 8) & 0x0FU);
            const uint8_t displacement8 = static_cast<uint8_t>(op & 0xFFU);
            const bool word_displacement = displacement8 == 0;
            const uint32_t size = word_displacement ? 4 : 2;
            const int32_t displacement = word_displacement
                ? sign_extend_16(rom_.u16(address + 2))
                : sign_extend_8(displacement8);
            // On 68000, both the byte and word branch displacements are
            // relative to the address immediately after the opcode word. The
            // word form's extension word is part of the displacement, not the
            // base PC used for this calculation.
            const uint32_t target = static_cast<uint32_t>(static_cast<int32_t>(address + 2) + displacement);
            result.target = target;
            result.directReferences.push_back(target);
            if (condition == 0) {
                return finish(size, "BRA " + hex(target), Flow::UnconditionalBranch);
            }
            if (condition == 1) {
                return finish(size, "BSR " + hex(target), Flow::Call);
            }
            return finish(size, "Bcc[" + std::to_string(condition) + "] " + hex(target),
                          Flow::ConditionalBranch);
        }

        // DBcc and Scc / ADDQ / SUBQ.
        if ((op & 0xF0F8U) == 0x50C8U) {
            const uint32_t target = static_cast<uint32_t>(static_cast<int32_t>(address + 2) +
                                                           sign_extend_16(rom_.u16(address + 2)));
            result.target = target;
            result.directReferences.push_back(target);
            return finish(4, "DBcc " + hex(target), Flow::ConditionalBranch);
        }
        if ((op & 0xF0C0U) == 0x50C0U) {
            const uint8_t mode = static_cast<uint8_t>((op >> 3) & 7U);
            const uint8_t reg = static_cast<uint8_t>(op & 7U);
            const int extension = ea_extension_words(mode, reg, 1);
            if (extension < 0) {
                return result;
            }
            add_ea_reference(mode, reg, 0);
            return finish(2 + static_cast<uint32_t>(extension) * 2, "Scc", Flow::Fallthrough);
        }
        if ((op & 0xF000U) == 0x5000U) {
            const unsigned size_code = (op >> 6) & 3U;
            const unsigned operand_size = operand_size_bytes(size_code);
            const uint8_t mode = static_cast<uint8_t>((op >> 3) & 7U);
            const uint8_t reg = static_cast<uint8_t>(op & 7U);
            const int extension = operand_size == 0 ? -1 : ea_extension_words(mode, reg, operand_size);
            if (extension < 0) {
                return result;
            }
            add_ea_reference(mode, reg, 0);
            return finish(2 + static_cast<uint32_t>(extension) * 2, "ADDQ/SUBQ", Flow::Fallthrough);
        }

        // Immediate operations: ORI, ANDI, SUBI, ADDI, EORI, CMPI.
        if ((op & 0xF000U) == 0x0000U) {
            // Immediate bit operations use 0x08xx and carry an immediate
            // bit number before the effective-address extension. They are
            // common in the entity/post-pass code; treating them as generic
            // byte arithmetic shifts every following instruction boundary.
            if ((op & 0xFF00U) == 0x0800U) {
                const uint8_t mode = static_cast<uint8_t>((op >> 3) & 7U);
                const uint8_t reg = static_cast<uint8_t>(op & 7U);
                const int extension = ea_extension_words(mode, reg, 1);
                if (extension < 0) {
                    return result;
                }
                add_ea_reference(mode, reg, 1);
                return finish(4 + static_cast<uint32_t>(extension) * 2,
                              "immediate-bit-op", Flow::Fallthrough);
            }
            const uint16_t operation = op & 0x0F00U;
            const bool immediate_operation = operation == 0x0000U || operation == 0x0200U ||
                operation == 0x0400U || operation == 0x0600U || operation == 0x0A00U ||
                operation == 0x0C00U;
            const unsigned size_code = (op >> 6) & 3U;
            const unsigned operand_size = operand_size_bytes(size_code);
            if (immediate_operation && operand_size != 0) {
                const uint8_t mode = static_cast<uint8_t>((op >> 3) & 7U);
                const uint8_t reg = static_cast<uint8_t>(op & 7U);
                const int extension = ea_extension_words(mode, reg, operand_size);
                if (extension < 0) {
                    return result;
                }
                add_ea_reference(mode, reg, 1);
                const unsigned immediate_words = operand_size == 4 ? 2 : 1;
                return finish(2 + immediate_words * 2 + static_cast<uint32_t>(extension) * 2,
                              "immediate-op", Flow::Fallthrough);
            }
            // Dynamic/static bit operations and MOVEP all carry extra words.
            if ((op & 0xF100U) == 0x0100U) {
                const uint8_t mode = static_cast<uint8_t>((op >> 3) & 7U);
                const uint8_t reg = static_cast<uint8_t>(op & 7U);
                const int extension = ea_extension_words(mode, reg, 1);
                if (extension < 0) {
                    return result;
                }
                const bool immediate_bit = (op & 0x0100U) == 0;
                add_ea_reference(mode, reg, immediate_bit ? 1 : 0);
                return finish(2 + (immediate_bit ? 2 : 0) + static_cast<uint32_t>(extension) * 2,
                              "bit-op", Flow::Fallthrough);
            }
        }

        // MOVE and MOVEQ.
        if ((op & 0xF000U) == 0x7000U) {
            return finish(2, "MOVEQ", Flow::Fallthrough);
        }
        if ((op & 0xF000U) == 0x1000U || (op & 0xF000U) == 0x2000U ||
            (op & 0xF000U) == 0x3000U) {
            const unsigned operand_size = (op & 0xF000U) == 0x2000U ? 4 :
                ((op & 0xF000U) == 0x1000U ? 1 : 2);
            const uint8_t source_mode = static_cast<uint8_t>((op >> 3) & 7U);
            const uint8_t source_reg = static_cast<uint8_t>(op & 7U);
            const uint8_t dest_mode = static_cast<uint8_t>((op >> 6) & 7U);
            const uint8_t dest_reg = static_cast<uint8_t>((op >> 9) & 7U);
            const int source_extension = ea_extension_words(source_mode, source_reg, operand_size);
            const int dest_extension = ea_extension_words(dest_mode, dest_reg, operand_size);
            if (source_extension < 0 || dest_extension < 0) {
                return result;
            }
            add_ea_reference(source_mode, source_reg, 0);
            add_ea_reference(dest_mode, dest_reg, static_cast<unsigned>(source_extension));
            return finish(2 + static_cast<uint32_t>(source_extension + dest_extension) * 2,
                          "MOVE", Flow::Fallthrough);
        }

        // System/control-flow instructions.
        if (op == 0x4E70U) return finish(2, "RESET", Flow::Fallthrough);
        if (op == 0x4E71U) return finish(2, "NOP", Flow::Fallthrough);
        if (op == 0x4E72U) return finish(4, "STOP", Flow::Stop);
        if (op == 0x4E73U) return finish(2, "RTE", Flow::Return);
        if (op == 0x4E74U) return finish(4, "RTD", Flow::Return);
        if (op == 0x4E75U) return finish(2, "RTS", Flow::Return);
        if (op == 0x4E76U) return finish(2, "TRAPV", Flow::Stop);
        if (op == 0x4E77U) return finish(2, "RTR", Flow::Return);
        if (op == 0x44FCU) return finish(4, "MOVE #imm,CCR", Flow::Fallthrough);
        if (op == 0x46FCU) return finish(4, "MOVE #imm,SR", Flow::Fallthrough);
        if (op == 0x4E7AU || op == 0x4E7BU) return finish(4, "MOVEC", Flow::Fallthrough);
        if ((op & 0xFFF0U) == 0x4E40U) return finish(2, "TRAP", Flow::Stop);
        if ((op & 0xFFF8U) == 0x4E50U) return finish(4, "LINK", Flow::Fallthrough);
        if ((op & 0xFFF8U) == 0x4E58U) return finish(2, "UNLK", Flow::Fallthrough);
        if ((op & 0xFFF0U) == 0x4E60U) return finish(2, "MOVE USP", Flow::Fallthrough);

        if ((op & 0xFFC0U) == 0x4E80U || (op & 0xFFC0U) == 0x4EC0U) {
            const uint8_t mode = static_cast<uint8_t>((op >> 3) & 7U);
            const uint8_t reg = static_cast<uint8_t>(op & 7U);
            const int extension = ea_extension_words(mode, reg, 4);
            if (extension < 0) {
                return result;
            }
            add_ea_reference(mode, reg, 0);
            if (auto target = effective_address_target(rom_, address, mode, reg, 0)) {
                result.target = *target;
            }
            const bool jump = (op & 0xFFC0U) == 0x4EC0U;
            return finish(2 + static_cast<uint32_t>(extension) * 2,
                          jump ? "JMP" : "JSR", jump ? Flow::UnconditionalBranch : Flow::Call);
        }
        if ((op & 0xF1C0U) == 0x41C0U) {
            const uint8_t mode = static_cast<uint8_t>((op >> 3) & 7U);
            const uint8_t reg = static_cast<uint8_t>(op & 7U);
            const int extension = ea_extension_words(mode, reg, 4);
            if (extension < 0) {
                return result;
            }
            add_ea_reference(mode, reg, 0);
            return finish(2 + static_cast<uint32_t>(extension) * 2, "LEA", Flow::Fallthrough);
        }
        if ((op & 0xFFC0U) == 0x4840U) {
            const uint8_t mode = static_cast<uint8_t>((op >> 3) & 7U);
            const uint8_t reg = static_cast<uint8_t>(op & 7U);
            if (mode == 0) {
                return finish(2, "SWAP", Flow::Fallthrough);
            }
            const int extension = ea_extension_words(mode, reg, 4);
            if (extension < 0) {
                return result;
            }
            add_ea_reference(mode, reg, 0);
            return finish(2 + static_cast<uint32_t>(extension) * 2, "PEA", Flow::Fallthrough);
        }
        if ((op & 0xFFB8U) == 0x4880U && ((op >> 3) & 7U) == 0) {
            return finish(2, "EXT", Flow::Fallthrough);
        }
        if ((op & 0xFB80U) == 0x4880U || (op & 0xFB80U) == 0x4C80U) {
            const uint8_t mode = static_cast<uint8_t>((op >> 3) & 7U);
            const uint8_t reg = static_cast<uint8_t>(op & 7U);
            const int extension = ea_extension_words(mode, reg, 2);
            if (extension < 0) {
                return result;
            }
            add_ea_reference(mode, reg, 2);
            return finish(4 + static_cast<uint32_t>(extension) * 2, "MOVEM", Flow::Fallthrough);
        }

        // Unary operations in the 0x4xxx block.
        if ((op & 0xFF00U) == 0x4A00U || (op & 0xFF00U) == 0x4200U ||
            (op & 0xFF00U) == 0x4400U || (op & 0xFF00U) == 0x4600U ||
            (op & 0xFF00U) == 0x4000U || (op & 0xFF00U) == 0x4400U) {
            const unsigned operand_size = operand_size_bytes((op >> 6) & 3U);
            const uint8_t mode = static_cast<uint8_t>((op >> 3) & 7U);
            const uint8_t reg = static_cast<uint8_t>(op & 7U);
            const int extension = operand_size == 0 ? -1 : ea_extension_words(mode, reg, operand_size);
            if (extension < 0) {
                return result;
            }
            add_ea_reference(mode, reg, 0);
            return finish(2 + static_cast<uint32_t>(extension) * 2, "unary-op", Flow::Fallthrough);
        }

        // MUL/DIV have a word-sized effective-address operand.
        if ((op & 0xF1C0U) == 0x80C0U || (op & 0xF1C0U) == 0xC0C0U) {
            const uint8_t mode = static_cast<uint8_t>((op >> 3) & 7U);
            const uint8_t reg = static_cast<uint8_t>(op & 7U);
            const int extension = ea_extension_words(mode, reg, 2);
            if (extension < 0) {
                return result;
            }
            add_ea_reference(mode, reg, 0);
            return finish(2 + static_cast<uint32_t>(extension) * 2, "MUL/DIV", Flow::Fallthrough);
        }

        // Address arithmetic has its size in bit 8 rather than bits 7-6.
        if ((op & 0xF0C0U) == 0x90C0U || (op & 0xF0C0U) == 0xB0C0U ||
            (op & 0xF0C0U) == 0xD0C0U) {
            const unsigned operand_size = (op & 0x0100U) != 0 ? 4 : 2;
            const uint8_t mode = static_cast<uint8_t>((op >> 3) & 7U);
            const uint8_t reg = static_cast<uint8_t>(op & 7U);
            const int extension = ea_extension_words(mode, reg, operand_size);
            if (extension < 0) {
                return result;
            }
            add_ea_reference(mode, reg, 0);
            return finish(2 + static_cast<uint32_t>(extension) * 2, "SUBA/CMPA/ADDA", Flow::Fallthrough);
        }

        // Most remaining 68000 arithmetic instructions use the low six bits
        // as one effective address. This fallback is deliberately conservative:
        // an unrecognized line-F opcode terminates a path instead of inventing
        // code inside graphics or map data.
        const unsigned top_nibble = (op >> 12) & 0x0FU;
        if (top_nibble == 0x0FU) {
            result.text = "illegal-line-F";
            return result;
        }
        const unsigned size_code = (op >> 6) & 3U;
        const unsigned operand_size = operand_size_bytes(size_code);
        const uint8_t mode = static_cast<uint8_t>((op >> 3) & 7U);
        const uint8_t reg = static_cast<uint8_t>(op & 7U);
        const int extension = operand_size == 0 ? -1 : ea_extension_words(mode, reg, operand_size);
        if (extension < 0) {
            return result;
        }
        add_ea_reference(mode, reg, 0);
        return finish(2 + static_cast<uint32_t>(extension) * 2, "arithmetic-op", Flow::Fallthrough);
    }

    void enqueue_function(uint32_t address, std::string label) {
        if (address >= rom_.size() || (address & 1U) != 0) {
            return;
        }
        if (seed_labels_.find(address) == seed_labels_.end()) {
            seed_labels_[address] = std::move(label);
        }
        if (function_queue_seen_.insert(address).second) {
            function_queue_.push_back(address);
        }
    }

    void seed_vectors() {
        // Genesis vectors are 32-bit big-endian addresses at the beginning of
        // the ROM. Include every in-ROM target, not only the reset vector.
        for (uint32_t offset = 0; offset < kVectorEnd; offset += 4) {
            const uint32_t target = rom_.u32(offset) & 0x00FFFFFFU;
            if (target < rom_.size() && (target & 1U) == 0) {
                std::ostringstream label;
                label << "Vector_" << std::uppercase << std::hex << std::setw(2)
                      << std::setfill('0') << (offset / 4);
                enqueue_function(target, label.str());
            }
        }
    }

    void seed_verified_indirect_tables() {
        // At 0x1F580 the prompt loop loads this fixed command table,
        // indexes it by a command below 0xFA, and executes the selected
        // longword as JSR (A0). Seed the complete 0x00..0xF9 target run so
        // source handlers are not silently omitted while avoiding arbitrary
        // ROM-looking longwords outside the reviewed table.
        for (size_t index = 0; index < kIndirectDispatchCount; ++index) {
            const uint32_t address = rom_.u32(
                kIndirectDispatchTable + static_cast<uint32_t>(index * 4));
            std::ostringstream label;
            label << "Dispatch_12CCB4_" << std::uppercase << std::hex
                  << std::setw(2) << std::setfill('0') << index;
            enqueue_function(address, label.str());
        }

        // 0x1E69C selects one longword from this table using FFFBED. The
        // table is a distinct run ending immediately before the paired table
        // below, so its 22 entries are seeded as a reviewed group.
        for (size_t index = 0; index < kRoutineSelectCount; ++index) {
            const uint32_t address = rom_.u32(
                kRoutineSelectTable + static_cast<uint32_t>(index * 4));
            std::ostringstream label;
            label << "RoutineSelect_F3DC6_" << std::uppercase << std::hex
                  << std::setw(2) << std::setfill('0') << index;
            enqueue_function(address, label.str());
        }

        // 0x214B0 and 0x214CE index ten eight-byte rows. A zero secondary
        // entry is an explicit no-call marker; only nonzero entries can be
        // function targets.
        for (size_t row = 0; row < kStateRoutinePairCount; ++row) {
            for (size_t column = 0; column < 2; ++column) {
                const uint32_t address = rom_.u32(
                    kStateRoutinePairTable + static_cast<uint32_t>(row * 8 + column * 4));
                if (address == 0) {
                    continue;
                }
                std::ostringstream label;
                label << "StateRoutine_F3E1E_" << std::uppercase << std::hex
                      << std::setw(2) << std::setfill('0') << row << '_'
                      << column;
                enqueue_function(address, label.str());
            }
        }

        // 0x46F6 indexes this table with the entity byte at +0xB2 and calls
        // the selected longword. Keep the known valid run separate from the
        // following non-code value at 0x4946.
        for (size_t index = 0; index < kEntityDispatchCount; ++index) {
            const uint32_t address = rom_.u32(
                kEntityDispatchTable + static_cast<uint32_t>(index * 4));
            std::ostringstream label;
            label << "EntityDispatch_490A_" << std::uppercase << std::hex
                  << std::setw(2) << std::setfill('0') << index;
            enqueue_function(address, label.str());
        }
    }

    void discover_functions() {
        while (!function_queue_.empty()) {
            const uint32_t start = function_queue_.front();
            function_queue_.pop_front();
            if (functions_.find(start) != functions_.end()) {
                continue;
            }

            FunctionInfo function;
            function.address = start;
            function.label = seed_labels_.find(start) != seed_labels_.end()
                ? seed_labels_.at(start)
                : "Function_" + hex(start, 6).substr(2);

            std::deque<uint32_t> blocks;
            std::set<uint32_t> local_instructions;
            blocks.push_back(start);
            while (!blocks.empty()) {
                uint32_t address = blocks.front();
                blocks.pop_front();
                while (rom_.contains(address, 2) && (address & 1U) == 0 &&
                       local_instructions.insert(address).second) {
                    Decoded decoded;
                    const auto existing = instructions_.find(address);
                    if (existing != instructions_.end()) {
                        decoded = existing->second;
                    } else {
                        decoded = decode(address);
                        instructions_.emplace(address, decoded);
                    }
                    if (!decoded.valid) {
                        break;
                    }

                    function.instructions.push_back(address);
                    for (uint32_t byte = 0; byte < decoded.size; ++byte) {
                        code_bytes_.insert(address + byte);
                    }
                    for (uint32_t reference : decoded.directReferences) {
                        if (decoded.flow == Flow::Call && decoded.target && reference == *decoded.target &&
                            reference < rom_.size() && (reference & 1U) == 0) {
                            function.calls.insert(reference);
                            enqueue_function(reference, "Function_" + hex(reference, 6).substr(2));
                        }
                    }

                    if (decoded.target && decoded.flow == Flow::UnconditionalBranch &&
                        *decoded.target < rom_.size() && (*decoded.target & 1U) == 0) {
                        // An absolute JMP is commonly used as a tail call in
                        // this ROM. Keep the control-flow edge, but also make
                        // the destination independently inspectable as a
                        // function entry. Relative BRA remains part of the
                        // current function's control-flow graph.
                        if (decoded.text.rfind("JMP", 0) == 0) {
                            enqueue_function(*decoded.target,
                                             "Function_" + hex(*decoded.target, 6).substr(2));
                        }
                        blocks.push_back(*decoded.target);
                        break;
                    }
                    if (decoded.target && decoded.flow == Flow::ConditionalBranch &&
                        *decoded.target < rom_.size() && (*decoded.target & 1U) == 0) {
                        blocks.push_back(*decoded.target);
                    }
                    if (decoded.flow == Flow::Return || decoded.flow == Flow::Stop ||
                        decoded.flow == Flow::Illegal) {
                        break;
                    }
                    address += decoded.size;
                }
            }
            functions_.emplace(start, std::move(function));
        }
    }

    void write_disassembly() const {
        std::ofstream out(output_ / "direct_disassembly.s");
        for (const auto& [address, instruction] : instructions_) {
            out << hex(address, 8) << "\t" << instruction.text << "\t; size="
                << instruction.size << " opcode=" << hex(instruction.opcode, 4)
                << " flow=" << flow_name(instruction.flow) << "\n";
        }
    }

    void write_report() const {
        std::ofstream out(output_ / "analysis_report.txt");
        out << "Shadowrun Genesis direct C++ analysis\n\n";
        out << "ROM size: " << rom_.size() << " bytes\n";
        out << "Header title: ";
        for (uint32_t offset = 0x120; offset < 0x150 && rom_.contains(offset); ++offset) {
            const char c = static_cast<char>(rom_.u8(offset));
            out << (c >= 32 && c <= 126 ? c : ' ');
        }
        out << "\n\nVectors:\n";
        for (uint32_t offset = 0; offset < kVectorEnd; offset += 4) {
            out << hex(offset, 2) << " -> " << hex(rom_.u32(offset) & 0x00FFFFFFU, 8) << "\n";
        }
        out << "\nDirect-analysis counts:\n";
        out << "Functions: " << functions_.size() << "\n";
        out << "Decoded instructions: " << instructions_.size() << "\n";
        out << "Marked code bytes: " << code_bytes_.size() << "\n";
        out << "\nHardware address constants seen through direct EA decoding:\n";
        std::map<uint32_t, size_t> counts;
        for (const auto& [address, instruction] : instructions_) {
            for (uint32_t reference : instruction.directReferences) {
                if ((reference & 0xFFFF00U) == 0xA10000U ||
                    (reference & 0xFFFFFCU) == kVdpData ||
                    (reference & 0xFFFFFCU) == kVdpControl ||
                    (reference & 0xFFFF00U) == 0xA11000U) {
                    ++counts[reference];
                }
            }
        }
        for (const auto& [address, count] : counts) {
            out << hex(address, 8) << "\t" << count << "\n";
        }
        out << "\nCode ranges:\n";
        if (!code_bytes_.empty()) {
            auto start = *code_bytes_.begin();
            auto previous = start;
            for (auto it = std::next(code_bytes_.begin()); it != code_bytes_.end(); ++it) {
                if (*it != previous + 1) {
                    out << hex(start, 8) << "-" << hex(previous, 8) << "\n";
                    start = *it;
                }
                previous = *it;
            }
            out << hex(start, 8) << "-" << hex(previous, 8) << "\n";
        }
    }

    void write_functions() const {
        std::ofstream out(output_ / "functions.tsv");
        out << "address\tlabel\tinstructions\tstart\tend\tcalls\n";
        for (const auto& [address, function] : functions_) {
            uint32_t end = address;
            for (uint32_t instruction : function.instructions) {
                const auto& decoded = instructions_.at(instruction);
                end = std::max(end, instruction + decoded.size - 1);
            }
            out << hex(address, 8) << "\t" << function.label << "\t"
                << function.instructions.size() << "\t" << hex(address, 8) << "\t"
                << hex(end, 8) << "\t" << function.calls.size() << "\n";
        }
    }

    void write_callgraph() const {
        std::ofstream out(output_ / "callgraph.tsv");
        out << "caller\tcallee\n";
        for (const auto& [address, function] : functions_) {
            for (uint32_t callee : function.calls) {
                out << hex(address, 8) << "\t" << hex(callee, 8) << "\n";
            }
        }
    }

    void write_pointer_tables() const {
        struct Entry { uint32_t offset; uint32_t target; };
        std::vector<Entry> entries;
        for (uint32_t offset = 0x200; rom_.contains(offset, 4); offset += 2) {
            const uint32_t target = rom_.u32(offset) & 0x00FFFFFFU;
            if (target >= 0x200 && target < rom_.size() && (target & 1U) == 0 &&
                functions_.find(target) != functions_.end()) {
                entries.push_back({offset, target});
            }
        }

        std::ofstream out(output_ / "pointer_tables.tsv");
        out << "start\tend\tentries\tfirst_target\tlast_target\n";
        size_t index = 0;
        while (index < entries.size()) {
            size_t end = index;
            while (end + 1 < entries.size() && entries[end + 1].offset == entries[end].offset + 4) {
                ++end;
            }
            const size_t count = end - index + 1;
            if (count >= 3) {
                out << hex(entries[index].offset, 8) << "\t" << hex(entries[end].offset + 3, 8)
                    << "\t" << count << "\t" << hex(entries[index].target, 8) << "\t"
                    << hex(entries[end].target, 8) << "\n";
            }
            index = end + 1;
        }
    }

    void write_verified_tables() const {
        std::ofstream out(output_ / "verified_tables.tsv");
        out << "name\toffset\tentries\twidth\tvalues\tevidence\n";

        auto word_values = [&](uint32_t offset, size_t count) {
            std::ostringstream values;
            for (size_t index = 0; index < count; ++index) {
                if (index != 0) {
                    values << ',';
                }
                values << hex(rom_.u16(offset + static_cast<uint32_t>(index * 2)), 4);
            }
            return values.str();
        };
        auto long_values = [&](uint32_t offset, size_t count) {
            std::ostringstream values;
            for (size_t index = 0; index < count; ++index) {
                if (index != 0) {
                    values << ',';
                }
                values << hex(rom_.u32(offset + static_cast<uint32_t>(index * 4)), 8);
            }
            return values.str();
        };
        auto byte_values = [&](uint32_t offset, size_t count) {
            std::ostringstream values;
            for (size_t index = 0; index < count; ++index) {
                if (index != 0) {
                    values << ',';
                }
                values << hex(rom_.u8(offset + static_cast<uint32_t>(index)), 2);
            }
            return values.str();
        };

        out << "reset_bootstrap_words\t" << hex(0x02C4, 8) << "\t3\t16\t"
            << word_values(0x02C4, 3)
            << "\tloaded by ResetEntry before RAM initialization\n";
        out << "reset_bootstrap_addresses\t" << hex(0x02CA, 8) << "\t5\t32\t"
            << long_values(0x02CA, 5)
            << "\tGenesis Z80, VDP, and control-port operands\n";
        out << "region_selector\t" << hex(0x046E, 8) << "\t4\t8\t"
            << byte_values(0x046E, 4)
            << "\tindexed by ((A10001 >> 6) & 3) at ResetEntry\n";
        out << "header_region_field\t" << hex(0x01F0, 8) << "\t16\t8\t"
            << byte_values(0x01F0, 16)
            << "\tcompared against the selected region byte\n";
        out << "vdp_configuration\t" << hex(0x0D14, 8) << "\t19\t16\t"
            << word_values(0x0D14, 19)
            << "\tconsumed by function 0x0CEC: 4 RAM words plus 15 VDP writes\n";
        out << "indexed_variant_descriptor_pointer_table\t"
            << hex(kIndexedVariantDescriptorTable, 8) << "\t"
            << kIndexedVariantDescriptorCount << "\t32\t"
            << long_values(kIndexedVariantDescriptorTable,
                           kIndexedVariantDescriptorCount)
            << "\tindexed by object word +0x0E low two bits at 0x4ADC\n";
        out << "indexed_offset_pointer_table\t" << hex(0x4BB0, 8) << "\t8\t32\t"
            << long_values(0x4BB0, 8)
            << "\tindexed by the 0x49A2/0x4ADC/0x4B6C data path\n";
        for (size_t index = 0; index < 8; ++index) {
            const uint32_t pointer = rom_.u32(0x4BB0 + static_cast<uint32_t>(index * 4));
            out << "indexed_offset_vector_" << index << "\t" << hex(pointer, 8)
                << "\t5\t16\t" << word_values(pointer, 5)
                << "\tsigned five-word offset vector consumed by DBF at 0x4BA0\n";
        }
        out << "indexed_descriptor_pointer_table\t" << hex(0x4C0C, 8)
            << "\t8\t32\t" << long_values(0x4C0C, 8)
            << "\tsecond data-pointer table referenced by the same cluster\n";
        out << "indirect_dispatch_table_12ccb4\t" << hex(kIndirectDispatchTable, 8)
            << "\t" << kIndirectDispatchCount << "\t32\t"
            << long_values(kIndirectDispatchTable, kIndirectDispatchCount)
            << "\tindexed by 0x1F580 and executed through JSR (A0)\n";
        out << "routine_select_table_f3dc6\t" << hex(kRoutineSelectTable, 8)
            << "\t" << kRoutineSelectCount << "\t32\t"
            << long_values(kRoutineSelectTable, kRoutineSelectCount)
            << "\tindexed by FFFBED at 0x1E69C and executed through JSR (A0)\n";
        out << "state_routine_pair_table_f3e1e\t" << hex(kStateRoutinePairTable, 8)
            << "\t" << (kStateRoutinePairCount * 2) << "\t32\t"
            << long_values(kStateRoutinePairTable, kStateRoutinePairCount * 2)
            << "\t8-byte rows indexed by FFFBED at 0x214B0/0x214CE; zero is no-call\n";
        out << "entity_dispatch_table_490a\t" << hex(kEntityDispatchTable, 8)
            << "\t" << kEntityDispatchCount << "\t32\t"
            << long_values(kEntityDispatchTable, kEntityDispatchCount)
            << "\tindexed by entity +0xB2 at 0x46F6; following longword is excluded\n";
    }

    void write_indirect_control_flow() const {
        std::ofstream out(output_ / "indirect_control_flow.tsv");
        out << "instruction\topcode\toperation\tea_mode\tea_register\n";
        for (const auto& [address, instruction] : instructions_) {
            if ((instruction.text != "JSR" && instruction.text != "JMP") ||
                instruction.target.has_value()) {
                continue;
            }
            out << hex(address, 8) << "\t" << hex(instruction.opcode, 4) << "\t"
                << instruction.text << "\t" << ((instruction.opcode >> 3) & 7U)
                << "\t" << (instruction.opcode & 7U) << "\n";
        }
    }

    void write_regions() const {
        constexpr uint32_t block_size = 0x1000;
        std::ofstream out(output_ / "regions.tsv");
        out << "offset\tsize\tcode_bytes\tnonzero_bytes\tprintable_bytes\tunique_bytes\tentropy_bits\tclassification\n";
        for (uint32_t offset = 0; offset < rom_.size(); offset += block_size) {
            const uint32_t end = std::min<uint32_t>(static_cast<uint32_t>(rom_.size()), offset + block_size);
            std::vector<size_t> counts(256, 0);
            size_t nonzero = 0;
            size_t printable = 0;
            size_t code = 0;
            for (uint32_t address = offset; address < end; ++address) {
                const uint8_t value = rom_.u8(address);
                ++counts[value];
                nonzero += value != 0;
                printable += value >= 32 && value <= 126;
                code += code_bytes_.find(address) != code_bytes_.end();
            }
            double entropy = 0.0;
            const double block_bytes = static_cast<double>(end - offset);
            for (size_t count : counts) {
                if (count != 0) {
                    const double probability = static_cast<double>(count) / block_bytes;
                    entropy -= probability * std::log2(probability);
                }
            }
            std::string classification = "mixed/data";
            if (code > 0x100) {
                classification = "code-dense";
            } else if (nonzero < 0x100) {
                classification = "mostly-zero";
            } else if (printable > 0x200) {
                classification = "text-like";
            } else if (entropy > 7.4) {
                classification = "high-entropy/graphics-or-compressed";
            }
            out << hex(offset, 8) << "\t" << (end - offset) << "\t" << code << "\t"
                << nonzero << "\t" << printable << "\t"
                << std::count_if(counts.begin(), counts.end(), [](size_t count) { return count != 0; })
                << "\t" << std::fixed << std::setprecision(3) << entropy << "\t"
                << classification << "\n";
        }
    }

    void write_strings() const {
        std::ofstream out(output_ / "strings.tsv");
        out << "offset\tlength\ttext\n";
        const auto& bytes = rom_.bytes();
        size_t offset = kHeaderStart;
        while (offset < bytes.size()) {
            const size_t start = offset;
            while (offset < bytes.size() && bytes[offset] >= 32 && bytes[offset] <= 126) {
                ++offset;
            }
            if (offset - start >= 6) {
                std::string text;
                for (size_t i = start; i < offset; ++i) {
                    const char c = static_cast<char>(bytes[i]);
                    text += c == '\t' ? ' ' : c;
                }
                out << hex(static_cast<uint32_t>(start), 8) << "\t" << (offset - start)
                    << "\t" << text << "\n";
            }
            ++offset;
        }
    }

    void write_direct_references() const {
        std::ofstream out(output_ / "direct_references.tsv");
        out << "instruction\treference\tkind\n";
        for (const auto& [address, instruction] : instructions_) {
            for (uint32_t reference : instruction.directReferences) {
                const bool hardware = (reference & 0xFFFF00U) == 0xA10000U ||
                    (reference & 0xFFFFFCU) == kVdpData ||
                    (reference & 0xFFFFFCU) == kVdpControl ||
                    (reference & 0xFFFF00U) == 0xA11000U;
                const bool in_rom = reference < rom_.size();
                out << hex(address, 8) << "\t" << hex(reference, 8) << "\t"
                    << (hardware ? "hardware" : (in_rom ? "rom" : "external")) << "\n";
            }
        }
    }
};

} // namespace

int main(int argc, char** argv) {
    if (argc < 2 || argc > 3) {
        std::cerr << "usage: shadowrun_rom_analyzer <rom.bin> [output-directory]\n";
        return 2;
    }
    try {
        const fs::path rom_path = argv[1];
        const fs::path output = argc == 3 ? fs::path(argv[2]) : fs::path("analysis");
        const Rom rom = Rom::load(rom_path);
        Analyzer analyzer(rom, output);
        analyzer.run();
        std::cout << "ROM bytes: " << rom.size() << "\n"
                  << "Directly decoded instructions: " << analyzer.instruction_count() << "\n"
                  << "Discovered function entries: " << analyzer.function_count() << "\n"
                  << "Reports: " << output.string() << "\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "error: " << error.what() << "\n";
        return 1;
    }
}
