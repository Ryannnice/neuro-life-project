# NeuroLife Project  
***The University of York & Guangzhou University***

This project focuses on **Bio-Inspired Models and Lifelong Learning Mechanisms for Robotics and Embodied Intelligence**. It is a collaborative initiative between the **Computational Autonomous Learning Systems (CALS) Lab**, Department of Computer Science, University of York, and the **Machine Life and Intelligence Research Centre**, School of Mathematics and Information Sciences, Guangzhou University.

Specifically, **NeuroLife** explores biologically-plausible mechanisms for machine intelligence in the following domains:



## 🔹 1. Neuro-Embodied Micro-Robots (`/micro_embodied`)

This direction focus on **Motion Perception**
Implementing a biologically-inspired visuomotor control system for micro-robots, modeled on the insect LPLC2 neuron and integrated with low-level motion control.

### 1. `colias_core/`
Core algorithm of Attention-LPLC2 neural network model running on STM32F427 chip, including visual perception model and visuomotor pathway for motion control. It includes:

- Pre-processed visual motion perception
- Attention-based LPLC2 motion integrating mechanism
- Fly-visuomotor-inspired motion control loop on STM32 hardware

Target platform: [**Colias Micro-Robot**](https://link.springer.com/chapter/10.1007/978-3-319-96728-8_17)

### 2. `colias_sim/`
This is a software simulation of the Colias hardware processing pipeline. It allows:

- Debugging and testing neural models without hardware
- Visualizing attention-driven motion decisions
- Experimenting with different neuron parameters

Simulation environment: Microsoft Visual C++ 2022, in Microsoft Visual Studio Professional 2022 (64 bit) - Current
Version: 17.11.5




## 🔹 2. ROS-based Lifelong Robotics (`/ros_robots`)
- This direction focus on **Motion Control**
- Bio-inspired models and biologically-plausible mechanisms for life-long learning machine intelligence, robotics and autonomous systems.
- Uses TurtleBot3 + ROS2




##  Project Documentation
See `/docs` for documentation and `/results` for experimental records.

