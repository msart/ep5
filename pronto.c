#include "../robot_fight.h"

static int adjacentes[6];

typedef enum {SAUDAVEL, RISCO} vidaRobo;

/* Esta funcao retorna o estado atual da vida do robo - SAUDAVEL E RISCO */
vidaRobo vidaAtual(Grid *g, Position p) 
{
	int hpAtual;
	hpAtual = g->map[p.x][p.y].object.robot.hp;

	if (hpAtual >= 20) return (SAUDAVEL);
	return (RISCO);
}

/*Checa se a posicao dada esta dentro do mapa*/
int limiteDoMapa (Position pos, int m, int n)
{
	return ((pos.x >= 0 && pos.x < m) && (pos.y >= 0 && pos.y < n));
}

/*Checa se a posicao dada esta vazia e esta dentro do mapa*/
int valid (Position p, int m, int n, Grid *g) {
	return ((p.x >= 0 && p.x < m && p.y >= 0 && p.y < n) && (g->map[p.x][p.y].type == NONE));
}

/* preenche o vetor das posicoes ao redor do robo */
void encontrarAdjacentes (Position pos, Grid *grid)
{
	int i;
	Position p;

	for (i = 0; i < 6; i++)
	{
		p = getNeighbor (pos, i);
		if (limiteDoMapa (p, grid->m, grid->n)) adjacentes[i] = grid->map[p.x][p.y].type;
		else adjacentes[i] = -1;
	}
}

/*Dada uma direcao inicial e uma direcao final, ve qual
o menor numero de viradas sao necessarias*/
int quickTurn(int ini, int end) {
	int i, j;
	for(i = ini, j = 0; i != end; i = (i+1)%6, j++)
		if (i >= 6) i-= 6;
	if (j > 3) j = 6-j;
	return j;
}

/*Dada uma direcao inicial e uma direcao final, ve
para qual lado virando eh mais rapido de se chegar*/
Action fastTurn(int ini, int end) {
	int dif = end-ini;
	if((dif <= 3 && dif >= 0) || (dif <= -3))
		return TURN_RIGHT;
	else
		return TURN_LEFT;		
}

/* checa se a posicao p contem um robo */
int isRobot (Grid *g, Position p) {
	if (g->map[p.x][p.y].type == ROBOT)
		return 1;
	return 0;
}
/*Dado uma posicao, checa se para alguma direcao
existe um robo, e retorna qual direcao esta
o mais perto, contando giradas necessárias*/
int searchNearestRobot (Grid *g, Position p, Robot *r) {
	int i, min = 500, best_dir = 0, cont;
	Position s;

	for (i = 0; i < 6; i++) {
		/*Conta para chegar o numero de viradas necessarias
		ja que elas gastam um turno*/
		cont = 1 + quickTurn (r->dir, i);
		s = getNeighbor (p, i);
		while (limiteDoMapa (s, g->m, g->n)) {
			if (isRobot (g, s)) {
				if(cont < min) {
					min = cont;
					best_dir = i;
					break;
				}
			}
			s = getNeighbor (s, i);
		}
	}
	/*Nao existe robos nas direções ao redor do jogador*/
	if (min == 500)
		return -1;
	
	else
		return best_dir;
}

/*atira no inimigo se ele estiver na frente, ou vira se ele não estiver*/
Action atiraNoInimigo (Grid *g, Position p, Robot *r)
{
	Direction inimigo_dir;

	inimigo_dir = searchNearestRobot (g, p, r);
	if (inimigo_dir == -1 || r->bullets == 0)
		return STAND;
	if (inimigo_dir == r->dir)
		return SHOOT_CENTER;
	if (inimigo_dir == (r->dir - 1)%6)
		return SHOOT_LEFT;
	if (inimigo_dir == (r->dir + 1)%6)
		return SHOOT_RIGHT;
	return fastTurn (r->dir, inimigo_dir);
}

/* as 2 funcoes seguintes foram retiradas de controller_basic.c */
int isControlPoint(Grid *g, Position p) {
	return (g->map[p.x][p.y].isControlPoint);	
}

/*Dado uma posicao, checa se para alguma direcao
existe um control point, e retorna qual direcao esta
o mais perto, contando giradas necessárias*/
int searchNearestControl(Grid *g, Position p, Robot *r) {
	int i, min = 500, best_dir = 0, cont;

	for(i = 0; i < 6; i++) {
		/*Conta para chegar o numero de viradas necessarias
		ja que elas gastam um turno*/
		cont = 1 + quickTurn(r->dir, i);
		Position s = getNeighbor(p,i);
		while(valid(s, g->m, g->n, g)) {
			if(isControlPoint(g,s)) {
				if(cont < min) {
					min = cont;
					best_dir = i;
					break;
				}
			}
			cont++;
			s = getNeighbor(s, i);
		}
	}

	/*Nao existe control points no mapa*/
	if (min == 500)
		return -1;
	
	else
		return best_dir;
}
/* coloca um bloco na direção do inimigo mais proximo */
Action porBloco (Position pos, Grid *grid, Robot *r) {
	Direction inimigo_dir;
	Position inimigo; /*posicao adjacente ao robo na direcao do nimigo mais proximo*/
	
	inimigo_dir = searchNearestRobot (grid, pos, r);
	if (inimigo_dir == -1 || r->obstacles == 0)
		return STAND;
	
	inimigo = getNeighbor (pos, inimigo_dir);
	if (grid->map[inimigo.x][inimigo.y].type == 0 || grid->map[inimigo.x][inimigo.y].type == 2) {
		if (inimigo_dir == (r->dir + 3)%6)
			return OBSTACLE_CENTER;
		if (inimigo_dir == (r->dir + 2)%6)
			return OBSTACLE_RIGHT;
		if (inimigo_dir == (r->dir + 4)%6)
			return OBSTACLE_LEFT;
	}
	return fastTurn (r->dir, (inimigo_dir + 3)%6);
}

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
			if (valid (andar_pos, grid->m, grid->n, grid))
				if ((proj->dir + 3)%6 == i && r->dir != i && r->dir != (i + 3)%6)
					return 1; /*pode desviar reto*/
				else
					return -2; /*ou atira para destruir o tiro, ou põe bloco*/
			else
				return -1; /*gira para o lado mais proximo para tentar fugir*/
		}
	}
	return 0;
}

Action Desviar(Position pos, Grid *grid, Robot *r){
	int deve, i;
	Position p, proj_pos;
	Projectile *proj;

	deve = deveDesviar (pos, grid, r);
	printf ("Deve Desviar (%d)\n", deve);
	if (isControlPoint (grid, pos)) {
		for (i = 0; i < 6; i++){
			if (adjacentes[i] == 3){
				proj_pos = getNeighbor (pos, i);
				proj = &grid->map[proj_pos.x][proj_pos.y].object.projectile;
				if ((proj->dir + 3)%6 == i){
					if (r->dir == i) return SHOOT_CENTER;
					if (r->dir == (i + 1)%6) return SHOOT_LEFT;
					if (r->dir == (i + 2)%6) return OBSTACLE_LEFT;
					if (r->dir == (i + 3)%6) return OBSTACLE_CENTER;
					if (r->dir == (i + 4)%6) return OBSTACLE_RIGHT;
					if (r->dir == (i + 5)%6) return SHOOT_RIGHT;
				}
			}
		}
	}
	else {
		if(deve==1) return WALK;
		else if(deve==-1){
			p=getNeighbor (pos, (r->dir+1)%6);
			if(limiteDoMapa (p, grid->m, grid->n) && (grid->map[p.x][p.y].type == NONE)) return TURN_RIGHT;
			else return TURN_LEFT;
			/*vai virar pra esquerda caso a posição da esquerda seja vazia, ou nem a da direita ou da esquerda seja vazia, ai ele vai ter q da uma volta*/
		}
		else {/*sobrou só o caso -2*/
			for (i = 0; i < 6; i++){
				if (adjacentes[i] == 3){
					proj_pos = getNeighbor (pos, i);
					proj = &grid->map[proj_pos.x][proj_pos.y].object.projectile;
					if ((proj->dir + 3)%6 == i){
						if (r->dir == i) return SHOOT_CENTER;
						if (r->dir == (i + 3)%6) return OBSTACLE_CENTER;
					}
				}
			}
		}
	}
	return STAND;	
}

/* Retorna a acao a ser realizada quando a vida do robo eh baixa */
Action acaoSaudavel (Grid *g, Position p) {
	int i, j, control_dir;
	Position s;
	Robot *r = &g->map[p.x][p.y].object.robot;
	
	/*Se estiver em cima de um control point, SCORE TIME*/
	if(isControlPoint(g,p)) {
		if (deveDesviar (p, g, r)) return Desviar (p, g, r);
		return atiraNoInimigo(g, p, r);
	}
	else {
		/*procura algum control point em alguam direcao do robo*/
		control_dir = searchNearestControl(g, p, r);
		/*Caso em nenhuma direcao tem um control point livre
		andar em uma direcao valida, ou comeca a virar para uma direcao valida*/
		if (control_dir == -1) {
			for(i = r->dir, j = 0; j < 6; i++,j++){
				if (i >= 6) i-=6;
				s = getNeighbor(p,i);
				if(valid(s, g->m, g->n, g)) {
					if(i == r->dir) {
						return WALK;
					}
					else {
						return fastTurn(r->dir, i);
					}
				}
			}
			/*Se nenhuma posicao em volta eh valida, SAD TIME*/
			return STAND;
		}
		/*Se encontrou um control point em alguma direcao,
		 comeca a virar e andar em sua direcao*/
		else if(control_dir == r->dir)
			return WALK;
		else {
			return fastTurn(r->dir, control_dir);
		}
	}
}

Action acaoRisco (Grid *g, Position p) {
	int i, j, control_dir;
	Position s;
	Robot *r = &g->map[p.x][p.y].object.robot;
	
	/*Se estiver em cima de um control point, SCORE TIME*/
	if(isControlPoint(g,p)) {
		if (deveDesviar (p, g, r)) return Desviar (p, g, r);
		return porBloco (p, g, r);
	}
	else {
		/*procura algum control point em alguam direcao do robo*/
		control_dir = searchNearestControl(g, p, r);
		/*Caso em nenhuma direcao tem um control point livre
		andar em uma direcao valida, ou comeca a virar para uma direcao valida*/
		if (control_dir == -1) {
			for(i = r->dir, j = 0; j < 6; i++,j++){
				if (i >= 6) i-=6;
				s = getNeighbor(p,i);
				if(valid(s, g->m, g->n, g)) {
					if(i == r->dir) {
						return WALK;
					}
					else {
						return fastTurn(r->dir, i);
					}
				}
			}
			/*Se nenhuma posicao em volta eh valida, SAD TIME*/
			return STAND;
		}
		/*Se encontrou um control point em alguma direcao,
		 comeca a virar e andar em sua direcao*/
		else if(control_dir == r->dir)
			return WALK;
		else
			return fastTurn(r->dir, control_dir);
	}
}

void prepareGame(Grid *g, Position p, int turnCount) {
	/*Um verdadeiro guerreiro nao precisa preparar nada*/
}
Action processTurn(Grid *g, Position p, int turnsLeft) {
	int estadoVida;

	encontrarAdjacentes (p, g);
	estadoVida = vidaAtual(g, p);
	switch(estadoVida)
	{
		case SAUDAVEL:
			return acaoSaudavel(g, p);
		break;
		case RISCO:
			return acaoRisco(g, p);
		break;
	}
	return STAND;
}
