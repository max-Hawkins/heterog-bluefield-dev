#include <iostream>
#include <sched.h>
#include <unistd.h>
#include <mpi.h>
#include <sstream>
#include <string.h>

using namespace std;

int
main(int argc, char* argv[])
{
  MPI_Init(&argc, &argv);

  int rank, num_procs;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

  if(rank == 0)
    cerr << "=== Job: " << num_procs << " processes" << " ===" << endl;

  MPI_Barrier(MPI_COMM_WORLD);

#define MAXLEN_HOSTNAME 127 // @TODO: Determine this length more "canonically" using `sysconf()`
  char hostname[MAXLEN_HOSTNAME+1];
  gethostname(hostname, MAXLEN_HOSTNAME);

  // Determine host node type via node name (specific to Thor cluster)
  int isHost;
  string node_type;

  if(hostname[4] == 'b'){
      isHost = 0;
      node_type = "BlueField";
  }
  else{
      isHost = 1;
      node_type = "host";
  }

  bool erase=false;
  int index=0;
  while(hostname[index]!='\0')
  {
    if(hostname[index]=='.')
      erase=true;
    if(erase)
      hostname[index]='\0';
    index++;
  }

  int cpu = sched_getcpu();
  stringstream msg;
  msg << "[" << hostname << ":p" << rank << "/" << num_procs << "::c" << cpu
      << "] Hello, world. I'm a " << node_type << " node!" << endl;
  cerr << msg.str();

  MPI_Finalize();
  return 0;
}