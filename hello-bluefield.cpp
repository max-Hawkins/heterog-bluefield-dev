#include <iostream>
#include <sched.h>
#include <unistd.h>
#include <mpi.h>
#include <sstream>
#include <string.h>

#define MAXLEN_HOSTNAME 127 // @TODO: Determine this length more "canonically" using `sysconf()`

using namespace std;

// Query and return the presence of a given PCI device where ID = "<vendorID>:<deviceID>"
int pciDevicePresent(string ID){
  string cmd = "lspci -d " + ID;
  FILE * stream;
  const int max_buffer = 256;
  char buffer[max_buffer];

  cmd.append(" 2>&1"); // Do we want STDERR?
  stream = popen(cmd.c_str(), "r");

  if (stream) {
    char __ = fgetc(stream);
    if(feof(stream))
      return 0;
    else
      return 1;
  }
  return -1;
}


int main(int argc, char* argv[])
{
  MPI_Init(&argc, &argv);

  int rank, num_procs;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

  if(rank == 0)
    cerr << "=== Job: " << num_procs << " processes" << " ===" << endl;

  MPI_Barrier(MPI_COMM_WORLD);

  char hostname[MAXLEN_HOSTNAME+1];
  gethostname(hostname, MAXLEN_HOSTNAME);

  // Determine node type via PCI Device Presence (default to traditional host)
  int isHost = 1;
  string node_type = "host";

  // Check for BlueField 2 PCI Bridge Device presence
  if(pciDevicePresent("15b3:1978")){
    isHost = 0;
    node_type = "BlueField 2";
  }
  // Check for BlueField 3 PCI Bridge Device presence
  if(pciDevicePresent("15b3:197b")){
    isHost = 0;
    node_type = "BlueField 3";
  }

  // Cleanup hostnames for printing
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