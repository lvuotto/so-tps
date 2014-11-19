var map = function () {
  if (this.total_votes >= 2000)
    emit("mas_de_2000", {
      arreglo: [ {
        titulo: this.title,
        puntaje: this.score,
      } ]
    });
};

var reduce = function (k, v) {
  var r = [];
  for (var i = 0; i < v.length; i++)
    r = r.concat(v[i].arreglo);
  return { arreglo: r };
};

var finalize = function (k, v) {
  v.arreglo.sort(function (a, b) {
    return b.puntaje - a.puntaje;
  });
  return v.arreglo.slice(0, 12);
};

var options = {
  "out": "ej2",
  "finalize": finalize
};


conn = new Mongo();
db = conn.getDB("reddit");


db.posts.mapReduce(map, reduce, options);
var r = db.ej2.find()[0];
printjson(r);
