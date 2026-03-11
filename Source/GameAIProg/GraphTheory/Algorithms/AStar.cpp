#include "AStar.h"

#include <list>

using namespace GameAI;

AStar::AStar(Graph* const pGraph, HeuristicFunctions::Heuristic hFunction)
	: pGraph(pGraph)
	, HeuristicFunction(hFunction)
{
}

std::vector<Node*>AStar::FindPath(Node* const pStartNode, Node* const pGoalNode) const
{
	std::vector<Node*> path{};
	std::list<NodeRecord> openList{};
	std::list<NodeRecord> closedList{};

	NodeRecord startRecord;
	startRecord.pNode = pStartNode;
	startRecord.pConnection = nullptr;
	startRecord.costSoFar = 0;
	startRecord.estimatedTotalCost = GetHeuristicCost(pStartNode, pGoalNode);

	openList.push_back(startRecord);

	NodeRecord currentRecord{};

	while (!openList.empty())
	{
		currentRecord = *std::min_element(openList.begin(), openList.end());

		if (currentRecord.pNode == pGoalNode) break;

		for (const auto connection : pGraph->FindConnectionsFrom(currentRecord.pNode->GetId()))
		{
			Node* pNextNode = pGraph->GetNode(connection->GetToId()).get();

			const float gCostSoFar = currentRecord.costSoFar + connection->GetWeight();

			const float hCost = GetHeuristicCost(pNextNode, pGoalNode);

			auto closedIterator = std::ranges::find_if(closedList,
			[pNextNode](const NodeRecord& r)
			{
				return r.pNode == pNextNode;
			});

			if (closedIterator != closedList.end())
			{
				if (closedIterator->costSoFar <= gCostSoFar) continue;
				closedList.erase(closedIterator);
			}

			auto openIterator = std::ranges::find_if(openList,
			[pNextNode](const NodeRecord& r)
			{
				return r.pNode == pNextNode;
			});

			if (openIterator != openList.end())
			{
				if (openIterator->costSoFar <= gCostSoFar) continue;
				openList.erase(openIterator);
			}

			NodeRecord nextRecord;
			nextRecord.pNode = pNextNode;
			nextRecord.pConnection = connection;
			nextRecord.costSoFar = gCostSoFar;
			nextRecord.estimatedTotalCost = gCostSoFar + hCost;
			openList.push_back(nextRecord);
		}

		openList.remove(currentRecord);
		closedList.push_back(currentRecord);
	}

	if (currentRecord.pNode == pGoalNode)
	{
		while (currentRecord.pNode != pStartNode)
		{
			path.push_back(currentRecord.pNode);

			auto fromId = currentRecord.pConnection->GetFromId();

			auto iterator = std::ranges::find_if(closedList,
				[fromId](const NodeRecord& record) 
				{
					return record.pNode->GetId() == fromId;
				});

			if (iterator != closedList.end())
			{
				currentRecord = *iterator;
			}
			else
			{
				break;
			}
		}

		path.push_back(pStartNode);

		std::ranges::reverse(path);
	}

	return path;
}

float AStar::GetHeuristicCost(Node* const pStartNode, Node* const pEndNode) const
{
	FVector2D toDestination = pGraph->GetNode(pEndNode->GetId())->GetPosition() - pGraph->GetNode(pStartNode->GetId())->GetPosition();
	return HeuristicFunction(abs(toDestination.X), abs(toDestination.Y));
}