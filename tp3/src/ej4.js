var map = function () {
  emit(this.username, {votes: this.number_of_upvotes, count: 1});
};

var reduce = function (k, v) {
	var return_value = {votes: 0, count: 0};
	for (var i = 0; i < v.length; i++){
		return_value.count += v[i].count;
		return_value.votes += v[i].votes;
	}
	if (return_value.count > 5) return_value.votes = 0;
	return return_value;
};

var finalize = function (k, v) {
  return v;
};

var options = {
  "out": "ej4"/*,
  "finalize": finalize*/
};


conn = new Mongo();
db = conn.getDB("reddit");


db.posts.mapReduce(map, reduce, options);
var r = db.ej4.find().sort({ "value": -1 })[0];
print(r._id);
