#pragma once

#include "graph.h"
#include "router.h"

class TransportRouter: public graph::Router<double>
{
public:
	TransportRouter(const graph::DirectedWeightedGraph<double>& graph);

	const graph::DirectedWeightedGraph<double>& GetGraph() const;
private:
};