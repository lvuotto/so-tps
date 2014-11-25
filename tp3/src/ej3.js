var map = function () {
  var v = {
    "arreglo": [ {
      "puntaje": this.score,
      "comentarios": this.number_of_comments
    } ]
  };
  emit("por_puntaje", v);
};

var reduce = function (k, v) {
  var r = [];
  for (var i = 0; i < v.length; i++)
    r = r.concat(v[i].arreglo);
  r.sort(function (a, b) { return b.puntaje - a.puntaje; }); 
  r = r.slice(0, 10);

  return { "arreglo": r };
};

var finalize = function (k, rv) {
  var suma = 0;
  for (var i = 0; i < rv.arreglo.length; i++)
    suma += rv.arreglo[i].comentarios;
  return suma / rv.arreglo.length;
};

var options = {
  "out": "ej3",
  "finalize": finalize
};


conn = new Mongo();
db = conn.getDB("reddit");


db.posts.mapReduce(map, reduce, options);
var r = db.ej3.find();
if (r.hasNext())
  print(r.next().value);
