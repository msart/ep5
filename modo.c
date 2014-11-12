int valid(Position p, int m, int n) {
	return p.x >= 0 && p.x < m && p.y >= 0 && p.y < n;
}

int recebendotiro(Grid *grid, Position p){
	int i, bullets_comming=0;
	Position s;
	for(i = 0; i < 6; i++){
		s = getNeighbor(p, i);
		if(valid(s, g->m, g->n)) 
			if(&g->map[s.x][s.y].object==projectile)
				bullets_comming++;
	}
	return bullets_comming;
}

int modo(Grid *grid, Position pos) {
	Robot *r = &g->map[p.x][p.y].object.robot;
	if(r->bullets<1 && recebendotiro(g,pos)>1)
		varialvelglobaldemodo=1;
	else
		varialvelglobaldemodo=0;
}
