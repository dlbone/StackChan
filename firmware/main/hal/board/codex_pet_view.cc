/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "codex_pet_view.h"
#include "hal_bridge.h"
#include <assets/assets.h>
#include <hal/hal.h>
#include <cstdio>

namespace {

constexpr uint32_t kBackgroundColor = 0x05070B;
constexpr uint32_t kRunningColor = 0x67B7FF;
constexpr uint32_t kNeedsInputColor = 0xFFD166;
constexpr uint32_t kReadyColor = 0x76E6A2;
constexpr uint32_t kBlockedColor = 0xFF6B6B;

struct AnimationDefinition {
    const char* name;
    size_t offset;
    size_t frameCount;
    uint32_t frameDurationMs;
    uint32_t labelColor;
    const char* label;
};

constexpr std::array<AnimationDefinition, 5> kAnimations{{
    {"idle", 0, 6, 180, kRunningColor, "IDLE"},
    {"ready", 6, 4, 180, kReadyColor, "READY"},
    {"blocked", 10, 8, 180, kBlockedColor, "BLOCKED"},
    {"needs_input", 18, 6, 180, kNeedsInputColor, "NEEDS INPUT"},
    {"running", 24, 6, 140, kRunningColor, "RUNNING"},
}};

const AnimationDefinition& animationForState(CodexPetState state)
{
    switch (state) {
        case CodexPetState::Ready:
            return kAnimations[1];
        case CodexPetState::Blocked:
            return kAnimations[2];
        case CodexPetState::NeedsInput:
            return kAnimations[3];
        case CodexPetState::Running:
            return kAnimations[4];
        case CodexPetState::Idle:
        default:
            return kAnimations[0];
    }
}

}  // namespace

CodexPetView::~CodexPetView()
{
    if (_animationTimer != nullptr) {
        lv_timer_delete(_animationTimer);
        _animationTimer = nullptr;
    }
    if (_panel != nullptr) {
        lv_obj_delete(_panel);
        _panel = nullptr;
    }
}

void CodexPetView::init(lv_obj_t* parent)
{
    loadFrames();

    _panel = lv_obj_create(parent);
    lv_obj_set_size(_panel, 320, 240);
    lv_obj_align(_panel, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(_panel, lv_color_hex(kBackgroundColor), 0);
    lv_obj_set_style_bg_opa(_panel, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(_panel, 0, 0);
    lv_obj_set_style_radius(_panel, 0, 0);
    lv_obj_set_style_pad_all(_panel, 0, 0);
    lv_obj_remove_flag(_panel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(_panel, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(_panel, handleClick, LV_EVENT_CLICKED, nullptr);

    _image = lv_image_create(_panel);
    lv_obj_set_size(_image, 192, 208);
    lv_obj_align(_image, LV_ALIGN_BOTTOM_MID, 0, 0);

    _statusLabel = lv_label_create(_panel);
    lv_obj_set_width(_statusLabel, 304);
    lv_obj_set_pos(_statusLabel, 8, 6);
    lv_label_set_long_mode(_statusLabel, LV_LABEL_LONG_DOT);
    lv_obj_set_style_text_font(_statusLabel, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_align(_statusLabel, LV_TEXT_ALIGN_CENTER, 0);

    selectAnimation(CodexPetState::Idle);
    setData(CodexPetData{});
    _animationTimer = lv_timer_create(handleAnimationTimer, kAnimations[0].frameDurationMs, this);
    hide();
}

void CodexPetView::setData(const CodexPetData& data)
{
    if (_state != data.state) {
        selectAnimation(data.state);
    }

    const AnimationDefinition& animation = animationForState(data.state);
    std::string label = "CODEX  ";
    label += animation.label;
    if (!data.workspace.empty()) {
        label += "  ";
        label += data.workspace;
    }
    if (data.activeTasks > 1) {
        char taskCount[16];
        std::snprintf(taskCount, sizeof(taskCount), "  +%d", data.activeTasks - 1);
        label += taskCount;
    }
    lv_label_set_text(_statusLabel, label.c_str());
    lv_obj_set_style_text_color(_statusLabel, lv_color_hex(animation.labelColor), 0);
}

void CodexPetView::show()
{
    if (_panel == nullptr) {
        return;
    }
    if (lv_obj_has_flag(_panel, LV_OBJ_FLAG_HIDDEN)) {
        lv_obj_remove_flag(_panel, LV_OBJ_FLAG_HIDDEN);
        lv_obj_move_foreground(_panel);
        if (_animationTimer != nullptr) {
            lv_timer_resume(_animationTimer);
        }
    }
}

void CodexPetView::hide()
{
    if (_panel != nullptr) {
        lv_obj_add_flag(_panel, LV_OBJ_FLAG_HIDDEN);
    }
    if (_animationTimer != nullptr) {
        lv_timer_pause(_animationTimer);
    }
}

void CodexPetView::loadFrames()
{
    size_t frameIndex = 0;
    for (const auto& animation : kAnimations) {
        for (size_t animationFrame = 0; animationFrame < animation.frameCount; ++animationFrame) {
            char filename[64];
            std::snprintf(filename, sizeof(filename), "glimble_%s_%u.png", animation.name,
                          static_cast<unsigned>(animationFrame));
            _frames[frameIndex++] = assets::get_image(filename);
        }
    }
}

void CodexPetView::selectAnimation(CodexPetState state)
{
    _state = state;
    const AnimationDefinition& animation = animationForState(state);
    _animationOffset = animation.offset;
    _animationFrameCount = animation.frameCount;
    _animationFrameIndex = 0;
    if (_image != nullptr) {
        lv_image_set_src(_image, &_frames[_animationOffset]);
    }
    if (_animationTimer != nullptr) {
        lv_timer_set_period(_animationTimer, animation.frameDurationMs);
        lv_timer_reset(_animationTimer);
    }
}

void CodexPetView::advanceAnimation()
{
    if (_image == nullptr || _animationFrameCount == 0) {
        return;
    }
    _animationFrameIndex = (_animationFrameIndex + 1) % _animationFrameCount;
    lv_image_set_src(_image, &_frames[_animationOffset + _animationFrameIndex]);
}

void CodexPetView::handleAnimationTimer(lv_timer_t* timer)
{
    auto* view = static_cast<CodexPetView*>(lv_timer_get_user_data(timer));
    view->advanceAnimation();
}

void CodexPetView::handleClick(lv_event_t* event)
{
    (void)event;
    static uint32_t lastToggleTick = 0;
    const uint32_t now = GetHAL().millis();
    if (lastToggleTick != 0 && now - lastToggleTick < 2000) {
        return;
    }
    if (hal_bridge::is_xiaozhi_ready()) {
        lastToggleTick = now;
        hal_bridge::toggle_xiaozhi_chat_state();
    }
}
