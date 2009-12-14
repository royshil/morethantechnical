###################################################################
#                                                                 #
#    MAXFLOW - software for computing mincut/maxflow in a graph   #
#                        Version 2.02                             #
#    http://www.cs.cornell.edu/People/vnk/software.html           #
#                                                                 #
#    Vladimir N. Kolmogorov                                       #
#    vnk@cs.cornell.edu                          2001             #
#                                                                 #
###################################################################

1. Introduction.

This software library is a modification of the maxflow algorithm
described in

	An Experimental Comparison of Min-Cut/Max-Flow Algorithms
	for Energy Minimization in Computer Vision.
	Yuri Boykov and Vladimir Kolmogorov.
	In Third International Workshop on Energy Minimization
	Methods in Computer Vision and Pattern Recognition, September 2001

This algorithm was originally developed at Siemens.
The main modification is that two trees are used for finding
augmenting paths - one grows from the source and the other
from the sink. (The original algorithm used only the former one).
Details will be described in my PhD thesis.

It can be used only for research purposes. IF YOU USE THIS SOFTWARE,
YOU SHOULD CITE THE AFOREMENTIONED PAPER IN ANY RESULTING PUBLICATION.

Tested under windows, Visual C++ 6.0 compiler and unix (SunOS 5.8
and RedHat Linux 7.0, GNU c++ compiler).

##################################################################

2. Graph representation.

There are two versions of the algorithm using different
graph representations (adjacency list and forward star).
The former one uses more than twice as much memory as the
latter one but is 10-20% faster.

Memory allocation (assuming that all capacities are 'short' - 2 bytes):

                 |   Nodes    |   Arcs
------------------------------------------
Adjacency list   | *24 bytes  | *14 bytes
Forward star     | *28 bytes  |  6 bytes

(* means that often it should be rounded up to be a multiple of 4
- some compilers (e.g. Visual C++) seem to round up elements
of arrays unless the are structures containing only char[].)

Note that arcs are always added in pairs - in forward and reverse directions.
Arcs between nodes and terminals (the source and the sink) are
not stored as arcs, but rather as a part of nodes.

The assumption for the forward star representation is that
the maximum number of arcs per node (except the source
and the sink) is much less than ARC_BLOCK_SIZE (1024 by default).

Both versions have the same interface.

##################################################################

3. Example usage.

This section shows how to use the library to compute
a minimum cut on the following graph:

		        SOURCE
		       /       \
		     1/         \2
		     /      3    \
		   node0 -----> node1
		     |   <-----   |
		     |      4     |
		     \            /
		     5\          /6
		       \        /
		          SINK

///////////////////////////////////////////////////

#include <stdio.h>
#include "graph.h"

void main()
{
	Graph::node_id nodes[2];
	Graph *g = new Graph();

	nodes[0] = g -> add_node();
	nodes[1] = g -> add_node();
	g -> set_tweights(nodes[0], 1, 5);
	g -> set_tweights(nodes[1], 2, 6);
	g -> add_edge(nodes[0], nodes[1], 3, 4);

	Graph::flowtype flow = g -> maxflow();

	printf("Flow = %d\n", flow);
	printf("Minimum cut:\n");
	if (g->what_segment(nodes[0]) == Graph::SOURCE)
		printf("node0 is in the SOURCE set\n");
	else
		printf("node0 is in the SINK set\n");
	if (g->what_segment(nodes[1]) == Graph::SOURCE)
		printf("node1 is in the SOURCE set\n");
	else
		printf("node1 is in the SINK set\n");

	delete g;
}

///////////////////////////////////////////////////
