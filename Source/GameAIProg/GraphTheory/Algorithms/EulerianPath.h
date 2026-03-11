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
		if (IsConnected()) return Eulerianity::notEulerian;

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

		if (count == 2) return Eulerianity::semiEulerian;

		return Eulerianity::notEulerian;
	}

	inline std::vector<Node*> EulerianPath::FindPath(Eulerianity& eulerianity) const
	{
		// Get a copy of the graph because this algorithm involves removing edges
		Graph graphCopy = m_pGraph->Clone();
		std::vector<Node*> Path = {};
		std::vector<Node*> Nodes = graphCopy.GetActiveNodes();
		int currentNodeId{ Graphs::InvalidNodeId };

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
		visited[startIndex] = true;

		auto NodeConnections = m_pGraph->FindConnectionsFrom(Nodes.at(startIndex)->GetId());

		for (auto NodeConnection : NodeConnections)
		{
			int id = NodeConnection->GetToId();

			if (!visited[id])
			{
				VisitAllNodesDFS(Nodes, visited, id);
			}
		}
	}

	inline bool EulerianPath::IsConnected() const
	{
		std::vector<Node*> Nodes = m_pGraph->GetActiveNodes();
		if (Nodes.size() == 0)
			return false;

		std::vector<bool> Visited{};
		Visited.reserve(Nodes.size());

		VisitAllNodesDFS(Nodes, Visited, Nodes[0]->GetId());

		
		
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