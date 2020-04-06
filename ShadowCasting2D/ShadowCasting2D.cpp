#include <iostream>
#include <algorithm>
using namespace std;

#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"


struct sEdge
{
	float sx, sy;
	float ex, ey;
};

struct sCell {
	int edge_id[4];
	bool edge_exist[4];
	bool exist = false;
};

#define NORTH 0
#define SOUTH 1
#define EAST 2
#define WEST 3


class ShadowCasting2D : public olc::PixelGameEngine
{
public:
	ShadowCasting2D()
	{
		sAppName = "ShadowCasting2D";
	}

private:
	sCell* world;
	int nWorldWidth = 40;
	int nWorldHeight = 30;

	vector<sEdge> vecEdges;

	void ConvertTileMapToPolyMap(int sx, int sy, int w, int h, float fBlockWidth, int pitch)
	{
		// Clear "PolyMap"
		vecEdges.clear();

		for (int x = 0; x < w; x++)
			for (int y = 0; y < h; y++)
				for (int j = 0; j < 4; j++)
				{
					world[(y + sy) * pitch + (x + sx)].edge_exist[j] = false;
					world[(y + sy) * pitch + (x + sx)].edge_id[j] = 0;
				}

		// Iterate through region from top left to bottom right
		for (int x = 1; x < w - 1; x++)
			for (int y = 1; y < h - 1; y++)
			{
				// Create some convenient indices
				int i = (y + sy) * pitch + (x + sx);			// This
				int n = (y + sy - 1) * pitch + (x + sx);		// Northern Neighbour
				int s = (y + sy + 1) * pitch + (x + sx);		// Southern Neighbour
				int w = (y + sy) * pitch + (x + sx - 1);	// Western Neighbour
				int e = (y + sy) * pitch + (x + sx + 1);	// Eastern Neighbour

				// If this cell exists, check if it needs edges
				if (world[i].exist)
				{
					// If this cell has no western neighbour, it needs a western edge
					if (!world[w].exist)
					{
						// It can either extend it from its northern neighbour if they have
						// one, or It can start a new one.
						if (world[n].edge_exist[WEST])
						{
							// Northern neighbour has a western edge, so grow it downwards
							vecEdges[world[n].edge_id[WEST]].ey += fBlockWidth;
							world[i].edge_id[WEST] = world[n].edge_id[WEST];
							world[i].edge_exist[WEST] = true;
						}
						else
						{
							// Northern neighbour does not have one, so create one
							sEdge edge;
							edge.sx = (sx + x) * fBlockWidth; edge.sy = (sy + y) * fBlockWidth;
							edge.ex = edge.sx; edge.ey = edge.sy + fBlockWidth;

							// Add edge to Polygon Pool
							int edge_id = vecEdges.size();
							vecEdges.push_back(edge);

							// Update tile information with edge information
							world[i].edge_id[WEST] = edge_id;
							world[i].edge_exist[WEST] = true;
						}
					}

					// If this cell dont have an eastern neignbour, It needs a eastern edge
					if (!world[e].exist)
					{
						// It can either extend it from its northern neighbour if they have
						// one, or It can start a new one.
						if (world[n].edge_exist[EAST])
						{
							// Northern neighbour has one, so grow it downwards
							vecEdges[world[n].edge_id[EAST]].ey += fBlockWidth;
							world[i].edge_id[EAST] = world[n].edge_id[EAST];
							world[i].edge_exist[EAST] = true;
						}
						else
						{
							// Northern neighbour does not have one, so create one
							sEdge edge;
							edge.sx = (sx + x + 1) * fBlockWidth; edge.sy = (sy + y) * fBlockWidth;
							edge.ex = edge.sx; edge.ey = edge.sy + fBlockWidth;

							// Add edge to Polygon Pool
							int edge_id = vecEdges.size();
							vecEdges.push_back(edge);

							// Update tile information with edge information
							world[i].edge_id[EAST] = edge_id;
							world[i].edge_exist[EAST] = true;
						}
					}

					// If this cell doesnt have a northern neignbour, It needs a northern edge
					if (!world[n].exist)
					{
						// It can either extend it from its western neighbour if they have
						// one, or It can start a new one.
						if (world[w].edge_exist[NORTH])
						{
							// Western neighbour has one, so grow it eastwards
							vecEdges[world[w].edge_id[NORTH]].ex += fBlockWidth;
							world[i].edge_id[NORTH] = world[w].edge_id[NORTH];
							world[i].edge_exist[NORTH] = true;
						}
						else
						{
							// Western neighbour does not have one, so create one
							sEdge edge;
							edge.sx = (sx + x) * fBlockWidth; edge.sy = (sy + y) * fBlockWidth;
							edge.ex = edge.sx + fBlockWidth; edge.ey = edge.sy;

							// Add edge to Polygon Pool
							int edge_id = vecEdges.size();
							vecEdges.push_back(edge);

							// Update tile information with edge information
							world[i].edge_id[NORTH] = edge_id;
							world[i].edge_exist[NORTH] = true;
						}
					}

					// If this cell doesnt have a southern neignbour, It needs a southern edge
					if (!world[s].exist)
					{
						// It can either extend it from its western neighbour if they have
						// one, or It can start a new one.
						if (world[w].edge_exist[SOUTH])
						{
							// Western neighbour has one, so grow it eastwards
							vecEdges[world[w].edge_id[SOUTH]].ex += fBlockWidth;
							world[i].edge_id[SOUTH] = world[w].edge_id[SOUTH];
							world[i].edge_exist[SOUTH] = true;
						}
						else
						{
							// Western neighbour does not have one, so I need to create one
							sEdge edge;
							edge.sx = (sx + x) * fBlockWidth; edge.sy = (sy + y + 1) * fBlockWidth;
							edge.ex = edge.sx + fBlockWidth; edge.ey = edge.sy;

							// Add edge to Polygon Pool
							int edge_id = vecEdges.size();
							vecEdges.push_back(edge);

							// Update tile information with edge information
							world[i].edge_id[SOUTH] = edge_id;
							world[i].edge_exist[SOUTH] = true;
						}
					}

				}

			}
	}


public:
	bool OnUserCreate() override
	{
		world = new sCell[nWorldWidth * nWorldHeight];
		return true;
	}

	bool OnUserUpdate(float fElapsedTime)
	{
		float fBlockWidth = 16.0f;
		float fSourceX = GetMouseX();
		float fSourceY = GetMouseY();

		// Toggle tiles
		if (GetMouse(0).bReleased)
		{
			// i = y * width + x
			int i = ((int)fSourceY / (int)fBlockWidth) * nWorldWidth + ((int)fSourceX / (int)fBlockWidth);
			world[i].exist = !world[i].exist;
		}

		// Take a region of "TileMap" and convert it to "PolyMap"
		ConvertTileMapToPolyMap(0, 0, 40, 30, fBlockWidth, nWorldWidth);

		// Clear screen
		Clear(olc::BLACK);

		// Draw Blocks from TileMap
		for (int x = 0; x < nWorldWidth; x++)
			for (int y = 0; y < nWorldHeight; y++)
			{
				if (world[y * nWorldWidth + x].exist)
					FillRect(x * fBlockWidth, y * fBlockWidth, fBlockWidth, fBlockWidth, olc::BLUE);
			}

		// Draw Edges from PolyMap
		for (auto& e : vecEdges)
		{
			DrawLine(e.sx, e.sy, e.ex, e.ey);
			FillCircle(e.sx, e.sy, 3, olc::RED);
			FillCircle(e.ex, e.ey, 3, olc::RED);
		}

		return true;
	}
};

int main() {
	ShadowCasting2D demo;
	if (demo.Construct(640, 480, 2, 2))
		demo.Start();
}