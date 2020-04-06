// ShadowCasting2D.cpp : By @pathetic_geek

#include <iostream>
using namespace std;

#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

struct sEdge {
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
#define WEST 4

class ShadowCasting2D : public olc::PixelGameEngine
{
public:
	ShadowCasting2D() {
		sAppName = "ShadowCasting2D";
	}

private:
	sCell* world;
	int nWorldWidth = 40;
	int nWorldHeight = 30;

	vector<sEdge> vecEdges;
	vector<tuple<float, float, float>> vecVisibilityPolygonPoints;

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

	void CalculateVisibilityPolygon(float ox, float oy, float radius)
	{
		vecVisibilityPolygonPoints.clear();

		for (auto& e1 : vecEdges)
		{
			for (int i = 0; i < 2; i++)
			{
				float rdx, rdy;
				rdx = (i == 0 ? e1.sx : e1.sy) - ox;
				rdy = (i == 0 ? e1.sy : e1.sx) - oy;
				float base_ang = atan2f(rdy, rdx);

				float ang = 0;

				for (int j = 0; j < 3; j++)
				{
					if (j == 0) ang = base_ang - 0.0001f;
					if (j == 1) ang = base_ang;
					if (j == 2) ang = base_ang + 0.0001f;

					// Create a ray along the angle
					rdx = radius * cosf(ang);
					rdy = radius * sinf(ang);

					float min_t1 = INFINITY;
					float min_px = 0, min_py = 0, min_ang = 0;
					bool bValid = false;

					for(auto e2 : vecEdges)
					{
						// Create ling segment vector
						float sdx = e2.ex - e2.sx;
						float sdy = e2.ey - e2.sy;

						if (fabs(sdx - rdx) > 0.0f && fabs(sdy - rdy) > 0.0f)
						{
							// t2 is normalised distance from line segment start to line segment end of intersect point
							float t2 = (rdx * (e2.sy - oy) + (rdy * (ox - e2.sx))) / (sdx * rdy - sdy * rdx);
							// t1 is normalised distance from source along ray to ray length of intersect point
							float t1 = (e2.sx + sdx * t2 - ox) / rdx;

							// If intersection pont exists
							if (t1 > 0 && t2 >= 0 && t2 <= 1.0f)
							{
								if (t1 < min_t1)
								{
									min_t1 = t1;
									min_px = ox + rdx * t1;
									min_py = oy + rdy * t1;
									min_ang = atan2f(min_py - oy, min_px - ox);
									bValid = true;
								}
							}
						}
					}

					// Add intersection point to visibility polygon perimeter
					if (bValid) {
						vecVisibilityPolygonPoints.push_back({ min_ang, min_px, min_py });
					}
				}
			}
		}

		sort(
			vecVisibilityPolygonPoints.begin(),
			vecVisibilityPolygonPoints.end(),
			[&](const tuple<float, float, float>& t1, const tuple<float, float, float>& t2)
			{
				return get<0>(t1) < get<0>(t2);
			});
	}

public:
	bool OnUserCreate() override
	{
		world = new sCell[nWorldWidth * nWorldHeight];

		// Add a boundary
		for (int x = 2; x < (nWorldWidth - 2); x++)
		{
			world[2 * nWorldWidth + x].exist = true;
			world[(nWorldHeight - 4) * nWorldWidth + x].exist = true;
		}

		for (int x = 2; x < (nWorldHeight - 2); x++)
		{
			world[x * nWorldWidth + 2].exist = true;
			world[x * nWorldWidth + (nWorldWidth - 4)].exist = true;
		}

		return true;
	}
	bool OnUserUpdate(float fElapsedTime) override
	{
		float fBlockWidth = 16.0f;
		float fSourceX = GetMouseX();
		float fSourceY = GetMouseY();

		// Toggle block existance on click
		if (GetMouse(0).bReleased)
		{
			int i = ((int)fSourceY / (int)fBlockWidth) * nWorldWidth + ((int)fSourceX / (int)fBlockWidth);
			world[i].exist = !world[i].exist;
		}

		ConvertTileMapToPolyMap(0, 0, 40, 30, fBlockWidth, nWorldWidth);

		if (GetMouse(1).bHeld)
		{
			CalculateVisibilityPolygon(fSourceX, fSourceY, 1000.0f);
		}

		// Drawing
		Clear(olc::BLACK);

		if (GetMouse(1).bHeld && vecVisibilityPolygonPoints.size() > 1)
		{
			// Draw each triangle in fan
			for (int i = 0; i < vecVisibilityPolygonPoints.size() - 1; i++)
			{

				FillTriangle(
					fSourceX,
					fSourceY,

					get<1>(vecVisibilityPolygonPoints[i]),
					get<2>(vecVisibilityPolygonPoints[i]),

					get<1>(vecVisibilityPolygonPoints[i + 1]),
					get<2>(vecVisibilityPolygonPoints[i + 1]));
			}

			FillTriangle(
				fSourceX,
				fSourceY,

				get<1>(vecVisibilityPolygonPoints[vecVisibilityPolygonPoints.size() - 1]),
				get<2>(vecVisibilityPolygonPoints[vecVisibilityPolygonPoints.size() - 1]),

				get<1>(vecVisibilityPolygonPoints[0]),
				get<2>(vecVisibilityPolygonPoints[0]));
		}

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

int main()
{
	ShadowCasting2D demo;
	if (demo.Construct(640, 480, 2, 2))
		demo.Start();
}