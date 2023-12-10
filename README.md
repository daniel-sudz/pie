# Osciliscope Music PIE Team 
This contains code for our final PIE project (osciliscope music) at Olin College of Engineering. 

# Arduino
The ```arduino``` folder includes all the code run on an ```Arduino Uno``` to handle our custom keyboard and potentiometer inputs which are streamed to serial output. 

# Visualizer 
The ```visualizer``` folder is a high-performance c++ program for consuming arduino serial input and driving the osciliscope music output. 

# Install
To clone the repo correctly use the following flags
```bash 
git clone --recurse-submodules https://github.com/daniel-sudz/pie/
```
The ```--recurse-submodules``` flag will include the ```portaudio``` and ```libserial``` dependencies necessary for building the ```visualizer``` project. 

# Build Instruction
Take a look at the official CI build scripts to setup a similar environment for your operating system here: https://github.com/daniel-sudz/pie/blob/main/.github/workflows/build.yaml.

We currently officially test against the following build targets: `[ubuntu-20.04, ubuntu-22.04, ubuntu-latest, macos-12, macos-13, macos-latest]`.
