# Butterworth-Optimization
Butterworth Filter implementation for UVic's Embedded Software Engineering course.

## Milestone Zero: Documentation and Characterization
Describe the computing scenario using in UML and outline the design implementation.

Characterize the problem and determine the desired precision and floating point representation.

Goal: Be able to describe a Butterworth filter, what it is used for, why it is used, and a scenario in which the performance of such a filter is critical *(may be theoretical, used to frame the need for optimization)*

## Milestone One: Basic Implementation and Bottleneck identification
Implement a Butterworth filter in **C** using fixed point arithmetic for a 32bit ARM processor with no floating point unit. Test and profile the implemented Butterworth filter to determine performance bottlenecks.

ARM assembly code should be produced and the performance of the pure software solution without optimization should be estimated.

## Milestone Two: Software Performance Optimization
Based on the identified performance bottle necks attempt to improve function performance by:
1. Inlining function(s)
2. Use compiler optimizations
3.  Hand optimize the function(s) in assembly

Techniques such as software pipelining and loop unrolling should be attempted during the hand optimization stage.

Code should be profiled, noting performance and correctness changes at each step of the optimization process

## Milestone Three: Hardware Accelerator
Based on the optimized assembly code identify a function that could be sped up using hardware acceleration. Define a new additional instruction to be added to the instruction set that will use the designed hardware accelerator. The created functionality is not expected to be able to be assembled as adding new OP codes to the assembler is out of scope of project.
1. Design the accelerator using existing microcode without multiplication and division. Two solutions are expected, one for a 1-issue slot and one for a 2-issue slot microcode engine. *(Unsure of exactly what he means by this)*
2. Design custom hardware to implement the accelerator

Performance in this stage should be theoretically computed as it will not be able to be run on actual hardware or within the software simulator.

**Ask if we will be required to implement this in VHDL, it seems like this is greyed out / not required?**

## Milestone Four: Technical Report
Create a report of no more than 20 pages, 11/12 point font, single spaced and having sections:
1. Front Page: Title, Author, Affiliation
2. Introduction: short description of the problem domain, performance requirements, description of project members contributions, and layout of the project (steps taken, etc.)
3. Theoretical Background
4. Description of the design process
5. Performance evaluation of the various process steps
6. Conclusions
7. Bibliography
