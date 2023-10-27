# Introduction to Heterogeneous BlueField Development

This repo is meant to provide introductory details on developing applications for
cross-architecture, BlueField and Host computing nodes.

## Heterogeneous Execution

BlueFields currently run Arm architecture. This is in contrast to most traditional cluster
computing nodes running on x86. Let's look at a simple example: running `hostname` on two nodes - 1 BlueField
and 1 traditional computing node. A traditional Slurm job executing on homogeneous architectures
would be simple. However, multiple architectures introduces additional complexity.

Slurm has capabilities for heterogeneous job execution. To learn more, look ()[]here TODO.
However, when Slurm jobs are dispatched, the environment variables aren't properly managed
for this usecase. Thus, the user must manage it directly. We can do this by configuring
some environment variables and our application versions in our `.bashrc` file. This ensures
that the correct executables and variables are set regardless of the set of execution nodes.
Since this research group primarily uses the Rogues Gallery and the Thor cluster, we'll look
at examples directly applicable to them.

Let's start on the Thor cluster. Here's an example bash snippet to add to your `.bashrc` to
properly configure execution from both node architectures. You may want to adjust certain paths
as you see fit, but copying blindly *should* work.

```bash
arch=$(uname -i)
FILE=/tmp/hpcx

if [ "$arch" == 'x86_64' ]; then
       ln -s /global/software/centos-8.x86_64/modules/gcc/8.3.1/hpcx/2.15.0/ /tmp/hpcx
       export PATH=/global/software/centos-8.x86_64/modules/gcc/8.3.1/hpcx/2.14.0/ucx/bin:$PATH
export LD_LIBRARY_PATH=/usr/local/lib:/usr/local/lib64:/global/software/centos-8.x86_64/modules/gcc/8.3.1/hpcx/2.14.0/ucx/lib:/global/software/centos-8.x86_64/modules/gcc/8.3.1/hpcx/2.14.0/ompi/lib/:$LD_LIBRARY_PATH
elif [ "$arch" == 'aarch64' ]; then
      ln -s /global/software/rocky-9.aarch64/modules/gcc/11/hpcx/2.15 /tmp/hpcx
      export PATH=/global/software/rocky-9.aarch64/modules/gcc/11/hpcx/2.15/ucx/bin:/tmp/hpcx/ompi/tests/osu-micro-benchmarks-5.6.2:$PATH
      export LD_LIBRARY_PATH=/usr/local/lib:/usr/local/lib64:/global/software/rocky-9.aarch64/modules/gcc/11/hpcx/2.15/ucx/lib:/global/software/rocky-9.aarch64/modules/gcc/11/hpcx/2.15.0/ompi/lib/:$LD_LIBRARY_PATH
fi

source /tmp/hpcx/hpcx-init.sh
hpcx_load
```

From a Thor login node, let's allocate the nodes we want to use, ssh into the BlueField node we
just allocated, then run a heterogeneous MPI command. Change the requested nodes and parameters
as you see fit, but ensure that one traditional host and one BlueField are allocated.

```bash
[user@login02]$ salloc -p thor --nodes=2 --ntasks-per-node=1 --time=00:05:00 -w thor014,thorbf3a014
[user@thor014]$ ssh thorbf3a014
[user@thorbf3a014]$ mpirun --oversubscribe -np 1 -H thor014 hostname : -np 1 -H thorbf3a014 hostname
thorbf3a014.hpcadvisorycouncil.com
thor014.hpcadvisorycouncil.com
```

If everything works, you should get output from the last command similar to the above. This
shows that the execution of `hostname` was run on both hosts. It's simple, but it's something!


## Heterogeneous Compilation

Most people want to run more meaningful code than the above, but to do that, we have to compile
our own code. To simultaneously run on both architectures, separate compilers,
dependencies, and executables must be properly maintained. Depending on your application,
this will result in varying difficulty.

It's possible to compile both the host and BlueField programs on x86 nodes, but cross-compiling
BlueField code is beyond the scope of this repository. If you need DOCA libraries,
[NVIDIA's documentation](https://docs.nvidia.com/doca/archive/doca-v2.2.0/developer-guide/index.html)
is very helpful, otherwise, a basic cross-compilation setup can be used.

Instead, let's start simpler and compile the host executable on the host, and the BlueField
executable on the BlueField. `hello-bluefield.cpp` is a simple program made to show heterogeneous
execution and code divergence based on node type. When running code on across traditional
compute nodes and BlueFields, it's potentially advantageous to have divergent execution paths -
meaning the BlueField and host nodes execute different or varying amounts of work.

First, we allocate two nodes then separately compile both versions of the executable. Finally,
we execute the mpirun command from the BlueField node.

```
[user@login01]$ salloc -p thor --nodes=2 --ntasks-per-node=1 --time=00:05:00 -w thor008,thorbf3a008
salloc: Granted job allocation 99999
[user@thor008]$ mpicxx hello-bluefield.cpp -o hello-bluefield-host.out
[user@thor008]$ ssh thorbf3a008
# Change directory to the proper place
[user@thorbf3a008]$ mpicxx hello-bluefield.cpp -o hello-bluefield-bf.out
[user@thorbf3a008]$ mpirun -np 1 -H thor008:2 hello-bluefield-host.out : -np 1 -H thorbf3a008:2 hello-bluefield-bf.out
=== Job: 2 processes ===
[thor008:p0/2::c0] Hello, world. I'm a host node!
[thorbf3a008:p1/2::c0] Hello, world. I'm a BlueField node!
```

## MiniMD Host/BlueField Execution

Further increasing the complexity of our example programs, let's now look at running
[MiniMD](https://github.com/hpcgarage/miniMD/tree/force_on_bf) (a scaled-down version of the
LAMMPS code) across BlueFields and Hosts. Since miniMD is OpenMP-enabled, we'll run two
threads on each node.

The process of building and executing this is the
same as above:
- Allocate nodes

    ```
    [user@login01]$ salloc -p thor --nodes=2 --ntasks-per-node=2 --time=00:15:00 -w thor014,thorbf3a014
    ```
- Compile separate executables for each architecture
    - Host

        ```
        [user@thor014]$ cd <miniMD-folder>/ref
        [user@thor014]$ make clean && rm miniMD_openmpi_host
        rm -r Obj_*
        [user@thor014]$ make openmpi arch=intel
        ... [ignoring output for readability]
        [user@thor014]$ mv miniMD_openmpi miniMD_openmpi_host
        ```

    - BlueField

        ```
        [user@thor014]$ ssh thorbf3a014
        [user@thorbf3a014]$ cd <miniMD-folder>/ref
        [user@thorbf3a014]$ make clean && rm miniMD_openmpi_bf
        [user@thorbf3a014]$ make openmpi arch=arm AVX=no
        [user@thorbf3a014]$ mv miniMD_openmpi miniMD_openmpi_bf
        ```

- From a BlueField node, run a heterogeneous MPI job

    ```
    [user@thorbf3a014]$ mpirun -np 1 -H thor014 miniMD_openmpi_host -t 2 : -np 1 -H thorbf3a014 miniMD_openmpi_bf -t 2
    # Create System:
    # Done ....
    # miniMD-Reference 2.0 (MPI+OpenMP) output ...
    # Run Settings:
            # MPI processes: 2
            # OpenMP threads: 2
    ...
    [output truncated for readability]
    ...
    # Performance Summary:
    # MPI_proc OMP_threads nsteps natoms t_total t_force t_neigh t_comm t_other performance perf/thread grep_string t_extra
    2 2 100 131072 8.443480 5.010997 0.496549 2.861831 0.074103 1552345.720422 388086.430105 PERF_SUMMARY 0.026152
    ```

Your output should mimic the above, but the reported number of MPI processes and OpenMP threads
should be the exact same. Again, we've done something simple yet more complex than what we did before.


## BlueField and Beyond

For further development, the above process can be followed - but hopefully with more automated
processes than shown here. Additionally, the
[Nvidia DOCA SDK](https://developer.nvidia.com/networking/doca) can be harnessed to extract
optimum performance from the hardware.