#include "RHI/FrameGraph.hpp"

#include <stack>
#include <set>
#include <unordered_set>

namespace RHI
{
	void IFrameGraph::TopologicalSort()
	{
		Graph unsortedGraph = m_graph;
		Graph sortedGraph;
		
		for(auto& node : unsortedGraph)
		{
			if(node.IsRootNode())
			{
				sortedGraph.push_back(node);
			}
			else 
			{
				// insert it after all of its producers.
			}
		}
		
		m_graph = sortedGraph;
	}
} // namespace RHI
