#pragma once
#include <stack>
#include "Shared/Graph/Graph.h"

namespace GameAI
{
	enum class Eulerianity
	{
		notEulerian,
		semiEulerian,
		eulerian,
	};

	class EulerianPath final
	{
	public:
		EulerianPath(Graph* const pGraph);

		Eulerianity IsEulerian() const;
		std::vector<Node*> FindPath(Eulerianity& eulerianity) const;

	private:
		void VisitAllNodesDFS(const std::vector<Node*>& pNodes, std::vector<bool>& visited, int startIndex) const;
		bool IsConnected() const;

		Graph* m_pGraph;
	};

	inline EulerianPath::EulerianPath(Graph* const pGraph)
		: m_pGraph(pGraph)
	{
	}

	inline Eulerianity EulerianPath::IsEulerian() const
	{
		// TODO If the graph is not connected, there can be no Eulerian Trail
		if (IsConnected()) return Eulerianity::notEulerian;

		// TODO Count nodes with odd degree
		int count = 0;

		std::vector<Node*> Nodes = m_pGraph->GetActiveNodes();

		for (auto Node : Nodes)
		{
			std::vector<Connection*> NodeConnections = m_pGraph->FindConnectionsFrom(Node->GetId());

			if (NodeConnections.size() % 2 != 0)
			{
				count++;
			}
		}



		if (count == 0) return Eulerianity::eulerian;

		// TODO A connected graph with exactly 2 nodes with an odd degree is Semi-Eulerian (unless there are only 2 nodes)
		if (count == 2) return Eulerianity::semiEulerian;

		// TODO An Euler trail can be made, but only starting and ending in these 2 nodes



		// TODO A connected graph with more than 2 nodes with an odd degree (an odd amount of connections) is not Eulerian
		// TODO A connected graph with no odd nodes is Eulerian
		return Eulerianity::notEulerian;
	}

	inline std::vector<Node*> EulerianPath::FindPath(Eulerianity& eulerianity) const
	{
		// Get a copy of the graph because this algorithm involves removing edges
		Graph graphCopy = m_pGraph->Clone();
		std::vector<Node*> Path = {};
		std::vector<Node*> Nodes = graphCopy.GetActiveNodes();
		int currentNodeId{ Graphs::InvalidNodeId };
		
		// TODO Check if there can be an Euler path
		// TODO If this graph is not eulerian, return the empty path

		switch (eulerianity)
		{
		case Eulerianity::notEulerian:
			return Path;

		case Eulerianity::semiEulerian:
			for (auto node : Nodes)
			{
				auto NodeConnections = m_pGraph->FindConnectionsFrom(node->GetId());

				if (NodeConnections.size() % 2 != 0)
				{
					currentNodeId = node->GetId();
				}
			}
			break;

		case Eulerianity::eulerian:
			currentNodeId = Nodes.at(std::rand() % Nodes.size())->GetId();
			break;
		}

		
		// TODO Start algorithm loop
		std::stack<int> nodeStack;

		while (nodeStack.empty() && graphCopy.FindConnectionsFrom(currentNodeId).empty())
		{
			nodeStack.push(currentNodeId);

			auto connect = graphCopy.FindConnectionsFrom(currentNodeId).at(0);

			if (auto newId = connect->GetToId()) currentNodeId = newId;
			graphCopy.RemoveConnection(connect);

			Path.emplace_back(m_pGraph->GetNode(currentNodeId).get());
		}

		std::ranges::reverse(Path);


		return Path;
	}

	inline void EulerianPath::VisitAllNodesDFS(const std::vector<Node*>& Nodes, std::vector<bool>& visited, int startIndex ) const
	{
		// TODO Mark the visited node
		visited[startIndex] = true;

		// TODO Ask the graph for the connections from that node
		auto NodeConnections = m_pGraph->FindConnectionsFrom(Nodes.at(startIndex)->GetId());

		// TODO recursively visit any valid connected nodes that were not visited before

		for (auto NodeConnection : NodeConnections)
		{
			int id = NodeConnection->GetToId();

			if (!visited[id])
			{
				VisitAllNodesDFS(Nodes, visited, id);
			}
		}

		// TODO Tip: use an index-based for-loop to find the correct index
	}

	inline bool EulerianPath::IsConnected() const
	{
		std::vector<Node*> Nodes = m_pGraph->GetActiveNodes();
		if (Nodes.size() == 0)
			return false;

		// TODO choose a starting node

		std::vector<bool> Visited{};
		Visited.reserve(Nodes.size());

		// TODO start a depth-first-search traversal from the node that has at least one connection
		VisitAllNodesDFS(Nodes, Visited, Nodes[0]->GetId());

		
		
		// TODO if a node was never visited, this graph is not connected
		for (auto visit : Visited)
		{
			if (!visit)
			{
				return false;
			}
		}

		return true;
	}
}