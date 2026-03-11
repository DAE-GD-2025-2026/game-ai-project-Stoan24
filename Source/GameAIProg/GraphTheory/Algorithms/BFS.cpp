#include "BFS.h"

#include <queue>
#include <set>

#include "Shared/Graph/Graph.h"

using namespace GameAI;

BFS::BFS(Graph* const pGraph)
	: pGraph(pGraph)
{
}

std::vector<Node*> BFS::FindPath(Node* const pStartNode, Node* const pDestinationNode) const
{
	std::queue<Node*> queue{};
	queue.push(pStartNode);

	std::set<Node*> visited{};
	std::map<Node*, Node*> parent{};

	visited.emplace(pStartNode);

	while (!queue.empty())
	{
		auto node = queue.front();
		queue.pop();

		if (node == pDestinationNode)
		{
			return ReconstructPath(parent, pStartNode, pDestinationNode);
		}

		std::vector<Connection*> connections = pGraph->FindConnectionsFrom(node->GetId());

		for (auto connection : pGraph->FindConnectionsFrom(node->GetId()))
		{
			Node* neighbour = pGraph->GetNode(connection->GetToId()).get();

			if (!visited.contains(neighbour))
			{
				visited.emplace(neighbour);

				parent[neighbour] = node;
				queue.push(neighbour);
			}
		}
	}

	return std::vector<Node*>{};
}

std::vector<Node*> BFS::ReconstructPath(const std::map<Node*, Node*>& parent, Node* const pStartNode, Node* const pDestinationNode) const
{
	auto current = pDestinationNode;

	std::vector<Node*> path{};

	while (current != pStartNode)
	{
		path.push_back(current);
		current = parent.at(current);
	}

	path.push_back(pStartNode);
	path.reserve(path.size());

	return path;
}