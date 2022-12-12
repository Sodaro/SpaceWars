#pragma once
#include <map>
#include <vector>

#include "SDL/SDL.h"

namespace input {
enum class Axis { kHorizontal, kVertical };

constexpr SDL_Scancode USED_SCANCODES[] = {
    SDL_SCANCODE_W,    SDL_SCANCODE_A,      SDL_SCANCODE_S,
    SDL_SCANCODE_D,    SDL_SCANCODE_UP,     SDL_SCANCODE_DOWN,
    SDL_SCANCODE_LEFT, SDL_SCANCODE_RIGHT,  SDL_SCANCODE_SPACE,
    SDL_SCANCODE_X,    SDL_SCANCODE_ESCAPE, SDL_SCANCODE_RETURN};

class Handler {
  static std::map<Axis, std::vector<SDL_Scancode>> axis_mappings;
  static std::map<SDL_Scancode, bool> key_states;

 public:
  static void Initialize() {
    for (int i = 0; i < sizeof(USED_SCANCODES) / sizeof(SDL_Scancode); i++)
      key_states[USED_SCANCODES[i]] = false;

    axis_mappings = {{Axis::kHorizontal,
                      {SDL_SCANCODE_RIGHT, SDL_SCANCODE_D, SDL_SCANCODE_LEFT,
                       SDL_SCANCODE_A}},
                     {Axis::kVertical,
                      {
                          SDL_SCANCODE_UP,
                          SDL_SCANCODE_W,
                          SDL_SCANCODE_DOWN,
                          SDL_SCANCODE_S,
                      }}};
  }

  static void Update() {
    SDL_Event e;
    // Poll all the events in the event queue
    while (SDL_PollEvent(&e) != 0) {
      key_states[e.key.keysym.scancode] = e.key.state;
    }
  }
  static float GetAxis(Axis axis) {
    const auto& a = axis_mappings[axis];
    return (float)((key_states[a[0]] | key_states[a[1]]) -
                   (key_states[a[2]] | key_states[a[3]]));
  };

  static bool IsKeyUp(SDL_Scancode scan_code) { return !key_states[scan_code]; }

  static bool IsKeyDown(SDL_Scancode scan_code) {
    return key_states[scan_code];
  }
};

std::map<Axis, std::vector<SDL_Scancode>> Handler::axis_mappings = {};
std::map<SDL_Scancode, bool> Handler::key_states;
}  // namespace input
