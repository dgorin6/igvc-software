# Add Noise to Simulation

*Issue #552*

**Author:**
- David Gorin

## The Problem

The problem I am trying to solve is how to add extra noise to the gazebo simulation. This problem is important because we need to be able to see if the localization and mapping nodes can take more stress than the current noise generated.
## Proposed Solution

I am going to simulate noise for the x and y position of the robot using a random normal distribution

- add normal distribution randomizer
- add noise to x,y,z, and RPY
- have it so the randomizer's standard deviaion can be set in the launch file(simulation.launch)

## Questions & Research

I need to figure out how to implement normal distributions in c++


## Overall Scope

### Affected Packages

- igvc_gazebo
    - ground_truth/main.cpp
    - simulation.launch

### Schedule

Subtask 1 (2/xx/22): Create a normal distribution randomizer and use it to add noise to x pose and y pose

Subtask 2 (2/xx/22): Make it so that the standard deviation can be set in the launch files

Code Review: (2/xx/22)
