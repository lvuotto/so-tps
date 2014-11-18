function (k, v) {
  var reduced_v = { score_sum: 0, count: 0 }; 

  for (var i = 0; i < v.length; i++){
    reduced_v.score_sum += v[i].score_sum;
    reduced_v.count += v[i].count;
  }

  return reduced_v;
}
