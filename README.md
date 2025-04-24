# somp 
Strength Of Materials Project:

An attempt to use the integration method or just brute force to calculate the
stresses in a beam and display it visually. The project will be done in SDL to
make it a learning process, maybe it will be updated to use a higher level (or
even lower??) framework one day

# Notes 
Just touched up a bit, last thing I was doing was deciding if the wall
reaction force should be part of the raw sections (either way will break one of
the test cases) 

## Rendering
* Should probably seperate rendering arrows, graphs and beam stress gradient
* Dont want to calculate stress for every single pixel, tends to be inaccurate
  and takes a lot of computation but its the easiest so im gonna do it
* Rendering chunks based off functions seems better but more complicated and
  still needs to calculate for every pixel in function but can do chunks for
  point forces (probably most common)
* Do we want to represent the beam from 0-1? (Probably)
* Need to display the distances that each load is from base (relative to each
  other would be cool)
* Draw single arrow for point force
* Draw a bunch of arrows for functions spaced by some setting

## Dependencies
* Make sure that the following are either in path or in the top directory:
    * `SDL2.dll`
    * `SDL2_ttf.dll`
* Testing:
    * ejtest: [](https://github.com/elj40/ejtest.git)
