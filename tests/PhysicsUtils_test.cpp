#include <gtest/gtest.h>
#include "PhysicsUtils.hpp"
#include <glm/glm.hpp>
#include <cmath>

using namespace gear_engine::physics_utils;

TEST(PhysicsUtilsTest, CalculatesCorrectPhaseAlignment) {
    float pi = 3.14159265359f;
    
    // Test 1: Two identical gears (r=1.0 -> 4 teeth) on the X axis
    // Gear A is at origin, unrotated. Gear B is at (2.0, 0, 0).
    // Gear A has a tooth at angle 0. Gear B must have a gap at angle PI.
    // Gap of Gear B is at PI/4, so it must rotate by PI - PI/4 = 3PI/4.
    float phi_b_1 = CalculateMeshRotationOffset(1.0f, 1.0f, glm::vec3(0.0f), glm::vec3(2.0f, 0.0f, 0.0f), 0.0f);
    EXPECT_NEAR(phi_b_1, pi - (pi / 4.0f), 0.001f);
    
    // Test 2: Mismatched sizes. r_a=2.0 (8 teeth), r_b=1.0 (4 teeth).
    // On X axis. A unrotated. A has tooth at 0.
    // B gap is at PI/4. So B needs to rotate by 3PI/4.
    float phi_b_2 = CalculateMeshRotationOffset(2.0f, 1.0f, glm::vec3(0.0f), glm::vec3(3.0f, 0.0f, 0.0f), 0.0f);
    EXPECT_NEAR(phi_b_2, pi - (pi / 4.0f), 0.001f);
    
    // Test 3: If A is rotated by 1 tooth (2PI/8 = PI/4)
    // The contact point still sees a tooth of A. So B should be the same!
    float phi_b_3 = CalculateMeshRotationOffset(2.0f, 1.0f, glm::vec3(0.0f), glm::vec3(3.0f, 0.0f, 0.0f), pi / 4.0f);
    
    // The formula gives: phi_b = 0 + pi - pi/4 + (8/4)*(0 - pi/4) = 3pi/4 - pi/2 = pi/4
    // Wait, if A rotates by PI/4, A's tooth #2 is at 0.
    // If B's rotation becomes PI/4, let's check its gap.
    // B has gap at PI/4. If rotated by PI/4, its gap is at PI/2.
    // But the contact point is at PI! We need B's gap at PI.
    // Actually, A rotates by PI/4, B MUST rotate by - (N_a/N_b) * PI/4 = -2 * PI/4 = -PI/2.
    // Original B was 3PI/4. 3PI/4 - PI/2 = PI/4.
    // So B's gap will be at PI/4 (original gap) + PI/4 (rotation) = PI/2? No!
    // B has gaps at PI/4, 3PI/4, 5PI/4, 7PI/4.
    // If rotated by PI/4, gaps are at PI/2, PI, 3PI/2, 2PI.
    // Ah, a gap is exactly at PI! So they mesh perfectly!
    EXPECT_NEAR(std::fmod(phi_b_3 + 2*pi, 2*pi), std::fmod(pi / 4.0f + 2*pi, 2*pi), 0.001f);
}
