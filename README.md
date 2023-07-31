# Butterworth-Optimization
Butterworth Filter implementation for UVic's Embedded Software Engineering 440 course.

## Milestone Zero: Documentation and Characterization
Describe the computing scenario using in UML and outline the design implementation.

Characterize the problem and determine the desired precision and floating point representation.

Goal: Be able to describe a Butterworth filter, what it is used for, why it is used. Be able to describe the computing scenario and the desired precision and floating point representation.
## Milestone One: Basic Implementation and Bottleneck identification
Implement a Butterworth filter in **C** using fixed point arithmetic for a 32-bit ARM processor with no floating point unit. Test and profile the implemented Butterworth filter to determine performance bottlenecks.

ARM assembly code should be produced and the performance of the pure software solution without optimization should be estimated.

## Milestone Two: Software Performance Optimization
Based on the identified performance bottle necks attempt to improve function performance by:
1. Inlining function(s)
2. Use compiler optimizations
3.  Hand optimize the function(s) in assembly

Techniques such as software pipelining and loop unrolling should be attempted during the hand optimization stage.

Code should be profiled, noting performance and correctness changes at each step of the optimization process

## Milestone Three: Technical Report
Create a report of no more than 20 pages, 11/12 point font, single spaced and having sections:
1. Front Page: Title, Author, Affiliation
2. Introduction: short description of the problem domain, performance requirements, description of project members contributions, and layout of the project (steps taken, etc.)
3. Theoretical Background
4. Description of the design process
5. Performance evaluation of the various process steps
6. Conclusions
7. Bibliography
