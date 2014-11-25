var map = function () {
  if (this.total_votes >= 2000) {
    var v = {
      "arreglo": [ {
        "titulo": this.title,
        "puntaje": this.score,
      } ]
    };
    emit("mas_de_2000", v);
  }
};

var reduce = function (k, v) {
  var r = [];
  for (var i = 0; i < v.length; i++)
    r = r.concat(v[i].arreglo);

  return { "arreglo": r };
};

var finalize = function (k, rv) {
  rv.arreglo.sort(function (a, b) { return b.puntaje - a.puntaje; });
  var p = rv.arreglo.slice(0, 12);
  var res = new Array(p.length);
  for (var i = 0; i < p.length; i++)
    res[i] = p[i].titulo;

  return res;
};

var options = {
  "out": "ej2",
  "finalize": finalize
};


conn = new Mongo();
db = conn.getDB("reddit");


db.posts.mapReduce(map, reduce, options);
var r = db.ej2.find();
if (r.hasNext())
  print(r.next().value.join("\n"));
