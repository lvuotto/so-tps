var map = function () {
  emit(this.subreddit, {score:this.score, word_count:this.title.trim().split(/\s+/).length});
};

var reduce = function (k, v) {
	var return_value = {score: 0, word_count: 0};
	for (var i = 0; i < v.length; i++){
		return_value.score += v[i].score;
		return_value.word_count += v[i].word_count;
	}
	return return_value;
};

var finalize = function (k, v) {
  if (v.score > 280 && v.score < 300)
		return v.word_count;
	return -1;
};

var options = {
  "out": "ej5",
  "finalize": finalize
};


conn = new Mongo();
db = conn.getDB("reddit");


db.posts.mapReduce(map, reduce, options);
var r = db.ej5.find({ "value": { $gt: -1 } }).sort({"value": -1});
while (r.hasNext())
  print(r.next().value);
