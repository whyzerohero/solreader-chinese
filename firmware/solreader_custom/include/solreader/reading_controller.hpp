#pragma once
#include <utility>
#include "solreader/contracts.hpp"
#include "solreader/reference_reader.hpp"

namespace solreader {
// The official remote callback can enqueue InputEvent here on the UI task.
// No BLE driver, pairing, sleep or display-waveform code is replaced.
class ReadingController final : public InputConsumer {
public:
    ReadingController(reference::Reader &reader, std::function<void()> open_menu)
        : reader_(reader), menu_(std::move(open_menu)) {}
    void on_input(const InputEvent &event) override {
        if (reader_.busy()) return;
        switch (txt::official_command(static_cast<int>(event.action))) {
        case txt::ReadingCommand::previous: redraw_ = reader_.previous() || redraw_; break;
        case txt::ReadingCommand::next: redraw_ = reader_.next() || redraw_; break;
        case txt::ReadingCommand::menu: if (menu_) menu_(); break;
        case txt::ReadingCommand::none: break;
        }
    }
    void step() { if (reader_.busy()) redraw_ = reader_.step() || redraw_; }
    bool take_redraw() { const bool r = redraw_; redraw_ = false; return r; }
private:
    reference::Reader &reader_;
    std::function<void()> menu_;
    bool redraw_ = false;
};
} // namespace solreader
