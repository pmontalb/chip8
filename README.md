# Chip8 Interpreter

I developed this project as a learning exercise for developing something more interesting (like a GB emulator). I keep referring to this as an emulator, although I'm aware this is just an interpreter.

There are a number of online resources that I used for learning how Chip8 works:
- https://austinmorlan.com/posts/chip8_emulator/
- http://devernay.free.fr/hacks/chip8/C8TECH10.HTM
- https://tobiasvl.github.io/blog/write-a-chip-8-emulator/

There's a bit of controversy and discussion about how certain instructions work, but I decided to stick to the same decisions
as Austin Morlan's work.

I have no experience with GUIs, and I decided to use mahi-gui (https://github.com/mahilab/mahi-gui)
, which is wrapper of some other popular libraries such as ImGui. The other external dependency I used is spdlog (https://github.com/gabime/spdlog).
There are some utilities that I could have written as separate side projects (such as RingBuffer, StopWatch and Rng), but they're
so small (and potentially not that useful) that it's probably better to keep them here.

I spent way too long on this project, but I wanted to satisfy the following requirements as good practise:
- Dependency Injection
  - The Cpu depends on RAM, Display and Keypad, but I didn't want to hardcode this choice in the Cpu itself. That's why instead of having the instructions methods that just accepts the opcode, I often pass additional interface parameters
- gcc-11 and clang-13 compile clean
- all gcc and clang sanitizers run clean
  - corollary: no undefined behaviour
      - this requires a bit more verbosity, as, for instance, the wrap-around of std::uint8_t has to be done via an explicit cast
      - clang-ubsan complains about the standard library implementation. I managed to silent that for the Rng, but not for bitset
      - I could equivalently have had some test roms run at compile-time, having all constexpr functions, like Jason Turner does, and it would have been cool, but that's for another time
- 100% code coverage
  - Everything needs to be unit tested
  - I didn't manage to run clang-coverage in a fully automated way. This gives more info than gcov's, but the html report produced with gcovr is enough

A design choice that I'm not fully satisfied with is the fact that the Chip8 interpreter object is templated. I could have injected dependencies at creation alternatively, but I decided to use templates as it was quicker to write unit tests for.

A screenshot is for your reference here:
![ScreenShot](https://raw.github.com/pmontalb/chip8/master/chip8.png)

I spent just the necessary time for having this GUI to visualize the emulator display and some other tools for helping debugging issues.
I'm not an expert with ImGui and I wanted to spend more time on the coding/quality parts rather than having a super cool GUI.
