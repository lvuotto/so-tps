var map = function () {
  if (this.total_votes >= 2000)
		emit("mas_de_2000", {
			titulo: this.title,
			puntaje: this.score
		});
};
