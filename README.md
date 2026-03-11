# Game AI Project

This repository contains an Unreal Engine-based simulation focused on autonomous agent movement and spatial optimization. The project covers the implementation of various steering behaviors, combined steering logic, and graph-based navigation.

# Basic Steering Behaviors
Individual behaviors that calculate a desired velocity for an agent:
- Seek & Flee: Moving directly toward or away from a target.
- Arrive: Slowing down as the agent approaches a target radius to prevent overshooting.
- Pursuit & Evade: Predicting a target's future position based on its current velocity.
- Wander: Randomized movement using a projected circle to create smooth, organic paths.
- Face: Rotating the agent to look toward a specific target.

# Combined Steering
Complex behaviors created by blending multiple simple behaviors together:
- Blended Steering: Assigning weights (0.0 to 1.0) to multiple behaviors and taking the weighted average.
- Priority Steering: An ordered list where the agent executes the first "valid" behavior (e.g., "Evade" if a threat is near, otherwise "Wander").

# Flocking (Spatial Partitioning)
To handle large groups of agents (Boids), the system utilizes Uniform Grid Spatial Partitioning. This avoids the $O(n^2)$ bottleneck where every agent checks every other agent.
- Grid System: The world is divided into cells, agents only check for neighbors in their current and adjacent cells.
- Performance: Significant FPS improvements in high-density scenarios (e.g., jumping from ~8 FPS to ~13 FPS at 1000 agents).

# Graph Theory & Pathfinding
Implementation of graph-based structures for environment navigation:
- Graph Representation: Uses an Edge List notation where nodes (Vertices) and connections (Edges) define the traversable space.
- Eulerian Trails: Algorithms to determine "Eulerianity" and find paths that visit every connection in a graph exactly once.
- Graph Editor: A real-time component allowing for the creation, movement, and deletion of nodes and connections directly in the simulation.

# Credits & Resources
- Engine: Unreal Engine (C++).
