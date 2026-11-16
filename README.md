
# CART3R GD AI

An experimental Geometry Dash AI mod for Android.

**Version:** 0.5.0  
**Game version:** 2.2081  
**Platform:** Android64  
**Geode SDK:** 5.11.2

## Features

- Automatic player input.
- Adaptive Learning Mode.
- Tracks best progress on each level.
- Tracks deaths and attempts.
- Saves timing experiments between attempts.
- Adjustable press duration and learning rate.

## Adaptive Learning

CART3R experiments with different input timings.

After each attempt, it:
1. Records the progress reached.
2. Saves a new personal best.
3. Adjusts its timing when an attempt does not improve.
4. Tries again using the updated timing.

This is an early learning prototype. It does not visually recognize obstacles or understand level layouts.

## Settings

| Setting | Description |
|---|---|
| Auto Control | Enables automatic inputs |
| Adaptive Learning | Enables timing adjustments |
| Press Duration | Controls input hold duration |
| Release Duration | Controls starting interval |
| Learning Rate | Controls adjustment size |

## Installation

1. Build the Android64 mod using GitHub Actions.
2. Download the build artifact.
3. Extract the `.geode` file.
4. Install it through Geode.

## Project Status

CART3R is experimental. Build success does not guarantee that automatic gameplay or adaptive learning works correctly in every level.

## Developer

CART3R
