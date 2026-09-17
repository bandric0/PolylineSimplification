# PolylineSimplification

You are dealing with high-resolution map data where coastlines, borders, and mountain ridges contain thousands of points, causing rendering lag and consuming too much memory. Your task is to implement an efficient $O(n \log n)$ algorithm to simplify these polylines. By iteratively removing the least significant points—determined by the area of the triangle they form with their immediate neighbors (Visvalingam-Whyatt approach)—you must reduce the number of segments while preserving the macroscopic shape of the original curve.

## Author

- [Boško Andrić] — [`bandric0`](https://github.com/bandric0), index: `1012/2025`

## Problem formulation

The project is inspired by Kattis — [Polyline Simplification](https://open.kattis.com/problems/polylines)

## Dependencies and Installation

The project requires:

- CMake 3.5 or newer
- A C++ compiler with C++17 support
- Qt 5 or Qt6 Widgets

On Ubuntu or Debian, install all required dependencies with:

```bash
sudo apt update
sudo apt install build-essential cmake qt6-base-dev

```

## Building and Running application

```bash
cmake -B build/
cmake --build build
./build/PolylineSimplification
```
