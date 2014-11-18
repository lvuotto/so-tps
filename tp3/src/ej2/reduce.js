function (k, v) {
  var r = [];
  for (var i = 0; i < v.length; i++)
    r = r.concat(v[i].arreglo);
  return { arreglo: r };
}
