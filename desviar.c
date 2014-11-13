#include "../robot_fight.h"
#include <stdlib.h>
static int adjacentes[6];

int valid (Position pos, int m, int n)
{
	return ((pos.x >= 0 && pos.x < m) && (pos.y >= 0 && pos.y < n));
}
/* preenche o vetor adjacentes com os conteúdos de todas as células ao redor do robo*/
/* deve ser chamada 1 vez a cada turno*/
void encontrarAdjacentes (Position pos, Grid *grid)
{
	int i;
	Position p;

	for (i = 0; i < 6; i++)
	{
		p = getNeighbor (pos, i);
		if (valid (p, grid->m, grid->n)) adjacentes[i] = grid->map[p.x][p.y].type;
		else adjacentes[i] = -1; 
	}
}
void prepareGame(Grid *g, Position p, int turnCount) {

}
/*retorna 1 caso o robo deva desviar, ou retorna 0 caso contrário*/
int deveDesviar (Position pos, Grid *grid, Robot *r)
{
	int i;
	Projectile *proj;
	Position proj_pos, andar_pos;

	for (i = 0; i < 6; i++)
	{
		if (adjacentes[i] == 3)
		{
			proj_pos = 	getNeighbor (pos, i);
			proj = &grid->map[proj_pos.x][proj_pos.y].object.projectile;
			andar_pos = getNeighbor (pos, r->dir);
			if (valid (andar_pos, grid->m, grid->n) && (grid->map[andar_pos.x][andar_pos.y].type == NONE))
				if ((proj->dir + 3)%6 == i && r->dir != i && r->dir != (i + 3)%6)
					return 1;
			else
				return -1;
		}
	}
	return 0;
}

/*Action desviar (Position pos, Grid *grid, Robot *r)
{
	int i;
	Position proj_pos, andar_pos;
	Projectile *proj;

	for (i = 0; i < 6; i++)
	{
		if (adjacentes[i] == 3)
		{
			proj_pos = 	getNeighbor (pos, i);
			proj = &grid->map[proj_pos.x][proj_pos.y].object.projectile;
			if ((proj->dir + 3)%6 == i)
			{
				if (r->dir == i) return SHOOT_CENTER;
				if (r->dir == (i + 3)%6) return OBSTACLE_CENTER;
				if (grid->map[pos.x][pos.y].isControlPoint)
				{
					if (r->dir == (i + 1)%6)
						return SHOOT_LEFT;
					if (r->dir == (i + 2)%6)
						return OBSTACLE_LEFT;
					if (r->dir == (i + 4)%6)
						return OBSTACLE_RIGHT;
					if (r->dir == (i + 5)%6)
						return SHOOT_RIGHT;
				}
				else
				{
					andar_pos = getNeighbor (pos, r->dir);
					if (valid (pos, grid->m, grid->n) && grid->map[andar_pos.x][andar_pos.y].type == NONE)
						return WALK;
					else
						return TURN_LEFT;
				}
			}

		}
	}
	return STAND;
}*/

Action processTurn(Grid *grid, Position pos, int turnsLeft) {
	Robot *r = &grid->map[pos.x][pos.y].object.robot;
	encontrarAdjacentes (pos, grid);
	if (deveDesviar (pos, grid, r))
		return WALK;
	return STAND;
}
