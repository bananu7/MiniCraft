#pragma once

float dot(float x, float y, float z, float* g);
float noise(float xin, float yin, float zin);
float simplex_noise(int octaves, float x, float y, float z);
float simplex_noise(int octaves, float x, float y);