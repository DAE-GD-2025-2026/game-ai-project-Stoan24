#pragma once
#include <map>
#include <vector>

namespace GameAI
{
	class Graph;
	class Node;

	class BFS
	{
	public:
		BFS(Graph* const pGraph);

		std::vector<Node*> FindPath(Node* const pStartNode, Node* const pDestinationNode) const;
		std::vector<Node*> ReconstructPath(const std::map<Node*, Node*>& parent, Node* const pStartNode, Node* const pDestinationNode) const;

	private:
		Graph* pGraph;
	};
}
