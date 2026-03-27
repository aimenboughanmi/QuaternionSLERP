#pragma once

#include <vector>

#include "math/Vec3.h"

struct Cube {
	static std::vector<Vec3> createUnitCube() {
		return {
			{-1, -1, -1}, {-1, -1,  1}, {-1,  1, -1}, {-1,  1,  1},
			{ 1, -1, -1}, { 1, -1,  1}, { 1,  1, -1}, { 1,  1,  1}
		};
	}
};
