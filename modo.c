int valid(Position p, int m, int n) {
	return p.x >= 0 && p.x < m && p.y >= 0 && p.y < n;
}



int modo(Grid *grid, Position pos) {
	Robot *r = &g->map[p.x][p.y].object.robot;
	if(r->bullets<1 && recebendotiro(g,pos)>1)
		varialvelglobaldemodo=1;
	else
		varialvelglobaldemodo=0;
}
