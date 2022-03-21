#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

struct Ant
{
public:
	int tour[100] = { 0 };
	float cost = 0;
};

// Override base class with your custom functionality
class Example : public olc::PixelGameEngine
{
public:
	Example()
	{
		// Name your application
		sAppName = "ACO";
	}
	int nPoints = 4; // How many points? (no more than 99)
	int nAnts = 2; // How many ants? (no more than 999)

	olc::vf2d loc[100];
	Ant bestTour;
	Ant ant[1000];
	float length[100][100]; // Length of paths (Heuristic information)
	float pher[100][100]; // Pheromone
	float alpha = 1;
	float beta = 1;
	float rho = 0.1;

	int fps = 1;
	int fps2go = 1;
	int iter = 0;
	bool added = false;

public:
	bool OnUserCreate() override
	{
		// Called once at the start, so create things here
		// Create Points
		srand((unsigned)time(NULL));
		for (int i = 1; i <= nPoints; ++i)
		{
			loc[i].x = rand() % ScreenWidth();
			loc[i].y = rand() % ScreenHeight();
		}
		// Setting pheromone state at the start (apparently it was set to 0 but I just do it one more time)
		for (int i = 1; i <= 100; ++i)
		{
			for (int j = 1; j <= 100; ++j)
			{
				pher[i][j] = 0;
			}
		}
		// Calculate distance between points
		for (int i = 1; i <= nPoints - 1; ++i)
		{
			for (int j = i + 1; j <= nPoints; ++j)
			{
				length[i][j] = sqrt(pow(loc[i].x - loc[j].x, 2) + pow(loc[i].y - loc[j].y, 2));
				length[j][i] = length[i][j];
			}
		}

		bestTour.cost = INFINITY; // Set it to inf so that the very next tour found will be the best tour

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		// Draw Points
		for (int p = 1; p <= nPoints; ++p)
		{
			DrawCircle(loc[p], ScreenWidth() / 200 + 2);
			FillCircle(loc[p], ScreenWidth() / 200);
		}
		if (GetMouse(olc::Mouse::RIGHT).bHeld && !added)
		{
			loc[nPoints + 1].x = float(GetMouseX());
			loc[nPoints + 1].y = float(GetMouseY());
			++nPoints;
			int j = nPoints;
			for (int i = 1; i <= nPoints - 1; ++i)
			{
				length[i][j] = sqrt(pow(loc[i].x - loc[j].x, 2) + pow(loc[i].y - loc[j].y, 2));
				length[j][i] = length[i][j];
			}
			added = true;
			//bestTour = new Ant;
			bestTour.cost = INFINITY;
		}
		if (GetMouse(olc::Mouse::RIGHT).bReleased && added)
		{
			added = false;
		}
		// Nested loop to control speed 
		while (fps > fps2go)
		{
			// Ants on action
			for (int a = 1; a <= nAnts; ++a) // Loop all ants
			{
				// Unload resouces to make ant happy
				ant[a].cost = 0;
				for (int p = 1; p <= nPoints + 1; ++p)
				{
					ant[a].tour[p] = 0;
				}
				// Set ant on a new tour
				ant[a].tour[1] = (rand() % nPoints) + 1;
				for (int end = 1; end <= nPoints - 1; ++end) // Loop all points
				{
					float P[100] = { 0 };
					int i = ant[a].tour[end];
					float sumP = 0;
					for (int j = 1; j <= nPoints; ++j) // dealing with matrix
					{
						// Calculate possibility
						P[j] = pher[i][j] / length[i][j] + 0.01;
						for (int n = 1; n <= end; ++n)
						{
							if (j == ant[a].tour[n])
							{
								P[j] = 0;
							}
						}
						sumP += P[j];
					}
					for (int j = 1; j <= nPoints; ++j)
					{
						P[j] = P[j] / sumP; // This is the final possibility
					}
					// Chose next path
					int j = RouletteWheelSelection(P);
					ant[a].tour[end + 1] = j;
					ant[a].cost += length[i][j]; // cost is the length of paths
				}
				ant[a].cost += length[(ant[a].tour[nPoints])][(ant[a].tour[1])]; // length of the path from the last to the first point (maybe there's another way to implement that)
				ant[a].tour[nPoints + 1] = ant[a].tour[1]; // copu the first destination to the very end to form a closed tour
				// Update the best tour so far
				if (ant[a].cost < bestTour.cost)
				{
					for (int p = 1; p <= nPoints + 1; ++p)
					{
						bestTour.tour[p] = ant[a].tour[p];
					}
					bestTour.cost = ant[a].cost;
				}
			}
			// Update Pheromone
			for (int a = 1; a <= nAnts; ++a)
			{
				for (int p = 1; p <= nPoints; ++p)
				{
					int i = ant[a].tour[p];
					int j = ant[a].tour[p + 1];
					pher[i][j] += 1 / length[i][j];
					pher[j][i] = pher[i][j];
				}
			}
			// Evaporation
			for (int i = 1; i <= nPoints; ++i)
			{
				for (int j = 1; j <= nPoints; ++j)
				{
					pher[i][j] = (1 - rho) * pher[i][j];
				}
			}
			++iter;
			std::cout << " Iteration #" << iter << ": " << bestTour.cost << std::endl << nPoints << std::endl;
			fps = 0;
		}
		++fps;
		// Draw optimal tour for the saleman
		for (int p = 1; p <= nPoints; ++p)
		{
			int i = bestTour.tour[p];
			int j = bestTour.tour[p + 1];
			DrawLineDecal(loc[i], loc[j]);
		}
		return true;
	}

private:
	int RouletteWheelSelection(float P[])
	{
		float r = (float)rand()/RAND_MAX;
		//std::cout << r;
		float cumsum[100]{0};
		for (int j = 1; j <= nPoints; ++j)
		{
			for (int n = 1; n <= j; ++n)
			{
				cumsum[j] += P[n]; // cumulative sum
			}
			if (cumsum[j] >= r)
			{
				return j;
			}
		}
	}
};

int main()
{
	Example model;
	if (model.Construct(800, 600, 1, 1))
	{
		model.Start();
	}
	return 0;
}