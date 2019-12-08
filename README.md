# Image Resize SSAA

Image Resize using Super Sampling Anti-Aliasing technique

## Getting Started

This project contains:
* Serial implementation
* Parallel implementation using pthread library


### How to run

All implementations require running the **make** command:
```
make
```

* Serial version

```
./homework-serial inputImageName outputImageName resizeFactor
```
 * Parallel version using pthread library

 ```
 ./homework-pthread inputImageName outputImageName resizeFactor numberOfThreads
 ```

## Profiling

### Serial implementation  
  
Profiling for this implementation is available in the **serial/profiling** folder.   
We extracted 10 running times for this version using 2, 3, 4 and 8 as values for the resize factor. The results were plotted with gnuplot. In order to display the graph, run the following commands:

```
gnuplot
load "plot"
```

## Authors

* **Bianca-Larisa Matei** - 342C1 
* **Anca Suliman** - 342C1
