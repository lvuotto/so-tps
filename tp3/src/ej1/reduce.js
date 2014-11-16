var reduce = function (k, v) {
	var reduced_v = {score_sum: 0, count: 0}; 
	
	for (var i = 0; i < v.length; i++){
		reduced_v.score_sum += v[i].score;
	}

	reduced_v.count = v.length;

	return reduced_v;
}
