/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once

#include <array>
#include <string>
#include <lvgl.h>

enum class CodexPetState {
    Idle,
    Running,
    NeedsInput,
    Ready,
    Blocked,
};

struct CodexPetData {
    CodexPetState state = CodexPetState::Idle;
    int activeTasks = 0;
};

class CodexPetView {
public:
    ~CodexPetView();

    void init(lv_obj_t* parent);
    void setData(const CodexPetData& data);
    void show();
    void hide();

private:
    static constexpr size_t kFrameCount = 30;

    lv_obj_t* _panel = nullptr;
    lv_obj_t* _image = nullptr;
    lv_obj_t* _statusLabel = nullptr;
    lv_timer_t* _animationTimer = nullptr;
    std::array<lv_image_dsc_t, kFrameCount> _frames{};
    CodexPetState _state = CodexPetState::Idle;
    size_t _animationOffset = 0;
    size_t _animationFrameCount = 0;
    size_t _animationFrameIndex = 0;

    void loadFrames();
    void selectAnimation(CodexPetState state);
    void advanceAnimation();
    static void handleAnimationTimer(lv_timer_t* timer);
    static void handleClick(lv_event_t* event);
};
