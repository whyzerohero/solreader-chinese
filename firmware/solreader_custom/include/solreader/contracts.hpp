#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace solreader {

// Architecture proposal only: the logical values follow official RemoteCommand.
// Reading control only. Never forward these events to the Chinese IME.
// An adapter must preserve reading behavior and run consumers on the UI queue.
enum class InputAction : std::int8_t {
    up = 0x5, left = 0x8, confirm = 0x9, right = 0xA,
    down = 0xD, back = 0xE, none = -1
};
enum class InputSource { factory_remote, app_remote };
enum class Gesture { unspecified, press, hold, double_press, repeat };

struct InputEvent {
    InputAction action{InputAction::none};
    InputSource source{InputSource::factory_remote};
    Gesture gesture{Gesture::unspecified};
    std::optional<std::uint16_t> raw_code;
    std::uint64_t timestamp_ms{};
};
// Gestures cannot be reconstructed from logical commands alone.
// Do not synthesize release/repeat/hold events without protocol evidence.
class InputConsumer {
public:
    virtual ~InputConsumer() = default;
    virtual void on_input(const InputEvent &event) = 0;
};

enum class DocumentKind { txt, epub };
struct ReadingPosition {
    DocumentKind kind{DocumentKind::txt};
    std::string document_id;
    std::string content_fingerprint;
    std::string spine_href; // Empty for TXT. Preserve official EPUB place identity.
    std::uint64_t utf8_byte_offset{};
    std::uint32_t layout_revision{};
};
// Offset refers to normalized UTF-8 content, never an original GBK byte offset.
class ProgressStore {
public:
    virtual ~ProgressStore() = default;
    virtual std::optional<ReadingPosition> load(std::string_view document_id) = 0;
    virtual bool save_atomic(const ReadingPosition &position) = 0;
};

// Semantic extension boundary for future dedicated one-handed input hardware.
// No button count/layout, transport, protocol or factory-remote codes are defined.
enum class CompositionAction { input_symbol, select_candidate, erase, commit, cancel };
struct CompositionEvent {
    CompositionAction action{CompositionAction::cancel};
    std::string symbol_utf8; // Semantic symbol, not a physical key or wire payload.
    std::optional<std::uint32_t> candidate_index;
    std::uint64_t timestamp_ms{};
};

enum class Mode { reading, writing };
class TextComposition {
public:
    virtual ~TextComposition() = default;
    virtual void on_composition(const CompositionEvent &event) = 0;
    virtual std::string_view preedit_utf8() const = 0;
    virtual std::optional<std::string> accept_candidate() = 0;
    virtual void cancel() = 0;
};

struct NoteSnapshot {
    std::string note_id;
    std::uint64_t revision{};
    std::string utf8_text;
};
class NoteStore {
public:
    virtual ~NoteStore() = default;
    virtual std::optional<NoteSnapshot> load(std::string_view note_id) = 0;
    // Must refuse a stale expected_revision and retain the last complete note.
    virtual bool save_atomic(const NoteSnapshot &note,
                             std::uint64_t expected_revision) = 0;
};

// Future power adapter calls this while storage and UI queues still work.
// Returning false means data is pending; the caller applies a bounded policy.
class ShutdownParticipant {
public:
    virtual ~ShutdownParticipant() = default;
    virtual bool prepare_for_sleep(std::uint64_t deadline_ms) = 0;
};

} // namespace solreader
