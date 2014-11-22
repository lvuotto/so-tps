var map = function () {
  emit("por_score", {
    arreglo: [ {
      score: this.score,
      comments: this.number_of_comments
    } ]
  });
};

var reduce = function (k, v) {
  var r = [];
  for (var i = 0; i < v.length; i++)
    r = r.concat(v[i].arreglo);
	r.sort(function (a, b) { return b.score - a.score; }); 
	r = r.slice(0, 10);
  return { arreglo: r };
};

var finalize = function (k, reduced_v) {
	var suma = 0;
	for (var i = 0; i < reduced_v.arreglo.length; i++)
	  suma += reduced_v.arreglo[i].comments;
  reduced_v.avg = suma / reduced_v.arreglo.length;
  return reduced_v.avg;
};

var options = {
  "out": "ej3",
  "finalize": finalize
};


conn = new Mongo();
db = conn.getDB("reddit");


db.posts.mapReduce(map, reduce, options);
var r = db.ej3.find()[0];
print(r.value);
