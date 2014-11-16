/**
 * Faltaría buscar el mínimo, que sería como 
 * correr otro reduce sobre la salida del
 * finalize. Pero todavía no sabemos como hacerlo.
 **/
var finalize = function (k, reduced_v) {
	reduced_v.avg = reduced_v.score_sum/reduced_v.count;
	return reduced_v;	
}
