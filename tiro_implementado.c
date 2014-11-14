#include "../robot_fight.h"
/* Ultima acao */
/* Sempre que chamarmos a funcao processTurn temos que atualizar esta variavel */
int ultimaAcao;

/* Vida do robo */
typedef enum {ALTA, MEDIA, BAIXA} vidaRobo;

/* Esta funcao retorna o estado atual da vida do robo - ALTA, MEDIA ou BAIXA */
vidaRobo vidaAtual(Grid *g, Position p) 
{
	int hpAtual;
	hpAtual = g->map[p.x][p.y].object.robot.hp;

	if (hpAtual >= 60)
		return (ALTA);
	if (hpAtual >= 30)
		return (MEDIA);
	else
		return (BAIXA);
}

/***************************************/
/* BLOCO COPIADO DE controller_basic.c */
/***************************************/

static int control_dir;
/*Checa se a posicao dada esta dentro do mapa e nao esta sendo ocupada*/
int valid(Position p, int m, int n, Grid *g) {
	return ((p.x >= 0 && p.x < m && p.y >= 0 && p.y < n) && (g->map[p.x][p.y].type == NONE));
}

void prepareGame(Grid *g, Position p, int turnCount) {

	/*Um verdadeiro guerreiro nao precisa preparar nada*/

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
		cont = 1 + quickTurn(r->dir, i);
		s = getNeighbor(p, i);
		while (valid(s, g->m, g->n, g)) {
			if (isRobot(g, s)) {
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
	if (inimigo_dir == r->dir)
		return SHOOT_CENTER;
	if (inimigo_dir == (r->dir - 1)%6)
		return SHOOT_LEFT;
	if (inimigo_dir == (r->dir + 1)%6)
		return SHOOT_RIGHT;
	return fastTurn (r->dir, inimigo_dir);
}
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
/**************************************************/
/* Retorna a acao a ser realizada quando a vida do robo eh baixa */
Action acaoVidaBaixa(Grid *g, Position p)
{
	int i, j;
	Position s;
	Robot *r = &g->map[p.x][p.y].object.robot;
	
	/*Se estiver em cima de um control point, SCORE TIME*/
	if(isControlPoint(g,p))
		return atiraNoInimigo(g, p, r);

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

Action processTurn(Grid *g, Position p, int turnsLeft) 
{
	int estadoVida;
	Action acaoRobo;
	estadoVida = vidaAtual(g, p);
	estadoVida = 2;
	switch(estadoVida)
	{
		case ALTA:
			acaoRobo = acaoVidaAlta(g, p);
		break;
		case MEDIA:
			acaoRobo = acaoVidaMedia(g, p);
		break;
		case BAIXA:
			acaoRobo = acaoVidaBaixa(g, p);
		break;
	}

	return (acaoRobo);
}
