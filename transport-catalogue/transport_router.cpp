#include "transport_router.h"

using namespace graph;

TransportRouter::TransportRouter(const graph::DirectedWeightedGraph<double>& graph): Router<double>(graph)
{
}

const graph::DirectedWeightedGraph<double>& TransportRouter::GetGraph() const
{
	return graph_;
}
