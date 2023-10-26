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

TODO

## Divergent Execution Paths

When running code on across traditional compute nodes and BlueFields, it's potentially
advantageous to have divergent execution paths - meaning the BlueField and host nodes
execute different or varying amounts of work.

TODO