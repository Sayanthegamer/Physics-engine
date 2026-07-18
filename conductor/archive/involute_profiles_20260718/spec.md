# Specification: Fix Gear Tooth Clipping (Involute Profiles)

## Overview
Update the existing trapezoidal gear teeth generation to use mathematically correct involute curves, resolving the current clipping issues during rotation. This track also introduces automatic phase alignment for newly spawned gears to ensure perfect initial meshing, and exposes advanced gear parameters to the user interface.

## Functional Requirements
- **Involute Profile SDF:** Update the custom SDF GPU rendering shader to procedurally generate involute gear teeth profiles based on module, pressure angle, and teeth count.
- **Auto-Meshing (Phase Alignment):** Implement calculation logic to automatically determine the exact rotation offset for a newly placed gear so that its teeth perfectly mesh with the target gear it snaps to.
- **UI Parameters:** Expose "Pressure Angle" and "Clearance" controls in the existing ImGui gear properties panel.

## Non-Functional Requirements
- **Performance:** The procedural SDF shader modifications must maintain extreme efficiency to meet the project's budget hardware target (e.g., Intel i3 integrated graphics).
- **Architecture:** Keep CPU-side parameter management compliant with strict Data-Oriented Design (DOD) principles.

## Acceptance Criteria
- Gears of mismatched sizes (e.g., 10 teeth and 40 teeth) can mesh and rotate without visual clipping.
- When a new gear is placed, it automatically snaps into a perfectly meshed position with its neighbor without requiring manual rotation tweaks.
- Adjusting the Pressure Angle or Clearance sliders in the UI updates the gear visuals smoothly in real-time.

## Out of Scope
- Full 3D bevel or helical gear generation (this track focuses on standard spur gears first).
