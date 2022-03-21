#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

struct Ant
{
public:
	int tour[100] = {0};
	double cost = 0;
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
	int nPoints = 99; // How many points? (no more than 99)
	int nAnts = 99; // How many ants? (like above)

	olc::vd2d Loc[100];
	Ant bestTour;
	Ant ant[100];
	double length[100][100]; // Length of paths (Heuristic information)
	double pher[100][100]; // Pheromone
	double alpha = 1;
	double beta = 1;
	double rho = 0.1;

	int fps = 1;
	int fps2go = 1;
	int iter = 0;

public:
	bool OnUserCreate() override
	{
		// Called once at the start, so create things here
		// Create Points
		srand((unsigned)time(NULL));
		for (int i = 1; i <= nPoints; ++i)
		{
			Loc[i].x = rand() % ScreenWidth();
			Loc[i].y = rand() % ScreenHeight();
		}
		// Setting pheromone state at the start (apparently it was set to 0 but I just do it one more time)
		for (int i = 1; i <= nPoints; ++i)
		{
			for (int j = 1; j <= nPoints; ++j)
			{
				pher[i][j] = 0.05;
			}
		}
		// Calculate distance between points
		for (int i = 1; i <= nPoints - 1; ++i)
		{
			for (int j = i + 1; j <= nPoints; ++j)
			{
				length[i][j] = sqrt(pow(Loc[i].x - Loc[j].x, 2) + pow(Loc[i].y - Loc[j].y, 2));
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
			DrawCircle(Loc[p], ScreenWidth() / 200 + 2);
			FillCircle(Loc[p], ScreenWidth() / 200);
		}
		// Nested loop to control speed 
		while (fps > fps2go)
		{
			// Ants on action
			for (int a = 1; a <= nAnts; ++a) // Loop all ants
			{
				// Unload resouces to make ant happy
				ant[a].cost = 0;
				for (int p = 1; p <= nPoints; ++p)
				{
					ant[a].tour[p] = 0;
				}
				// Set ant on a new tour
				ant[a].tour[1] = rand() % nPoints + 1;
				int end = 1;
				for (int p = 2; p <= nPoints; ++p) // Loop all points
				{
					double P[100] = { 0 };
					int i = ant[a].tour[end];
					double sumP = 0;
					for (int j = 1; j <= nPoints; ++j) // dealing with matrix
					{
						// Calculate possibility
						P[j] = pher[i][j] / length[i][j];
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
					++end;
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
			std::cout << " Iteration #" << iter << ": " << bestTour.cost << std::endl;
			fps = 0;
		}
		++fps;
		// Draw optimal tour for the saleman
		for (int p = 1; p <= nPoints; ++p)
		{
			int i = bestTour.tour[p];
			int j = bestTour.tour[p + 1];
			DrawLineDecal(Loc[i], Loc[j]);
		}
		return true;
	}

private:
	int RouletteWheelSelection(double P[])
	{
		double r = (double)rand()/RAND_MAX;
		//std::cout << r;
		double cumsum[100]{0};
		for (int j = 1; j <= nPoints; ++j)
		{
			for (int n = 1; n <= j; ++n)
			{
				cumsum[j] += P[n]; // cumulative sum
			}
			if (cumsum[j] > r)
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