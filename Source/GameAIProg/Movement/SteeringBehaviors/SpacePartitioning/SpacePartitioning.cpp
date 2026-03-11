#include "SpacePartitioning.h"

// --- Cell ---
// ------------
Cell::Cell(float Left, float Bottom, float Width, float Height)
{
	BoundingBox.Min = { Left, Bottom };
	BoundingBox.Max = { BoundingBox.Min.X + Width, BoundingBox.Min.Y + Height };
}

std::vector<FVector2D> Cell::GetRectPoints() const
{
	const float left = BoundingBox.Min.X;
	const float bottom = BoundingBox.Min.Y;
	const float width = BoundingBox.Max.X - BoundingBox.Min.X;
	const float height = BoundingBox.Max.Y - BoundingBox.Min.Y;

	std::vector<FVector2D> rectPoints =
	{
		{ left , bottom  },
		{ left , bottom + height  },
		{ left + width , bottom + height },
		{ left + width , bottom  },
	};

	return rectPoints;
}

// --- Partitioned Space ---
// -------------------------
CellSpace::CellSpace(UWorld* pWorld, float Width, float Height, int Rows, int Cols, int MaxEntities)
	: pWorld{pWorld}
	, SpaceWidth{Width}
	, SpaceHeight{Height}
	, NrOfRows{Rows}
	, NrOfCols{Cols}
	, NrOfNeighbors{0}
{
	Neighbors.SetNum(MaxEntities);
	
	//calculate bounds of a cell
	CellWidth = Width / Cols;
	CellHeight = Height / Rows;

	CellOrigin = { -Width / 2.f, -Height / 2.f };

	//Create cells
	for (int row = 0; row < Rows; ++row)
	{
		for (int col = 0; col < Cols; ++col)
		{
			float left = CellOrigin.X + (col * CellWidth);
			float bottom = CellOrigin.Y + (row * CellHeight);
			Cells.push_back(Cell(left, bottom, CellWidth, CellHeight));
		}
	}
}

void CellSpace::AddAgent(ASteeringAgent& Agent)
{
	const int index = PositionToIndex(Agent.GetPosition());
	Cells[index].Agents.push_back(&Agent);
}

void CellSpace::UpdateAgentCell(ASteeringAgent& Agent, const FVector2D& OldPos)
{
	const int oldIndex = PositionToIndex(OldPos);
	const int newIndex = PositionToIndex(Agent.GetPosition());

	if (oldIndex != newIndex)
	{
		Cells[oldIndex].Agents.remove(&Agent);
		Cells[newIndex].Agents.push_back(&Agent);
	}
}

void CellSpace::RegisterNeighbors(const ASteeringAgent& Agent, const float QueryRadius)
{
	NrOfNeighbors = 0;
	const FVector2D positon = Agent.GetPosition();

	FRect queryBox;
	queryBox.Min = { positon.X - QueryRadius, positon.Y - QueryRadius };
	queryBox.Max = { positon.X + QueryRadius, positon.Y + QueryRadius };

	const float queryRadiusSq = QueryRadius * QueryRadius;

	const int maxNeighbors = Neighbors.Num();

	for (const Cell& cell : Cells)
	{
		if (DoRectsOverlap(cell.BoundingBox, queryBox))
		{
			for (ASteeringAgent* pOtherAgent : cell.Agents)
			{
				if (NrOfNeighbors >= maxNeighbors)
				{
					return;
				}

				if (pOtherAgent == &Agent) continue;

				float distanceSq = FVector2D::DistSquared(positon, pOtherAgent->GetPosition());
				if (distanceSq < queryRadiusSq)
				{
					Neighbors[NrOfNeighbors] = pOtherAgent;
					NrOfNeighbors++;
				}
			}
		}
	}
}

void CellSpace::EmptyCells()
{
	for (Cell& c : Cells)
		c.Agents.clear();
}

void CellSpace::RenderCells() const
{
	if (!pWorld) return;

	for (const Cell& cell : Cells)
	{
		// Define the 4 corners based strictly on Min and the known Width/Height
		FVector2D Min = cell.BoundingBox.Min;
		FVector2D Max = Min + FVector2D(CellWidth, CellHeight); // Force the size

		FVector TL = FVector(Min.X, Min.Y, 90.f);
		FVector TR = FVector(Max.X, Min.Y, 90.f);
		FVector BR = FVector(Max.X, Max.Y, 90.f);
		FVector BL = FVector(Min.X, Max.Y, 90.f);

		// Draw the 4 edges
		DrawDebugLine(pWorld, TL, TR, FColor::Silver, false, -1.f, 0, 1.f);
		DrawDebugLine(pWorld, TR, BR, FColor::Silver, false, -1.f, 0, 1.f);
		DrawDebugLine(pWorld, BR, BL, FColor::Silver, false, -1.f, 0, 1.f);
		DrawDebugLine(pWorld, BL, TL, FColor::Silver, false, -1.f, 0, 1.f);

		if (cell.Agents.size() > 0)
		{
			FVector center = FVector(cell.BoundingBox.Min + (FVector2D(CellWidth, CellHeight) * 0.5f), 95.f);
			DrawDebugString(pWorld, center, FString::FromInt(cell.Agents.size()), nullptr, FColor::White, 0.01f);
		}
	}
}

int CellSpace::PositionToIndex(FVector2D const & Pos) const
{
	int col = static_cast<int>((Pos.X - CellOrigin.X) / CellWidth);
	int row = static_cast<int>((Pos.Y - CellOrigin.Y) / CellHeight);

	col = FMath::Clamp(col, 0, NrOfCols - 1);
	row = FMath::Clamp(row, 0, NrOfRows - 1);

	return row * NrOfCols + col;
}

bool CellSpace::DoRectsOverlap(FRect const & RectA, FRect const & RectB)
{
	// Check if the rectangles are separated on either axis
	if (RectA.Max.X < RectB.Min.X || RectA.Min.X > RectB.Max.X) return false;
	if (RectA.Max.Y < RectB.Min.Y || RectA.Min.Y > RectB.Max.Y) return false;
    
	// If they are not separated, they must overlap
	return true;
}