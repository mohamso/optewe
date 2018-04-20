## 3D Finite Difference Modeling for the Elastic Wave Equation 

This program models a elastic wave propagation for a single source. It is possible to specify the position for the
source and the source signature as well. The receiver grid can be specified by either by depth and grid start and end
points on the x- and y-axis, or by an input file specifying the grid points. As default, the output file is the receiver
recording file, but more components can be added to the receiver file (see below). It is also possible to save the
snapshots of all the different velocity and stress field on the boundaries as well as in the whole model. 

Receiver types for receiver output file:

rectype = 1: Pressure

rectype = 2: Pressure-Vz

rectype = 3: OBC (Pressure-Vz-Vx-Vy)

rectype = 4: Vz,Vx,Vy

rectype = 5: Vz

rectype = 6: Vx

rectype = 7: Vy

rectype = 8: \del Vx + \del Vz (time-derivative of dilatation)

To manipulate boundary conditions:

The Perfectly Matched Layer (PML) approach is used to take care of the wave propagation at the boundaries.
Three default values are used if not specified by input parameters. ghost_border is maybe to most important input
parameter to change, since it controls how many grid cells to use in the PML layers. If you experience reflections from
the boundaries try to change ghost_border (and amax and kmax if necessary).

### Build ###
Before the application can be built make sure that
a C compiler is loaded. Typically, on a HPC cluster
it is sufficient to write
something like this (given that you would like to
utilize Intel's C compiler suite and MKL):

    module purge
    module load intel/2016.2.181

Once the modules have been loaded, you are ready
to compile the sources into a binary file.
    
    cd code
    make -DHAVE_VISUALIZE=OFF -DSAVE_RECEIVERS=OFF ..
    make -j8

In the example above, writing vtk files used for visualization is turned off.
Likewise, saving of a receiver file in .csv format is also disabled.
If you prefer to turn these options on, please use the following cmake step
    
    make -DHAVE_VISUALIZE=ON -DSAVE_RECEIVERS=ON

The flags are interchangeable, which means that it is possible to turn
one of the flags on and the other off.
    
### Run ###
The application can be launched like this:
    
    ./optewe Nx Ny Nz Nt source_type
    
where Nx, Ny, Nz are the number of grid points along
the x, y and z axis, while Nt represents the number of iterations.

The source type represents the type of the stress exhibited by the
stress source. Valid source type values are: 1, 2 and 3, where:

    1 = stress monopole source
    2 = force monopole source
    3 = force dipole source
   
A typical way of executing the application is:
    
    ./optewe 512 512 512 100 1

### Problem sizes and typical values ###
The elastic wave equation is a physical equation such that the simulations should somewhat mimic the physical world.
First some basic physics: In an fluid, i.e. water, no shear waves can propagate and thus is Vs equal to zero.
The pressure wave velocity (Vp) is approximately 1500m/s in water. The density for water is 1000 kg/m^3. In a solid,
i.e. rocks or more general elastic medium, shear waves can propagate. In loose mediums Vs can be as small
as below 100m/s. This is far below the practical Vs that can be used in the modeling due to numerical dispersion.
The density is just a parameter that controls the amplitude on the signals, i.e. reflections at interfaces.
Therefore, it can, for simplicity, in all applications be put to 1000. Here are two examples of properties that can be
used in test runs:

#### Water ####
    
    Rho = 1000
    Vp = 1500
    Vs = 0

#### Solid ####

    Rho = 1000
    Vp = 2200
    Vs = 1000

The stability requirement is based on the time sampling dt, and a theoretical limit is given as

    dt = \leq \frac{2*dx}{\sqrt{3} \pi Vp_{max}}


The spatial discretizing should be Dx=Dy=Dz=10.0m. With this sampling, models of the different sizes will have these
physical sizes:

    128^3 = 1280m^3
    256^3 = 2560m^3
    512^3 = 5120m^3
    1024^3 = 10240m^3

This means that if a source is placed in the middle of the uniform model the P-waves will travel with the speed
as Vp and S-waves with the speed Vs. To see how long a wave travels from the source position just the following formula

    distance = speed*traveltime

Example: If 1024^3 is used and the source is in the middle of the model, the distance the waves must travel to get to
the boundary is approximately 5000m. With a speed of 2000m, this will take 2.5 s (remember to take the time delay on
the source into account when you compute this). With a timestep of dt=0.001, you will need 2500 time steps to propagate
to the boundaries.

## Performance
The total floating-point operation for each iteration is approximately 177 FLOPs.
Every forward and backward derivative function (dx_forward, dx_backward, etc) are called three times.
Moreover, each of the derivatives performs 4 FLOPs, meaning that the total FLOP rate for the derivative functions
are 4 FLOP * 3^3 = 108 FLOPs.

The compute_vx, compute_vy and compute_vz functions are only called once. Each of these functions perform 7 FLOPs,
placing the total FLOP count for these functions 21 FLOPs (3 * 7 FLOPs). Similarly, the three compute_sxy,
compute_syz and compute_sxz functions are also called once. Each of these function perform 8 FLOPs or 
24 FLOPs (3 * 8 FLOPs) in total. Finally, the batched compute_sxx_syy_szz function is also called once,
but performs 24 FLOPs. Thus, the total flop rate is: 108 + 21 + 24 + 24 = 177 FLOPs.

## Verification

There are at least _three_ ways to verify the correctness of the simulation.
By visual examination or by checking the output of the simulation with the serial version that is known to be correct.

In order to check the correctness using visual examination, make sure to compile using the following macro ```-DHAVE_VISUALIZE=ON```. At the end of a successful run, the code will output a ```.vtk``` file of the code, which can be imported and visualized using for example (Paraview)[http://paraview.org].

Unless you are an expert on geophysics, verification by visual examination can be a tedious process involving large files. Alternatively, you can verify the correctness of the solution using a Python script that checks the output of the simulation against solution file in ```.csv``` format that are generated when running the code with a single OpenMP thread. 

To generate a solution file, compile the code with the following macro ```-DSAVE_RECEIVERS=ON``` and then run the code using a single OpenMP thread ```export OMP_NUM_THREADS=1```. Next, rename the file and run the code using the desired number of OpenMP threads. Now, use a tool such as ```vimdiff``` to verify the correctness of ```.csv``` file from the the parallel code with the serial code. If the files are identical, your parallel solution is correct.

## License and Citation

OptEWE is released under the [BSD 3-Clause license](https://github.com/mohamso/optewe/blob/master/LICENSE).

Please cite OptEWE in your publications if it helps your research:

    @inproceedings{SourouriRRLHSK17,
    author    = {Mohammed Sourouri and
               Espen Birger Raknes and
               Nico Reissmann and
               Johannes Langguth and
               Daniel Hackenberg and
               Robert Sch{\"{o}}ne and
               Per Gunnar Kjeldsberg},
               title     = {Towards fine-grained dynamic tuning of {HPC} applications on modern
               multi-core architectures},
               booktitle = {Proceedings of the International Conference for High Performance Computing,
               Networking, Storage and Analysis, {SC} 2017, Denver, CO, USA, November
               12 - 17, 2017},
               pages     = {41:1--41:12},
               year      = {2017},
               url       = {http://doi.acm.org/10.1145/3126908.3126945},
               doi       = {10.1145/3126908.3126945}
               }
