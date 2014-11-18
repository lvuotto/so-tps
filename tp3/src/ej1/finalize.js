function (k, reduced_v) {
  reduced_v.avg = reduced_v.score_sum / reduced_v.count;
  return reduced_v;
}
